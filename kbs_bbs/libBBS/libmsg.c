#include "bbs.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <assert.h>
#if HAVE_MYSQL == 1
#include <mysql.h>
#endif
#define MAXMESSAGE 5
char MsgDesUid[20];
char msgerr[255];
int getuinfopid(void);
int write_peer(bbsmsg_t * msgbuf)
{
    char buf[2 * STRLEN];

    assert(msgbuf != NULL);
    snprintf(buf, sizeof(buf), "%d %s", msgbuf->type, msgbuf->rawdata);
    return write(msgbuf->sockfd, buf, strlen(buf));
}
int canmsg(struct userec *fromuser, struct user_info *uin)
{
    if ((uin->pager & ALLMSG_PAGER) || HAS_PERM(fromuser, PERM_SYSOP))
        return true;
    if ((uin->pager & FRIENDMSG_PAGER)) {
        if (hisfriend(searchuser(fromuser->userid), uin))
            return true;
    }
    return false;
}
int can_override(char *userid, char *whoasks)
{
    struct friends fh;
    char buf[255];

    sethomefile(buf, userid, "friends");
    return (search_record(buf, &fh, sizeof(fh), (RECORD_FUNC_ARG) cmpfnames, whoasks) > 0) ? true : false;
}
int read_peer(int sockfd, bbsmsg_t * msgbuf)
{
    char buf[2 * STRLEN];
    char *ptr;
    int rv;
    int rc;

    /*
     * if (msgbuf == NULL)
     * return -1; 
     */
    /*
     * assert() macro can be removed by -DNDEBUG 
     */
    assert(msgbuf != NULL);
    msgbuf->sockfd = sockfd;
    if ((rc = read(sockfd, buf, sizeof(buf) - 1)) < 0)
        return -1;
    buf[rc] = '\0';
    msgbuf->type = atoi(buf);
    ptr = strchr(buf, ' ');
    if (ptr == NULL) {
        msgbuf->rawdata[0] = '\0';
        rv = -1;
    } else {
        strcpy(msgbuf->rawdata, ptr + 1);
        rv = 0;
    }
    return rv;
}

int get_request_type(bbsmsg_t * msgbuf)
{
    assert(msgbuf != NULL);
    return msgbuf->type;
}

int get_response_type(bbsmsg_t * msgbuf)
{
    assert(msgbuf != NULL);
    return msgbuf->type;
}

int get_sockfd()
{
    struct sockaddr_un sockun;
    int sockfd;
    char path[80];

    bzero(&sockun, sizeof(sockun));
    snprintf(path, sizeof(path), BBSHOME "/.msgd");
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd == -1)
        return -1;
    sockun.sun_family = AF_UNIX;
    strncpy(sockun.sun_path, path, sizeof(sockun.sun_path) - 1);
    if (connect(sockfd, (struct sockaddr *) &sockun, sizeof(sockun)) < 0) {
        close(sockfd);
        return -1;
    }
    return sockfd;
}
int addto_msglist(int utmpnum, char *userid)
{
    bbsmsg_t msgbuf;

    if ((msgbuf.sockfd = get_sockfd()) < 0)
        return -1;
    msgbuf.type = MSGD_NEW;
    snprintf(msgbuf.rawdata, sizeof(msgbuf.rawdata), "NEW %s %d\n", userid, utmpnum);
    write_peer(&msgbuf);
    if (read_peer(msgbuf.sockfd, &msgbuf) < 0)
        goto add_failed;
    if (msgbuf.type != MSGD_HLO)
        goto add_failed;
    close(msgbuf.sockfd);
    return 0;
  add_failed:
    close(msgbuf.sockfd);
    return -1;
}
int delfrom_msglist(int utmpnum, char *userid)
{
    bbsmsg_t msgbuf;

    if ((msgbuf.sockfd = get_sockfd()) < 0)
        return -1;
    msgbuf.type = MSGD_DEL;
    snprintf(msgbuf.rawdata, sizeof(msgbuf.rawdata), "DEL %s %d\n", userid, utmpnum);
    write_peer(&msgbuf);
    if (read_peer(msgbuf.sockfd, &msgbuf) < 0)
        goto del_failed;
    if (msgbuf.type != MSGD_BYE)
        goto del_failed;
    close(msgbuf.sockfd);
    return 0;
  del_failed:
    close(msgbuf.sockfd);
    return -1;
}

int send_webmsg(int destutmp, char *destid, int srcutmp, char *srcid, 
				time_t sndtime, char *msg)
{
    bbsmsg_t msgbuf;

    if ((msgbuf.sockfd = get_sockfd()) < 0)
        return -1;
    msgbuf.type = MSGD_SND;
    snprintf(msgbuf.rawdata, sizeof(msgbuf.rawdata), "SND %s %d %s %d %d\n", 
			 destid, destutmp, srcid, srcutmp, sndtime);
    write_peer(&msgbuf);
    if (read_peer(msgbuf.sockfd, &msgbuf) < 0)
        goto send_failed;
    if (msgbuf.type != MSGD_OK)
        goto send_failed;
    msgbuf.type = MSGD_MSG;
    snprintf(msgbuf.rawdata, sizeof(msgbuf.rawdata), "MSG %s\n", msg);
    write_peer(&msgbuf);
    if (read_peer(msgbuf.sockfd, &msgbuf) < 0)
        goto send_failed;
    if (msgbuf.type != MSGD_OK)
        goto send_failed;
    close(msgbuf.sockfd);
    return 0;
  send_failed:
    close(msgbuf.sockfd);
    return -1;
}
int receive_webmsg(int destutmp, char *destid, int *srcutmp, char *srcid, 
				   time_t *sndtime, char *msg)
{
    bbsmsg_t msgbuf;
    char *ptr;
    char *ptr2;

    if ((msgbuf.sockfd = get_sockfd()) < 0)
        return -1;
    msgbuf.type = MSGD_RCV;
    snprintf(msgbuf.rawdata, sizeof(msgbuf.rawdata), "RCV %s %d\n", destid, destutmp);
    write_peer(&msgbuf);
    if (read_peer(msgbuf.sockfd, &msgbuf) < 0)
        goto receive_failed;
    if (msgbuf.type != MSGD_FRM)
        goto receive_failed;
	/* rawdata must be "FRM srcid srcutmp sndtime" */
    if ((ptr = strchr(msgbuf.rawdata, ' ')) == NULL)
        goto receive_failed;
    *ptr++ = '\0';
    if ((ptr2 = strchr(ptr, ' ')) == NULL)
        goto receive_failed;
    *ptr2++ = '\0';
    strncpy(srcid, ptr, IDLEN+2);
    srcid[IDLEN] = '\0';
    *srcutmp = atoi(ptr2);
	if ((ptr = strchr(ptr2, ' ')) == NULL)
		goto receive_failed;
	*ptr++ = '\0';
	*sndtime = atoi(ptr);
    msgbuf.type = MSGD_OK;
    snprintf(msgbuf.rawdata, sizeof(msgbuf.rawdata), 
			 "OK Ready to receive my message\n");
    write_peer(&msgbuf);
    
	if (read_peer(msgbuf.sockfd, &msgbuf) < 0)
        goto receive_failed;
    if (msgbuf.type != MSGD_MSG)
        goto receive_failed;
    /*
     * rawdata should be "MSG msgstr\n" 
     */
    if ((ptr = strchr(msgbuf.rawdata, ' ')) == NULL)
        goto receive_failed;
    *ptr++ = '\0';
    if ((ptr2 = strrchr(ptr, '\n')) != NULL)
        *ptr2 = '\0';
    strncpy(msg, ptr, MSG_LEN);
    msg[MSG_LEN] = '\0';
    close(msgbuf.sockfd);
    return 0;
  receive_failed:
    close(msgbuf.sockfd);
    return -1;
}
int store_msgfile(char *uident, char *msgbuf)
{
    char buf[STRLEN];
    FILE *fp;

    sethomefile(buf, uident, "msgfile");
    if ((fp = fopen(buf, "a")) == NULL)
        return -1;
    fputs(msgbuf, fp);
    fclose(fp);
    return 0;
}

int msg_can_sendmsg(char *userid, int utmpnum)
{
    struct userec *x;
    struct user_info *uin;

    if (getuser(userid, &x) == 0)
        return 0;
    if (strcmp(x->userid, "guest") && !HAS_PERM(currentuser, PERM_PAGE))
        return 0;
    if (utmpnum == 0)
        uin = t_search(userid, utmpnum);
    else
        uin = get_utmpent(utmpnum);
    if (uin == NULL)
        return 0;
    if (strcasecmp(uin->userid, userid))
        return 0;
    if (!canmsg(currentuser, uin))
        return 0;

    return 1;
}

#if HAVE_MYSQL == 1
MYSQL * my_connect_mysql(MYSQL *s){
	
    return mysql_real_connect(s, 
                            sysconf_str("MYSQLHOST"),
                            sysconf_str("MYSQLUSER"),
			    sysconf_str("MYSQLPASSWORD"),
			    sysconf_str("MYSQLSMSDATABASE"),
			    sysconf_eval("MYSQLPORT",1521), sysconf_str("MYSQLSOCKET"), 0);
}

int save_smsmsg(char *uident, struct msghead *head, char *msgbuf, int readed)
{
	MYSQL s;
	char * newmsgbuf;
	char sql[2600];

	mysql_init(&s);
	if (! my_connect_mysql(&s) ){
#ifdef BBSMAIN
		clear();
		prints("%s\n",mysql_error(&s));
		pressanykey();
#endif
		return -1;
	}

	newmsgbuf=(char *)malloc(strlen(msgbuf)*2+1);
	if(newmsgbuf==NULL) return -1;

	mysql_escape_string(newmsgbuf, msgbuf, strlen(msgbuf));

	sprintf(sql,"INSERT INTO smsmsg VALUES (NULL, '%s', '%s', NULL, %d, '%s', 0 , %d);",uident, head->id, head->sent, newmsgbuf, readed );

	if( mysql_real_query( &s, sql, strlen(sql) )){
#ifdef BBSMAIN
		clear();
		prints("%s\n",mysql_error(&s));
		pressanykey();
#endif
		mysql_close(&s);
		free(newmsgbuf);
		return -1;
	}

	free(newmsgbuf);
	mysql_close(&s);

	return 0;
}

int save_smsmsg_nomysqlconnect(MYSQL *s, char *uident, struct msghead *head, char *msgbuf, int readed)
{
	char * newmsgbuf;
	char sql[2600];

	newmsgbuf=(char *)malloc(strlen(msgbuf)*2+1);
	if(newmsgbuf==NULL) return -1;

	mysql_escape_string(newmsgbuf, msgbuf, strlen(msgbuf));

	sprintf(sql,"INSERT INTO smsmsg VALUES (NULL, '%s', '%s', NULL, %d, '%s', 0 , %d);",uident, head->id, head->sent, newmsgbuf, readed );

	if( mysql_real_query( s, sql, strlen(sql) )){
#ifdef BBSMAIN
		clear();
		prints("%s\n",mysql_error(&s));
		pressanykey();
#endif
		free(newmsgbuf);
		return -1;
	}

	free(newmsgbuf);
	return 0;
}
#endif

int save_msgtext(char *uident, struct msghead * head, char *msgbuf)
{
    char fname[STRLEN], fname2[STRLEN];
    int fd, fd2, i, j, count, size;
    struct flock ldata;
    struct stat buf;

    sethomefile(fname, uident, "msgindex");
    sethomefile(fname2, uident, "msgcontent");

    if ((fd = open(fname, O_WRONLY | O_CREAT, 0664)) == -1) {
        bbslog("user", "%s", "msgopen err");
        return -1;              /* ´´½¨ÎÄ¼þ·¢Éú´íÎó*/
    }
    if ((fd2 = open(fname2, O_WRONLY | O_CREAT, 0664)) == -1) {
        bbslog("user", "%s", "msgopen err");
        close(fd);
        return -1;              /* ´´½¨ÎÄ¼þ·¢Éú´íÎó*/
    }
    ldata.l_type = F_WRLCK;
    ldata.l_whence = 0;
    ldata.l_len = 0;
    ldata.l_start = 0;
    if (fcntl(fd, F_SETLKW, &ldata) == -1) {
        bbslog("user", "%s", "msglock err");
        close(fd2);
        close(fd);
        return -1;              /* lock error*/
    }
    fstat(fd2, &buf);
    size = buf.st_size;
    fstat(fd, &buf);
    count = (buf.st_size-4)/sizeof(struct msghead);
    if(buf.st_size<=0) {
        i = 0;
        write(fd, &i, 4);
        count = 0;
    }
    lseek(fd, count*sizeof(struct msghead)+4, SEEK_SET);
    i = strlen(msgbuf)+1;
    if (i>=MAX_MSG_SIZE) i=MAX_MSG_SIZE-1;
    head->pos = size;
    head->len = i;
    write(fd, head, sizeof(struct msghead));
    lseek(fd2, size, SEEK_SET);
    write(fd2, msgbuf, i);

    close(fd2);
    ldata.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &ldata);
    close(fd);

    if(!head->sent) {
        sethomefile(fname, uident, "msgindex2");
        if ((fd = open(fname, O_WRONLY | O_CREAT, 0664)) == -1) {
            bbslog("user", "%s", "msgopen err");
            return -1;              /* ´´½¨ÎÄ¼þ·¢Éú´íÎó*/
        }
        ldata.l_type = F_WRLCK;
        ldata.l_whence = 0;
        ldata.l_len = 0;
        ldata.l_start = 0;
        if (fcntl(fd, F_SETLKW, &ldata) == -1) {
            bbslog("user", "%s", "msglock err");
            close(fd);
            return -1;              /* lock error*/
        }
        fstat(fd, &buf);
        count = (buf.st_size-4)/sizeof(struct msghead);
        if(buf.st_size<=0) {
            i = 0;
            write(fd, &i, 4);
            count = 0;
        }
        lseek(fd, count*sizeof(struct msghead)+4, SEEK_SET);
        write(fd, head, sizeof(struct msghead));
        ldata.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW, &ldata);
        close(fd);
    }

    return 0;
}

int get_msgcount(int id, char *uident)
{
    char fname[STRLEN], idname[10];
    int i, j, count;
    struct stat buf;

    if(id) sprintf(idname, "msgindex%d", id+1);
    else strcpy(idname, "msgindex");
    sethomefile(fname, uident, idname);

    if(stat(fname, &buf)) return 0;
    count = (buf.st_size-4)/sizeof(struct msghead);
    if (buf.st_size<=0) count=0;

    return count;
}

int clear_msg(char *uident)
{
    char fname[STRLEN];

    sethomefile(fname, uident, "msgindex");
    my_unlink(fname);
    sethomefile(fname, uident, "msgindex2");
    my_unlink(fname);
    sethomefile(fname, uident, "msgcontent");
    my_unlink(fname);

    return 0;
}

int get_unreadmsg(char *uident)
{
    char fname[STRLEN];
    int fd, i, j, count, ret;
    struct flock ldata;
    struct stat buf;

    sethomefile(fname, uident, "msgindex2");

    if ((fd = open(fname, O_RDWR | O_CREAT, 0664)) == -1) {
        bbslog("user", "%s", "msgopen err");
        return -1;              /* ´´½¨ÎÄ¼þ·¢Éú´íÎó*/
    }
    ldata.l_type = F_RDLCK;
    ldata.l_whence = 0;
    ldata.l_len = 0;
    ldata.l_start = 0;
    if (fcntl(fd, F_SETLKW, &ldata) == -1) {
        bbslog("user", "%s", "msglock err");
        close(fd);
        return -1;              /* lock error*/
    }
    fstat(fd, &buf);
    count = (buf.st_size-4)/sizeof(struct msghead);
    if (buf.st_size<=0) ret = -1;
    else {
        read(fd, &ret, 4);
        if(ret >= count) ret = -1;
        else {
            i = ret+1;
            lseek(fd, 0, SEEK_SET);
            write(fd, &i, 4);
        }
    }

    ldata.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &ldata);
    close(fd);
    return ret;
}

int get_unreadcount(char *uident)
{
    char fname[STRLEN];
    int fd, i, j, count, ret;
    struct flock ldata;
    struct stat buf;

    sethomefile(fname, uident, "msgindex2");

    if ((fd = open(fname, O_RDONLY, 0664)) == -1) {
        return 0;              /* ´´½¨ÎÄ¼þ·¢Éú´íÎó*/
    }
    ldata.l_type = F_RDLCK;
    ldata.l_whence = 0;
    ldata.l_len = 0;
    ldata.l_start = 0;
    if (fcntl(fd, F_SETLKW, &ldata) == -1) {
        bbslog("user", "%s", "msglock err");
        close(fd);
        return 0;              /* lock error*/
    }
    fstat(fd, &buf);
    count = (buf.st_size-4)/sizeof(struct msghead);
    if (buf.st_size<=0) ret = 0;
    else {
        read(fd, &ret, 4);
        if(ret >= count) ret = 0;
        else
            ret = count-ret;
    }

    ldata.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &ldata);
    close(fd);
    return ret;
}

int load_msghead(int id, char *uident, int index, struct msghead *head)
{
    char fname[STRLEN], idname[10];
    int fd, i, j, count, now, next;
    struct flock ldata;
    struct stat buf;

    if(id) sprintf(idname, "msgindex%d", id+1);
    else strcpy(idname, "msgindex");
    sethomefile(fname, uident, idname);

    if ((fd = open(fname, O_RDONLY, 0664)) == -1) {
        bbslog("user", "%s", "msgopen err");
        return -1;              /* ´´½¨ÎÄ¼þ·¢Éú´íÎó*/
    }
    ldata.l_type = F_RDLCK;
    ldata.l_whence = 0;
    ldata.l_len = 0;
    ldata.l_start = 0;
    if (fcntl(fd, F_SETLKW, &ldata) == -1) {
        bbslog("user", "%s", "msglock err");
        close(fd);
        return -1;              /* lock error*/
    }
    fstat(fd, &buf);
    count = (buf.st_size-4)/sizeof(struct msghead);
    if(index<0||index>=count) {
        ldata.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW, &ldata);
        close(fd);
        return -1;
    }
    lseek(fd, index*sizeof(struct msghead)+4, SEEK_SET);
    read(fd, head, sizeof(struct msghead));

    ldata.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &ldata);
    close(fd);
    return 0;
}

int load_msgtext(char *uident, struct msghead *head, char *msgbuf)
{
    char fname2[STRLEN];
    int fd2, i, j;

    sethomefile(fname2, uident, "msgcontent");

    msgbuf[0] = 0;

    if ((fd2 = open(fname2, O_RDONLY, 0664)) == -1) {
        bbslog("user", "%s", "msgopen err");
        return -1;              /* ´´½¨ÎÄ¼þ·¢Éú´íÎó*/
    }
    lseek(fd2, head->pos, SEEK_SET);
    i = head->len;
    if(i >= MAX_MSG_SIZE) i=MAX_MSG_SIZE-1;
    read(fd2, msgbuf, i);
    msgbuf[i] = 0;

    close(fd2);
    return 0;
}

int sendmsgfunc(struct user_info *uentp, const char *msgstr, int mode)
{
    char uident[STRLEN];
    FILE *fp;
    time_t now;
    struct user_info *uin;
    char buf[80], *timestr;
    struct msghead head, head2;
    int msg_count = 0;

    *msgerr = 0;
    uin = uentp;
    strcpy(uident, uin->userid);
    if (!HAS_PERM(currentuser, PERM_SEECLOAK) && uin->invisible && strcmp(uin->userid, currentuser->userid) && mode != 4)
        return -2;
    if ((mode != 3) && (LOCKSCREEN == uin->mode)) {     /* Leeward 98.02.28 */
        strcpy(msgerr, "¶Ô·½ÒÑ¾­Ëø¶¨ÆÁÄ»£¬ÇëÉÔºòÔÙ·¢»ò¸øËû(Ëý)Ð´ÐÅ...");
        return -1;
    }
//    if ((mode != 3) && (uin->mode == BBSNET)) /* flyriver, 2002.9.12 */
//    {
//	strcpy(msgerr, "¶Ô·½ÉÐÓÐÒ»Ð©Ñ¶Ï¢Î´´¦Àí£¬ÇëÉÔºòÔÙ·¢»ò¸øËû(Ëý)Ð´ÐÅ...");
//       return -1;
//    }
    if ((mode != 3) && (false == canIsend2(currentuser,uin->userid))) {     /*Haohmaru.06.06.99.¼ì²é×Ô¼ºÊÇ·ñ±»ignore */
        strcpy(msgerr, "¶Ô·½¾Ü¾ø½ÓÊÜÄãµÄÑ¶Ï¢...");
        return -1;
    }
    if (mode != 3 && uin->mode != WEBEXPLORE) {
        if (get_unreadcount(uident) > MAXMESSAGE) {
            strcpy(msgerr, "¶Ô·½ÉÐÓÐÒ»Ð©Ñ¶Ï¢Î´´¦Àí£¬ÇëÉÔºòÔÙ·¢»ò¸øËû(Ëý)Ð´ÐÅ...");
            return -1;
        }
    }
    if (msgstr == NULL) {
        return 0;
    }
    head.time = time(0);
    head.sent = 0;
    head.mode = mode;
    strncpy(head.id, currentuser->userid, IDLEN+2);
    head.frompid = getuinfopid();
    head.topid = uin->pid;
    memcpy(&head2, &head, sizeof(struct msghead));
    head2.sent = 1;
    strncpy(head2.id, uident, IDLEN+2);
    
    if (uin->mode == WEBEXPLORE) {
        if (send_webmsg(get_utmpent_num(uin), uident, utmpent, 
						currentuser->userid, head.time, msgstr) < 0) {
            strcpy(msgerr, "ÎÞ·¨·¢ËÍWebÏûÏ¢...");
            return -1;
        }
        if (save_msgtext(uident, &head, msgstr) < 0)
            return -2;
        //save_smsmsg(uident, &head, msgstr, 1);
        if (strcmp(currentuser->userid, uident)&&mode!=3) {
            if (save_msgtext(currentuser->userid, &head2, msgstr) < 0)
                return -2;
            //save_smsmsg(currentuser->userid, &head2, msgstr, 1) ;
        }
        return 1;
    }

    uin = t_search(MsgDesUid, uentp->pid);
    if ((uin == NULL) || (uin->active == 0) || (uin->pid == 0) || ((kill(uin->pid, 0) != 0) && (uentp->pid != 1))
        || strncasecmp(MsgDesUid,uident,STRLEN)) {
        if (mode == 0)
            return -2;
        strcpy(msgerr, "¶Ô·½ÒÑ¾­ÀëÏß....");
        return -1;
    }

    if (save_msgtext(uident, &head, msgstr) < 0)
        return -2;
    //save_smsmsg(uident, &head, msgstr, 1) ;
    if (strcmp(currentuser->userid, uident)&&mode!=3) {
        if (save_msgtext(currentuser->userid, &head2, msgstr) < 0)
            return -2;
        //save_smsmsg(currentuser->userid, &head2, msgstr, 1) ;
    }
    if (uentp->pid != 1 && kill(uin->pid, SIGUSR2) == -1) {
        strcpy(msgerr, "¶Ô·½ÒÑ¾­ÀëÏß.....");
        return -1;
    }
    return 1;
}

int translate_msg(char* src, struct msghead *head, char* dest)
{
    char id[14], *time, attstr[STRLEN];
    int i,j=0,len,pos,ret=0;
    time = ctime(&head->time);
    dest[0] = 0;
    switch(head->mode) {
        case 0:
        case 2:
        case 4:
            if(!head->sent) {
                sprintf(dest, "[44%sm\x1b[36m%-14.14s[33m(%-16.16s)[37m[K[m\n", DEFINE(currentuser, DEF_HIGHCOLOR)?";1":"", head->id, time);
                sprintf(attstr, "[44%sm[37m", DEFINE(currentuser, DEF_HIGHCOLOR)?";1":"");
            }
            else {
                sprintf(dest, "\x1b[0;1;32m=>[37m%-12.12s[33m(%-16.16s)[36m[K[m\n", head->id, time);
                sprintf(attstr, "[36;1m");
            }
            break;
        case 3:
            sprintf(dest, "[44%sm\x1b[33mÕ¾³¤ÓÚ %16.16s Ê±¹ã²¥[37m[K[m\n", DEFINE(currentuser, DEF_HIGHCOLOR)?";1":"", time);
            sprintf(attstr, "[44%sm[37m", DEFINE(currentuser, DEF_HIGHCOLOR)?";1":"");
            break;
        case 1:
            if(!head->sent) {
                sprintf(dest, "[44%sm\x1b[36m%-12.12s(%-16.16s) ÑûÇëÄã[37m[K[m\n", DEFINE(currentuser, DEF_HIGHCOLOR)?";1":"", head->id, time);
                sprintf(attstr, "[44%sm[37m", DEFINE(currentuser, DEF_HIGHCOLOR)?";1":"");
            }
            else {
                sprintf(dest, "[44%sm\x1b[37mÄã(%-16.16s) ÑûÇë%-12.12s[36m[K[m\n", DEFINE(currentuser, DEF_HIGHCOLOR)?";1":"", time, head->id);
                sprintf(attstr, "[44%sm[36m", DEFINE(currentuser, DEF_HIGHCOLOR)?";1":"");
            }
//            space=33;
            break;
        case 5:
            sprintf(dest, "[45%sm\x1b[36m%-14.14s\x1b[33m(\x1b[36m%-16.16s\x1b[33m)\x1b[37m[K[m\n", DEFINE(currentuser, DEF_HIGHCOLOR)?";1":"", head->id, time);
            sprintf(attstr, "[45%sm[37m", DEFINE(currentuser, DEF_HIGHCOLOR)?";1":"");
//            space=29;
            break;
        case 6:
            if(!head->sent) {
                sprintf(dest, "[44%sm\x1b[36m¶ÌÐÅ     %-14.14s[33m(%-16.16s)[37m[K[m\n", DEFINE(currentuser, DEF_HIGHCOLOR)?";1":"", head->id, time);
                sprintf(attstr, "[44%sm[37m", DEFINE(currentuser, DEF_HIGHCOLOR)?";1":"");
            }
            else {
                sprintf(dest, "\x1b[0;1;32m¶ÌÐÅ   =>[37m%-12.12s[33m(%-16.16s)[36m[K[m\n", head->id, time);
                sprintf(attstr, "[36;1m");
            }
            break;
    }
    strcat(dest, attstr);
    len = strlen(dest);
    pos = 0;
    for(i=0;i<strlen(src);i++){
        if(j) j=0;
        else if(src[i]<0) j=1;
        if(j==0&&pos>=80||j==1&&pos>=79||src[i]=='\n'||src[i]=='\r') {
            dest[len++]='';
            dest[len++]='[';
            dest[len++]='K';
            dest[len++]='';
            dest[len++]='[';
            dest[len++]='m';
            dest[len++]='\n';
            ret++;
            for(pos=0;pos<strlen(attstr);pos++)
                dest[len++]=attstr[pos];
            pos=0;
            if(src[i]=='\n'||src[i]=='\r') continue;
        }
        dest[len++]=src[i];
        pos++;
    }
    dest[len++]='';
    dest[len++]='[';
    dest[len++]='K';
    dest[len++]='';
    dest[len++]='[';
    dest[len++]='m';
    dest[len++]='\n';
    dest[len]=0;
    return ret+2;
}

void mail_msg(struct userec* user)
{
    char fname[MAXPATH];
    char buf[MAX_MSG_SIZE],showmsg[MAX_MSG_SIZE*2];
    int i;
    struct msghead head;
    time_t now;
    char title[STRLEN];
    FILE* fn;
    int count;

	gettmpfilename(fname, "mailmsg" );
    //sprintf(fname, "tmp/%s.msg", user->userid);
    fn = fopen(fname, "w");
    count = get_msgcount(0, user->userid);
    for(i=0;i<count;i++) {
        load_msghead(0, user->userid, i, &head);
        load_msgtext(user->userid, &head, buf);
        translate_msg(buf, &head, showmsg);
        fprintf(fn, "%s", showmsg);
    }
    fclose(fn);

    now = time(0);
    sprintf(title, "[%12.12s] ËùÓÐÑ¶Ï¢±¸·Ý", ctime(&now) + 4);
    mail_file(user->userid, fname, user->userid, title, BBSPOST_MOVE, NULL);
    unlink(fname);
    clear_msg(user->userid);
}

#ifdef SMS_SUPPORT

void * smsbuf=NULL;
int smsresult=0;
struct user_info * smsuin;

int sms_init_memory()
{
    void * p;
    int iscreate;
    if(smsbuf) return 0;

    iscreate = 0;
    p = attach_shm("SMS_SHMKEY", 8914, SMS_SHM_SIZE+sizeof(struct sms_shm_head), &iscreate);
    head = (struct sms_shm_head *) p;
    smsbuf = p+sizeof(struct sms_shm_head);
}

void sendtosms(void * n, int s)
{
    if(head->length+s>=SMS_SHM_SIZE) return;
    memcpy(smsbuf+head->length, n, s);
    head->length+=s;
}

void SMS_request(int signo)
{
    char fn[80];
    struct stat st;
//	gettmpfilename( fn, "sms.res");
    sprintf(fn, "tmp/%d.res", smsuin->pid);
    if(stat(fn, &st)!=-1)
        smsresult=1;
}

int wait_for_result()
{
    int count;
    char fn[80];
    FILE* fp;
    int i,j;
    signal(SIGUSR1, SMS_request);
//	gettmpfilename( fn, "sms.res");
    sprintf(fn, "tmp/%d.res", smsuin->pid);
    unlink(fn);
    smsresult = 0;
    head->sem=0;

    count=0;
    while(!smsresult) {
#ifdef BBSMAIN
        move(t_lines-1, 0);
        clrtoeol();
        prints("·¢ËÍÖÐ....%d%%", count*100/30);
        refresh();
#else
		{
    		struct stat st;
    		if(stat(fn, &st)!=-1 && st.st_size > 0){
				smsresult = 1;
				break;
			}
		}
#endif
        sleep(1);
        count++;
        if(count>30) {
#ifdef BBSMAIN
            move(t_lines-1, 0);
            clrtoeol();
#endif
            return -1;
        }
    }
#ifdef BBSMAIN
    move(t_lines-1, 0);
    clrtoeol();
#endif
    fp=fopen(fn, "r");
    fscanf(fp, "%d%d", &i, &j);
    fclose(fp);
    if(i==1) return 0;
    else return -j;
}

int DoReg(char * n)
{
    int count=0;
    struct header h;
    struct RegMobileNoPacket h1;
    h.Type = CMD_REG;
    long2byte(smsuin->pid, h.pid);
    long2byte(sizeof(h1), h.BodyLength);
    strcpy(h1.MobileNo, n);
    uid2smsid(smsuin,h1.cUserID);
    while(head->sem) {
        sleep(1);
        count++;
        if(count>=5) return -1;
    }
    head->sem=1;
    head->total++;
    sendtosms(&h, sizeof(h));
    sendtosms(&h1, sizeof(h1));
    return wait_for_result();
}

int DoUnReg(char * n)
{
    int count=0;
    struct header h;
    struct UnRegPacket h1;
    h.Type = CMD_UNREG;
    long2byte(smsuin->pid, h.pid);
    long2byte(sizeof(h1), h.BodyLength);
    strcpy(h1.MobileNo, n);
    uid2smsid(smsuin,h1.cUserID);
    while(head->sem) {
        sleep(1);
        count++;
        if(count>=5) return -1;
    }
    head->sem=1;
    head->total++;
    sendtosms(&h, sizeof(h));
    sendtosms(&h1, sizeof(h1));
    return wait_for_result();
}

int DoCheck(char * n, char * c)
{
    int count=0;
    struct header h;
    struct CheckMobileNoPacket h1;
    h.Type = CMD_CHECK;
    long2byte(smsuin->pid, h.pid);
    long2byte(sizeof(h1), h.BodyLength);
    strcpy(h1.MobileNo, n);
    strcpy(h1.ValidateNo, c);
    uid2smsid(smsuin,h1.cUserID);
    while(head->sem) {
        sleep(1);
        count++;
        if(count>=5) return -1;
    }
    head->sem=1;
    head->total++;
    sendtosms(&h, sizeof(h));
    sendtosms(&h1, sizeof(h1));
    return wait_for_result();
}

int DoSendSMS(char * n, char * d, char * c)
{
    int count=0;
    int ret;
    struct header h;
    struct BBSSendSMS h1;
    int number;
    h.Type = CMD_BBSSEND;
    long2byte(smsuin->pid, h.pid);
    long2byte(sizeof(h1)+strlen(c)+1, h.BodyLength);
    long2byte(strlen(c)+1, h1.MsgTxtLen);
    strcpy(h1.SrcMobileNo, n);
    strcpy(h1.DstMobileNo, d);
    uid2smsid(smsuin,h1.SrccUserID);
    number=uid2smsnumber(smsuin);
    long2byte(number, h1.UserID);
    while(head->sem) {
        sleep(1);
        count++;
        if(count>=5) return -1;
    }
    head->sem=1;
    head->total++;
    sendtosms(&h, sizeof(h));
    sendtosms(&h1, sizeof(h1));
    sendtosms(c, strlen(c)+1);
    ret= wait_for_result();
    newbbslog(BBSLOG_SMS,"send %s for %s src %s ret %d",c,d,n,ret);
    return ret;
}

int DoReplyCheck(char * n, unsigned int sn, char isSucceed)
{
    int count=0;
    struct header h;
    struct ReplyBindPacket h1;
    h.Type = CMD_REPLY;
    long2byte(smsuin->pid, h.pid);
    long2byte(sizeof(h1), h.BodyLength);
    long2byte(sn, h.SerialNo);
    strcpy(h1.MobileNo, n);
    h1.isSucceed = isSucceed;
    while(head->sem) {
        sleep(1);
        count++;
        if(count>=5) return -1;
    }
    head->sem=1;
    head->total++;
    sendtosms(&h, sizeof(h));
    sendtosms(&h1, sizeof(h1));
    return wait_for_result();
}

#if HAVE_MYSQL == 1
int count_sql_smsmsg( char *userid, char *dest, time_t start_time, time_t end_time, int type, int level, char *msgtxt )
{

	MYSQL s;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char sql[600];
	char qtmp[100];
	int i;

	if(userid == NULL || *userid == 0 )
		return -1;

	mysql_init(&s);

	if (! my_connect_mysql(&s) ){
#ifdef BBSMAIN
		clear();
		prints("%s\n",mysql_error(&s));
		pressanykey();
#endif
		return -1;
	}

	sprintf(sql,"SELECT COUNT(*) FROM smsmsg WHERE userid=\"%s\"", userid );

	if(dest && *dest){
		snprintf(qtmp, 99, " AND dest=\"%s\"", dest);
		strcat(sql, qtmp);
	}

	if(start_time){
		snprintf(qtmp, 99, " AND timestamp>FROM_UNIXTIME(%lu)+0", start_time);
		strcat(sql, qtmp);
	}

	if(end_time){
		snprintf(qtmp, 99, " AND timestamp<FROM_UNIXTIME(%lu)+0", end_time);
		strcat(sql, qtmp);
	}

	if(type != -1){
		snprintf(qtmp, 99, " AND type=\"%d\"", type);
		strcat(sql, qtmp);
	}

	if(msgtxt && msgtxt[0]){
		char newmsgtxt[60];
		int ii,jj;
		for(ii=0,jj=0; msgtxt[ii] && jj<60; ii++){
			if(msgtxt[ii] == '\'' || msgtxt[ii]=='\"')
				newmsgtxt[jj++]='\\';
			newmsgtxt[jj++]=msgtxt[ii];
		}
		snprintf(qtmp, 99, " AND context LIKE \"%%%s%%\"", newmsgtxt);
		strcat(sql, qtmp);
	}

	if( mysql_real_query(&s, sql, strlen(sql)) ){
#ifdef BBSMAIN
		clear();
		prints("%s\n",mysql_error(&s));
		pressanykey();
#endif
		mysql_close(&s);
		return -1;
	}
	res = mysql_store_result(&s);
	row = mysql_fetch_row(res);

	i=0;
	if(row != NULL){
		i = atoi(row[0]);
	}
	mysql_free_result(res);

	mysql_close(&s);
	return i;
}

int get_sql_smsmsg( struct smsmsg * smdata, char *userid, char *dest, time_t start_time, time_t end_time, int type, 					int level, int start, int num, char *msgtxt, int desc)
{

	MYSQL s;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char sql[600];
	char qtmp[100];
	int i;

	if(userid == NULL || *userid == 0 )
		return -1;

	mysql_init(&s);

	if (! my_connect_mysql(&s) ){
#ifdef BBSMAIN
		clear();
		prints("%s\n",mysql_error(&s));
		pressanykey();
#endif
		return -1;
	}

	sprintf(sql,"SELECT * FROM smsmsg WHERE userid=\"%s\"", userid );

	if(dest && *dest){
		snprintf(qtmp, 99, " AND dest=\"%s\"", dest);
		strcat(sql, qtmp);
	}

	if(start_time){
		snprintf(qtmp, 99, " AND timestamp>FROM_UNIXTIME(%lu)+0", start_time);
		strcat(sql, qtmp);
	}

	if(end_time){
		snprintf(qtmp, 99, " AND timestamp<FROM_UNIXTIME(%lu)+0", end_time);
		strcat(sql, qtmp);
	}

	if(type != -1){
		snprintf(qtmp, 99, " AND type=\"%d\"", type);
		strcat(sql, qtmp);
	}

	if(msgtxt && msgtxt[0]){
		char newmsgtxt[60];
		int ii,jj;
		for(ii=0,jj=0; msgtxt[ii] && jj<60; ii++){
			if(msgtxt[ii] == '\'' || msgtxt[ii]=='\"')
				newmsgtxt[jj++]='\\';
			newmsgtxt[jj++]=msgtxt[ii];
		}
		snprintf(qtmp, 99, " AND context LIKE \"%%%s%%\"", newmsgtxt);
		strcat(sql, qtmp);
	}

	snprintf(qtmp, 99, " ORDER BY readed, timestamp %s LIMIT %d,%d", desc?"DESC":"ASC", start, num);
	strcat(sql, qtmp);

	if( mysql_real_query(&s, sql, strlen(sql)) ){
#ifdef BBSMAIN
		clear();
		prints("%s\n",mysql_error(&s));
		pressanykey();
#endif
		mysql_close(&s);
		return -1;
	}
	res = mysql_store_result(&s);
	row = mysql_fetch_row(res);

	i=0;
	while(row != NULL){
		i++;
		if( i>num )
			break;
		smdata[i-1].id = atoi(row[0]);
		strncpy(smdata[i-1].userid, row[1], 13);
		smdata[i-1].userid[12]=0;
		strncpy(smdata[i-1].dest, row[2], 13);
		smdata[i-1].dest[12]=0;
		strncpy(smdata[i-1].time, row[3], 15);
		smdata[i-1].time[14]=0;
		smdata[i-1].type = atoi(row[4]);
		smdata[i-1].level = atoi(row[6]);
		smdata[i-1].readed = atoi(row[7]);
		smdata[i-1].context = (char *)malloc( strlen(row[5]) + 1 );
		if( smdata[i-1].context != NULL )
			strncpy(smdata[i-1].context, row[5], strlen(row[5])+1);
		row = mysql_fetch_row(res);
	}
	mysql_free_result(res);

	mysql_close(&s);
	return i;
}

int chk_smsmsg(int force ){

	MYSQL s;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char sql[100];
	static int i=0;

	if( ! force )
		return i;

	i=0;
	mysql_init(&s);

	if (! my_connect_mysql(&s) ){
		return 0;
	}

	sprintf(sql, "SELECT * FROM smsmsg WHERE userid='%s' AND readed=0", currentuser->userid);
	if( mysql_real_query(&s, sql, strlen(sql)) ){
		mysql_close(&s);
		return 0;
	}
	res = mysql_store_result(&s);
	row = mysql_fetch_row(res);

	while(row != NULL){
		i++;
		row = mysql_fetch_row(res);
	}
	mysql_free_result(res);

	mysql_close(&s);
	return i;
}

int sign_smsmsg_read(int id ){

	MYSQL s;
	char sql[100];

	mysql_init(&s);

	if (! my_connect_mysql(&s) ){
		return 0;
	}

	sprintf(sql, "UPDATE smsmsg SET readed=1 WHERE id=%d", id);
	if( mysql_real_query(&s, sql, strlen(sql)) ){
		mysql_close(&s);
		return 0;
	}
	mysql_close(&s);
	return 1;
}
#else

int chk_smsmsg(int force ){

		return 0;
}

#endif  //HAVE_MYSQL

#else	//SMS_SUPPORT

int chk_smsmsg(int force ){

		return 0;
}

#endif

#if HAVE_MYSQL == 1
int count_sql_al( char *userid, char *dest, char *group, char *msgtxt)
{

	MYSQL s;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char sql[600];
	char qtmp[100];
	int i;

	if(userid == NULL || *userid == 0 )
		return -1;

	mysql_init(&s);

	if (! my_connect_mysql(&s) ){
#ifdef BBSMAIN
		clear();
		prints("%s\n",mysql_error(&s));
		pressanykey();
#endif
		return -1;
	}

	sprintf(sql,"SELECT COUNT(*) FROM addr WHERE userid=\"%s\"", userid );

	if(dest && *dest){
		snprintf(qtmp, 99, " AND name=\"%s\"", dest);
		strcat(sql, qtmp);
	}

	if(group && *group){
		snprintf(qtmp, 99, " AND groupname=\"%s\"", group);
		strcat(sql, qtmp);
	}

	if(msgtxt && msgtxt[0]){
		char newmsgtxt[60];
		int ii,jj;
		for(ii=0,jj=0; msgtxt[ii] && jj<60; ii++){
			if(msgtxt[ii] == '\'' || msgtxt[ii]=='\"')
				newmsgtxt[jj++]='\\';
			newmsgtxt[jj++]=msgtxt[ii];
		}
		newmsgtxt[jj]=0;
		snprintf(qtmp, 99, " AND memo LIKE \"%%%s%%\"", newmsgtxt);
		strcat(sql, qtmp);
	}

	if( mysql_real_query(&s, sql, strlen(sql)) ){
#ifdef BBSMAIN
		clear();
		prints("%s\n",mysql_error(&s));
		pressanykey();
#endif
		mysql_close(&s);
		return -1;
	}
	res = mysql_store_result(&s);
	row = mysql_fetch_row(res);

	i=0;
	if(row != NULL){
		i = atoi(row[0]);
	}
	mysql_free_result(res);

	mysql_close(&s);
	return i;
}


char * get_al_mobile( char *userid, char *mobile)
{

	MYSQL s;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char sql[300];
	char name1[2*STRLEN+1];

	if(userid == NULL || *userid == 0 )
		return NULL;

	if( mobile == NULL )
		return NULL;

	*mobile = 0;

	mysql_init(&s);

	if (! my_connect_mysql(&s) ){
#ifdef BBSMAIN
		clear();
		prints("%s\n",mysql_error(&s));
		pressanykey();
#endif
		return NULL;
	}

	mysql_escape_string(name1, userid, strlen(userid));
	sprintf(sql,"SELECT mobile FROM addr WHERE userid=\"%s\" AND name=\"%s\"", currentuser->userid, name1 );

	if( mysql_real_query(&s, sql, strlen(sql)) ){
#ifdef BBSMAIN
		clear();
		prints("%s\n",mysql_error(&s));
		pressanykey();
#endif
		mysql_close(&s);
		return NULL;
	}
	res = mysql_store_result(&s);
	row = mysql_fetch_row(res);

	if(row != NULL){
		strncpy(mobile, row[0], 15);
		mobile[14]=0;
	}
	mysql_free_result(res);
	mysql_close(&s);

	return mobile;
}

int get_sql_al( struct addresslist * smdata, char *userid, char *dest, char *group, int start, int num, int order, char *msgtxt)
{

	MYSQL s;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char sql[600];
	char qtmp[100];
	int i;

	if(userid == NULL || *userid == 0 )
		return -1;

	mysql_init(&s);

	if (! my_connect_mysql(&s) ){
#ifdef BBSMAIN
		clear();
		prints("%s\n",mysql_error(&s));
		pressanykey();
#endif
		return -1;
	}

	sprintf(sql,"SELECT * FROM addr WHERE userid=\"%s\"", userid );

	if(dest && *dest){
		snprintf(qtmp, 99, " AND name=\"%s\"", dest);
		strcat(sql, qtmp);
	}

	if(group && *group){
		snprintf(qtmp, 99, " AND groupname=\"%s\"", group);
		strcat(sql, qtmp);
	}

	if(msgtxt && msgtxt[0]){
		char newmsgtxt[60];
		int ii,jj;
		for(ii=0,jj=0; msgtxt[ii] && jj<60; ii++){
			if(msgtxt[ii] == '\'' || msgtxt[ii]=='\"')
				newmsgtxt[jj++]='\\';
			newmsgtxt[jj++]=msgtxt[ii];
		}
		newmsgtxt[jj]=0;
		snprintf(qtmp, 99, " AND memo LIKE \"%%%s%%\"", newmsgtxt);
		strcat(sql, qtmp);
	}

	if( order == AL_ORDER_NAME ){
		snprintf(qtmp, 99, " ORDER BY name");
		strcat(sql, qtmp);
	}else if( order == AL_ORDER_BBSID ){
		snprintf(qtmp, 99, " ORDER BY bbsid");
		strcat(sql, qtmp);
	}else if( order == AL_ORDER_GROUPNAME ){
		snprintf(qtmp, 99, " ORDER BY groupname");
		strcat(sql, qtmp);
	}

	snprintf(qtmp, 99, " LIMIT %d,%d", start, num);
	strcat(sql, qtmp);

	if( mysql_real_query(&s, sql, strlen(sql)) ){
#ifdef BBSMAIN
		clear();
		prints("%s\n",mysql_error(&s));
		pressanykey();
#endif
		mysql_close(&s);
		return -1;
	}
	res = mysql_store_result(&s);
	row = mysql_fetch_row(res);

	i=0;
	while(row != NULL){
		i++;
		if( i>num )
			break;
		smdata[i-1].id = atoi(row[0]);
		strncpy(smdata[i-1].userid, row[1], 13);
		smdata[i-1].userid[12]=0;
		strncpy(smdata[i-1].name, row[2], 15);
		smdata[i-1].name[14]=0;
		strncpy(smdata[i-1].bbsid, row[3], 15);
		smdata[i-1].bbsid[14]=0;
		strncpy(smdata[i-1].school, row[4], 100);
		smdata[i-1].school[99]=0;
		strncpy(smdata[i-1].zipcode, row[5], 6);
		smdata[i-1].zipcode[6]=0;
		strncpy(smdata[i-1].homeaddr, row[6], 100);
		smdata[i-1].homeaddr[99]=0;
		strncpy(smdata[i-1].companyaddr, row[7], 100);
		smdata[i-1].companyaddr[99]=0;
		strncpy(smdata[i-1].tel_o, row[8], 20);
		smdata[i-1].tel_o[19]=0;
		strncpy(smdata[i-1].tel_h, row[9], 20);
		smdata[i-1].tel_h[19]=0;
		strncpy(smdata[i-1].mobile, row[10], 15);
		smdata[i-1].mobile[14]=0;
		strncpy(smdata[i-1].email, row[11], 30);
		smdata[i-1].email[29]=0;
		strncpy(smdata[i-1].qq, row[12], 10);
		smdata[i-1].qq[9]=0;
		strncpy(smdata[i-1].group, row[15], 10);
		smdata[i-1].group[9]=0;
		sscanf(row[13], "%d-%d-%d", &(smdata[i-1].birth_year),&(smdata[i-1].birth_month),&(smdata[i-1].birth_day));
		smdata[i-1].memo = (char *)malloc( strlen(row[14]) + 1 );
		if( smdata[i-1].memo != NULL )
			strncpy(smdata[i-1].memo, row[14], strlen(row[14])+1);
		row = mysql_fetch_row(res);
	}
	mysql_free_result(res);

	mysql_close(&s);
	return i;
}

int add_sql_al(char *userid, struct addresslist *al, char *msgbuf)
{
	MYSQL s;
	char sql[1500];
	char newname[30];
	char newbbsid[30];
	char newschool[200];
	char newzipcode[14];
	char newhomeaddr[200];
	char newcompanyaddr[200];
	char newtel_o[40];
	char newtel_h[40];
	char newmobile[30];
	char newemail[60];
	char newqq[20];
	char newmsgbuf[200];
	char newgroup[20];

	newname[0]=0;
	newbbsid[0]=0;
	newschool[0]=0;
	newzipcode[0]=0;
	newhomeaddr[0]=0;
	newcompanyaddr[0]=0;
	newtel_o[0]=0;
	newtel_h[0]=0;
	newmobile[0]=0;
	newemail[0]=0;
	newqq[0]=0;
	newmsgbuf[0]=0;
	newgroup[0]=0;

	mysql_init(&s);
	if (! my_connect_mysql(&s) ){
#ifdef BBSMAIN
		clear();
		prints("%s\n",mysql_error(&s));
		pressanykey();
#endif
		return 0;
	}

	mysql_escape_string(newname, al->name, strlen(al->name));
	mysql_escape_string(newbbsid, al->bbsid, strlen(al->bbsid));
	mysql_escape_string(newschool, al->school, strlen(al->school));
	mysql_escape_string(newzipcode, al->zipcode, strlen(al->zipcode));
	mysql_escape_string(newhomeaddr, al->homeaddr, strlen(al->homeaddr));
	mysql_escape_string(newcompanyaddr, al->companyaddr, strlen(al->companyaddr));
	mysql_escape_string(newtel_o, al->tel_o, strlen(al->tel_o));
	mysql_escape_string(newtel_h, al->tel_h, strlen(al->tel_h));
	mysql_escape_string(newmobile, al->mobile, strlen(al->mobile));
	mysql_escape_string(newemail, al->email, strlen(al->email));
	mysql_escape_string(newqq, al->qq, strlen(al->qq));
	if( msgbuf )
		mysql_escape_string(newmsgbuf, msgbuf, strlen(msgbuf) > 99 ? 99 : strlen(msgbuf) );
	else 
		newmsgbuf[0]=0;
	mysql_escape_string(newgroup, al->group, strlen(al->group));

	if( al->id <= 0 )
		sprintf(sql,"INSERT INTO addr VALUES (NULL, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', \"%d-%d-%d\", '%s', '%s' );",userid, newname, newbbsid, newschool, newzipcode, newhomeaddr, newcompanyaddr, newtel_o, newtel_h,newmobile, newemail, newqq, al->birth_year, al->birth_month, al->birth_day, newmsgbuf, newgroup );
	else
		sprintf(sql,"UPDATE addr SET userid='%s', name='%s', bbsid='%s', school='%s', zipcode='%s', homeaddr='%s', companyaddr='%s', tel_o='%s', tel_h='%s', mobile='%s', email='%s', qq='%s', birthday=\"%d-%d-%d\", memo='%s', groupname='%s' WHERE id=%d ;",userid, newname, newbbsid, newschool, newzipcode, newhomeaddr, newcompanyaddr, newtel_o, newtel_h,newmobile, newemail, newqq, al->birth_year, al->birth_month, al->birth_day, newmsgbuf, newgroup, al->id );

	if( mysql_real_query( &s, sql, strlen(sql) )){
#ifdef BBSMAIN
		clear();
		prints("%s\n",mysql_error(&s));
		pressanykey();
#endif
		mysql_close(&s);
		return 0;
	}

	mysql_close(&s);

	return 1;
}

#define CSV_LIST_MAX_KEYNUM 100
struct csv_list
{
	struct csv_list * next;
	char *key;
};


static int free_csv_list( struct csv_list * cl)
{
	struct csv_list * nowcl = NULL;
	struct csv_list * cltmp = NULL;

	nowcl = cl;
	while( nowcl ){
		cltmp = nowcl;
		nowcl = nowcl->next;
		if(cltmp->key)
			free(cltmp->key);
		free(cltmp);
	}

	return 1;
}
		

static struct csv_list * read_csv_line(char *ptr, size_t size, size_t *dlength)
{
	struct csv_list * cl = NULL;
	struct csv_list * nowcl = NULL;
	int start=0;
	char *p = ptr;
	size_t sz=0;
	int i,have_quota;
	int clnum = 0;

	*dlength=0;
	for( ; sz < size ; sz++, p++ ){
		if( *p == '\n' || *p == '\r' || *p == 0 ){
			if( cl ){
				sz++;
				break;
			}
		}
		if( ! start ){
			start=1;
			cl = (struct csv_list *)malloc( sizeof(struct csv_list) );
			if( cl == NULL )
				return cl;
			bzero(cl, sizeof(struct csv_list));
			nowcl = cl;
			clnum ++;
		}else if( clnum < CSV_LIST_MAX_KEYNUM) {
			nowcl->next = (struct csv_list *)malloc( sizeof(struct csv_list) );
			if( nowcl->next == NULL )
				return cl;
			bzero(nowcl->next, sizeof(struct csv_list));
			nowcl = nowcl->next;
			clnum ++;
		}

		i=0;
		have_quota=0;
		if( *p == '\"' ){
			have_quota=2;
			for( i++; sz+i < size; i++ ){
				if ( p[i] == '\"'){
					if( p[i+1] != '\"' )
						break;
					else{
						i++;
					}
				}
			}
		}
		for(; sz+i < size; i++){
			if( p[i] == ',' || p[i] == '\n' || p[i] == '\r' || p[i] == 0)
				break;
		}
		if( i - have_quota > 0 && clnum < CSV_LIST_MAX_KEYNUM){
			nowcl->key = (char *)malloc( i+1 );
			if( nowcl->key == NULL )
				return cl;
			memcpy(nowcl->key, p, i);
			nowcl->key[i]=0;
			if( have_quota ){
				int j,k;
				for(j=0, k=1; k<i; k++){
					if( nowcl->key[k] == '\"'){
						if( nowcl->key[k+1] == '\"' ){
							k++;
							nowcl->key[j]='\"';
							j++;
						}
					}else{
						nowcl->key[j]= nowcl->key[k];
						j++;
					}
				}
				nowcl->key[j]=0;
			}
		}
		sz += i;
		p += i;
		if( *p == '\n' || *p == '\r' || *p == 0 ){
			sz++;
			break;
		}
	}

	*dlength = sz;

	return cl;

}

#define CSV_HASH_NAME 1
#define CSV_HASH_SCHOOL 2
#define CSV_HASH_ZIPCODE 3
#define CSV_HASH_HOMEADDR 4
#define CSV_HASH_COMPANYADDR 5
#define CSV_HASH_TELO 6
#define CSV_HASH_TELH 7
#define CSV_HASH_MOBILE 8
#define CSV_HASH_EMAIL 9
#define CSV_HASH_QQ 10
#define CSV_HASH_MEMO 11
int conv_csv_to_al(char *fname)
{
	FILE *fp;
	char *ptr;
	char *p;
	size_t size;
	size_t sz;
	size_t dlength;
	struct csv_list *cl;
	struct csv_list *nowcl;
	struct addresslist al;
	int i,j,first,csv_hash[CSV_LIST_MAX_KEYNUM];
	int ret=0;

    if ((fp = fopen(fname, "r+b")) == NULL) {
		return 0;
	}
	first=0;
	for( i=0; i< CSV_LIST_MAX_KEYNUM; i++)
		csv_hash[i]=0;

	if (safe_mmapfile_handle(fileno(fp), O_RDWR, PROT_READ | PROT_WRITE, MAP_SHARED, (void **)(&ptr) , (size_t *) & size) == 1) {

		sz = size;
		p=ptr;
		while( sz > 0 ){
			cl = read_csv_line(p, sz, &dlength);
			if( cl == NULL || dlength == -1 ){
				free_csv_list( cl );
				break;
			}

			if( ! first ){
				first=1;
				nowcl = cl;
				for(i=0; nowcl && i<CSV_LIST_MAX_KEYNUM ; nowcl=nowcl->next, i++){
					if( nowcl->key == NULL )
						continue;
					if( ! strcmp(nowcl->key,"ÐÕÃû") )
						csv_hash[i] = CSV_HASH_NAME ;
					else if( strstr(nowcl->key, "µç×ÓÓÊ¼þ") )
						csv_hash[i] = CSV_HASH_EMAIL ;
					else if( !strcmp(nowcl->key, "Ñ§Ð£") )
						csv_hash[i] = CSV_HASH_SCHOOL ;
					else if( !strcasecmp(nowcl->key, "QQ") )
						csv_hash[i] = CSV_HASH_QQ ;
					else if( strstr(nowcl->key, "ÒÆ¶¯µç»°") || strstr(nowcl->key,"ÊÖ»ú") )
						csv_hash[i] = CSV_HASH_MOBILE ;
					else if( strstr(nowcl->key, "¼ÒÍ¥µç»°") )
						csv_hash[i] = CSV_HASH_TELO ;
					else if( strstr(nowcl->key, "°ì¹«µç»°") || strstr(nowcl->key, "ÉÌÎñµç»°") )
						csv_hash[i] = CSV_HASH_TELH ;
					else if( strstr(nowcl->key, "ÓÊÕþ±àÂë") )
						csv_hash[i] = CSV_HASH_ZIPCODE ;
					else if( strstr(nowcl->key, "¼ÒÍ¥ËùÔÚ") )
						csv_hash[i] = CSV_HASH_HOMEADDR ;
					else if( strstr(nowcl->key, "¹«Ë¾") )
						csv_hash[i] = CSV_HASH_COMPANYADDR ;
					else if( !strcmp(nowcl->key, "¸½×¢") )
						csv_hash[i] = CSV_HASH_MEMO ;
				}
			}else{
				char * pmemo=NULL;

				bzero(&al, sizeof(al));
				nowcl = cl;
				for(i=0; nowcl && i<CSV_LIST_MAX_KEYNUM ; nowcl=nowcl->next, i++){
					if( nowcl->key == NULL )
						continue;
					switch( csv_hash[i] ){
					case CSV_HASH_NAME:
						strncpy( al.name, nowcl->key, 14 );
						al.name[14]=0;
						break;
					case CSV_HASH_EMAIL:
						strncpy( al.email, nowcl->key, 30 );
						al.email[29]=0;
						break;
					case CSV_HASH_SCHOOL:
						snprintf(al.school, 99, "%s %s",al.school, nowcl->key);
						al.school[99]=0;
						break;
					case CSV_HASH_HOMEADDR:
						snprintf(al.homeaddr, 99, "%s %s",al.homeaddr, nowcl->key);
						al.homeaddr[99]=0;
						break;
					case CSV_HASH_COMPANYADDR:
						snprintf(al.companyaddr, 99, "%s %s",al.companyaddr, nowcl->key);
						al.companyaddr[99]=0;
						break;
					case CSV_HASH_QQ:
						strncpy( al.qq, nowcl->key, 9 );
						al.qq[9]=0;
						break;
					case CSV_HASH_MOBILE:
						strncpy( al.mobile, nowcl->key, 15 );
						al.mobile[14]=0;
						break;
					case CSV_HASH_TELO:
						strncpy( al.tel_o, nowcl->key, 19 );
						al.tel_o[19]=0;
						break;
					case CSV_HASH_TELH:
						strncpy( al.tel_h, nowcl->key, 19 );
						al.tel_h[19]=0;
						break;
					case CSV_HASH_ZIPCODE:
						strncpy( al.zipcode, nowcl->key, 6 );
						al.zipcode[6]=0;
						break;
					case CSV_HASH_MEMO:
						pmemo = nowcl->key;
						break;
					}
				}
				strncpy(al.userid, currentuser->userid, 12);
				al.userid[12]=0;

				if( al.name[0] ){
					if(add_sql_al(currentuser->userid, &al, pmemo))
						ret++;
				}
			}
/*
			while(nowcl){
				printf(":");
				if(nowcl->key)
					printf("%s",nowcl->key);
				nowcl = nowcl->next;
			}
			printf("\n");
*/
			free_csv_list( cl );
			sz -= dlength;
			p += dlength;
		}

	}

	fclose(fp);

	end_mmapfile((void *) ptr, size, -1);

	return ret;
}
	
#endif
