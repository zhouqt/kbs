#include "bbs.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <assert.h>
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
    struct sockaddr_un sun;
    int sockfd;
    char path[80];

    bzero(&sun, sizeof(sun));
    snprintf(path, sizeof(path), BBSHOME "/.msgd");
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd == -1)
        return -1;
    sun.sun_family = AF_UNIX;
    strncpy(sun.sun_path, path, sizeof(sun.sun_path) - 1);
    if (connect(sockfd, (struct sockaddr *) &sun, sizeof(sun)) < 0) {
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
    unlink(fname);
    sethomefile(fname, uident, "msgindex2");
    unlink(fname);
    sethomefile(fname, uident, "msgcontent");
    unlink(fname);

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
        if (strcmp(currentuser->userid, uident)&&mode!=3) {
            if (save_msgtext(currentuser->userid, &head2, msgstr) < 0)
                return -2;
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
    if (strcmp(currentuser->userid, uident)&&mode!=3) {
        if (save_msgtext(currentuser->userid, &head2, msgstr) < 0)
            return -2;
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

unsigned int byte2long(byte arg[4]) {
    unsigned int tmp;
    tmp=((long)arg[0]<<24)+((long)arg[1]<<16)+((long)arg[2]<<8)+(long)arg[3];
    return tmp;
}

void long2byte(unsigned int num, byte* arg) {
    (arg)[0]=num>>24;
    (arg)[1]=(num<<8)>>24;
    (arg)[2]=(num<<16)>>24;
    (arg)[3]=(num<<24)>>24;
}

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
    strcpy(h1.cUserID, smsuin->userid);
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
    strcpy(h1.cUserID, smsuin->userid);
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
    strcpy(h1.cUserID, smsuin->userid);
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
    struct header h;
    struct BBSSendSMS h1;
    h.Type = CMD_BBSSEND;
    long2byte(smsuin->pid, h.pid);
    long2byte(sizeof(h1)+strlen(c)+1, h.BodyLength);
    long2byte(strlen(c)+1, h1.MsgTxtLen);
    long2byte(smsuin->uid, h1.UserID);
    strcpy(h1.SrcMobileNo, n);
    strcpy(h1.DstMobileNo, d);
    strcpy(h1.SrccUserID, smsuin->userid);
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
    return wait_for_result();
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

#endif

