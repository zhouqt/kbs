#include "bbs.h"
#include "screen.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <assert.h>

#define MAXMESSAGE 5
char MsgDesUid[20];
char msgerr[255];

int getuinfopid(void);

int write_peer(msg_t *msgbuf)
{
	char buf[2*STRLEN];

	assert(msgbuf != NULL);
	snprintf(buf, sizeof(buf), "%d %s", msgbuf->type, msgbuf->rawdata);
	return write(msgbuf->sockfd, buf, strlen(buf));
}

int read_peer(int sockfd, msg_t *msgbuf)
{
	char buf[2*STRLEN];
	char *ptr;
	int rv;
	int rc;

	/*if (msgbuf == NULL)
		return -1;*/
	/* assert() macro can be removed by -DNDEBUG */
	assert(msgbuf != NULL);
	msgbuf->sockfd = sockfd;
	if ((rc = read(sockfd, buf, sizeof(buf)-1)) < 0)
		return -1;
	buf[rc] = '\0';
	msgbuf->type = atoi(buf);
	ptr = strchr(buf, ' ');
	if (ptr == NULL)
	{
		msgbuf->rawdata[0] = '\0';
		rv = -1;
	}
	else
	{
		strcpy(msgbuf->rawdata, ptr+1);
		rv = 0;
	}

	return rv;
}

int get_request_type(msg_t *msgbuf)
{
	assert(msgbuf != NULL);
	return msgbuf->type;
}

int get_response_type(msg_t *msgbuf)
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
	snprintf(path, sizeof(path), BBSHOME"/.msgd");
	sockfd = socket(AF_LOCAL, SOCK_STREAM, 0);
	if (sockfd == -1)
		return -1;
	sun.sun_family = AF_LOCAL;
	strncpy(sun.sun_path, path, sizeof(sun.sun_path)-1);
	if (connect(sockfd, (struct sockaddr *)&sun, sizeof(sun)) < 0)
	{
		close(sockfd);
		return -1;
	}
	return sockfd;
}

int addto_msglist(int utmpnum, char *userid)
{
	msg_t msgbuf;

	if ((msgbuf.sockfd = get_sockfd()) < 0)
		return -1;
	msgbuf.type = MSGD_NEW;
	snprintf(msgbuf.rawdata, sizeof(msgbuf.rawdata), "NEW %s %d\n",
		userid, utmpnum);
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
	msg_t msgbuf;

	if ((msgbuf.sockfd = get_sockfd()) < 0)
		return -1;
	msgbuf.type = MSGD_DEL;
	snprintf(msgbuf.rawdata, sizeof(msgbuf.rawdata), "DEL %s %d\n",
		userid, utmpnum);
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

int send_webmsg(int destutmp, char *destid, int srcutmp, char *srcid, char *msg)
{
	msg_t msgbuf;

	if ((msgbuf.sockfd = get_sockfd()) < 0)
		return -1;
	msgbuf.type = MSGD_SND;
	snprintf(msgbuf.rawdata, sizeof(msgbuf.rawdata), "SND %s %d %s %d\n",
		destid, destutmp, srcid, srcutmp);
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

int receive_webmsg(int destutmp, char *destid, int *srcutmp, char *srcid, char *msg)
{
	msg_t msgbuf;
	char *ptr;
	char *ptr2;

	if ((msgbuf.sockfd = get_sockfd()) < 0)
		return -1;
	msgbuf.type = MSGD_RCV;
	snprintf(msgbuf.rawdata, sizeof(msgbuf.rawdata), "RCV %s %d\n",
		destid, destutmp);
	write_peer(&msgbuf);
	if (read_peer(msgbuf.sockfd, &msgbuf) < 0)
		goto receive_failed;
	if (msgbuf.type != MSGD_FRM)
		goto receive_failed;
	if ((ptr = strchr(msgbuf.rawdata, ' ')) == NULL)
		goto receive_failed;
	*ptr++ = '\0';
	if ((ptr2 = strchr(ptr, ' ')) == NULL)
		goto receive_failed;
	*ptr2++ = '\0';
	strncpy(srcid, ptr, IDLEN);
	srcid[IDLEN] = '\0';
	*srcutmp = atoi(ptr2);
	msgbuf.type = MSGD_OK;
	snprintf(msgbuf.rawdata, sizeof(msgbuf.rawdata), 
		"OK Ready to receive my message\n");
	write_peer(&msgbuf);
	if (read_peer(msgbuf.sockfd, &msgbuf) < 0)
		goto receive_failed;
	if (msgbuf.type != MSGD_MSG)
		goto receive_failed;
	/* rawdata should be "MSG msgstr\n" */
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

    sethomefile(buf,uident,"msgfile");
    if((fp=fopen(buf,"a"))==NULL)
        return -1;
    fputs(msgbuf,fp);
    fclose(fp);

	return 0;
}

int
sendmsgfunc(uentp,msgstr,mode)
struct user_info *uentp;
char msgstr[256];
int mode;
{
    char uident[STRLEN] ,ret_str[20];
    FILE *fp;
    time_t now;
    struct user_info *uin ;
    char buf[80],msgbuf[256] ,*timestr,msgbak[256];
    int msg_count=0;
    int Gmode = 0;

    *msgbak = 0;	/* period 2000-11-20 may be used without init */
    *msgbuf = 0;
    *msgerr=0; /* clear msg error */
    {
        /*  if(!strcasecmp(uentp->userid,currentuser->userid))	rem by Haohmaru,’‚—˘≤≈ø…“‘◊‘º∫∏¯◊‘º∫∑¢msg
            return 0;    
        */   uin=uentp;
        strcpy(uident,uin->userid);
        /*   strcpy(MsgDesUid, uin->userid); change by KCN,is wrong*/
    }
    if(!HAS_PERM(currentuser,PERM_SEECLOAK) && uin->invisible && strcmp(uin->userid,currentuser->userid) && mode!=4)
        return -2;

    if((mode!=3)&&(LOCKSCREEN == uin->mode)) /* Leeward 98.02.28 */
    {
    	strcpy(msgerr,"∂‘∑Ω“—æ≠À¯∂®∆¡ƒª£¨«Î…‘∫Ú‘Ÿ∑¢ªÚ∏¯À˚(À˝)–¥–≈...\n");
        return -1 ;
    }

    if ((mode!=3)&&(NA==canIsend2(uin->userid)))/*Haohmaru.06.06.99.ºÏ≤È◊‘º∫ «∑Ò±ªignore*/
    {
    	strcpy(msgerr,"∂‘∑Ωæ‹æ¯Ω” ‹ƒ„µƒ—∂œ¢...\n");
        return -1;
    }
    if (mode!=3 && uin->mode != WEBEXPLORE) {
    sethomefile(buf,uident,"msgcount");
    fp=fopen(buf, "rb");
    if (fp!=NULL)
    {
        fread(&msg_count,sizeof(int),1,fp);
        fclose(fp);

        if(msg_count>MAXMESSAGE)
        {
            strcpy(msgerr,"∂‘∑Ω…–”–“ª–©—∂œ¢Œ¥¥¶¿Ì£¨«Î…‘∫Ú‘Ÿ∑¢ªÚ∏¯À˚(À˝)–¥–≈...\n");
            return -1 ;
        }
    }
    }
    if (msgstr == NULL) {
    	/* per-sending check only */
    	return 0;
    }
    now=time(0);
    timestr=ctime(&now)+11;
    *(timestr+8)='\0';
    strcpy(ret_str,"R ªÿ—∂œ¢");
    if(mode == 0||mode==2||mode==4)
    {
        sprintf(msgbuf,"[44m[36m%-12.12s[33m(%-5.5s):[37m%-59.59s[m[%dm\033[%dm\n", currentuser->userid,
                timestr, msgstr,getuinfopid()+100,uin->pid+100);
        sprintf(msgbak,"[44m[0;1;32m=>[37m%-10.10s[33m(%-5.5s):[36m%-59.59s[m[%dm\033[%dm\n", uident,timestr, msgstr,getuinfopid()+100,uin->pid+100);
    }else
    {
        if(mode==3) {
            sprintf(msgbuf,"[44m[33m’æ≥§Ï∂ %8.8s  ±π„≤•£∫"
                    "[37m%-55.55s[m\033[%dm\n",
                    /*				"[37m%-59.59s[m\033[%dm\n",*/
                    timestr,msgstr,uin->pid+100);
        }
        else if(mode==1)
        {
            sprintf(msgbuf,"[44m[36m%-12.12s(%-5.5s) —˚«Îƒ„[37m%-43.43s(%s)[m[%dm\033[%dm\n",
                    currentuser->userid, timestr, msgstr,ret_str,getuinfopid()+100,uin->pid+100);
            sprintf(msgbak,"[44m[37mƒ„(%-5.5s) —˚«Î%-12.12s[36m%-43.43s(%s)[m[%dm\033[%dm\n", timestr,uident,msgstr,ret_str,getuinfopid()+100,uin->pid+100);
        }else if(mode==3)
        {
            sprintf(msgbuf,"[44m[32mBBS œµÕ≥Õ®∏Ê(%-5.5s):[37m%-59.59s[31m(%s)[m\033[%dm\n",
                    timestr, msgstr,ret_str,uin->pid+100);
        }
    }
    if (Gmode == 2)
        sprintf(msgbuf,"[44m[33m’æ≥§Ï∂ %8.8s  ±π„≤•£∫[37m%-59.59s[m\033[%dm\n",timestr,buf,uin->pid+100);
#ifdef BBSMAIN
	if (uin->mode == WEBEXPLORE)
	{
		if (send_webmsg(get_utmpent_num(uin), uident, utmpent, currentuser->userid, msgbuf) < 0)
		{
    		strcpy(msgerr,"send web message failed...\n");
			return -1;
		}
		if (store_msgfile(uident, msgbuf) < 0)
			return -2;

		/*Haohmaru.99.6.03.ªÿµƒmsg“≤º«¬º*/
		if(strcmp(currentuser->userid,uident))
		{
			if (store_msgfile(currentuser->userid, msgbak) < 0)
				return -2;
		}
		return 1;
	}
#endif

    /* ºÏ≤ÈÀ˘∑¢msgµƒƒøµƒuid «∑Ò“—æ≠∏ƒ±‰  1998.7.5 by dong*/
    uin=t_search(MsgDesUid,uentp->pid);

    if ((uin == NULL) || (uin->active == 0) || (uin->pid == 0) || (kill(uin->pid, 0) !=0) && (uentp->pid != 1))
    { /*
        uin=t_search(MsgDesUid, NA);
            if ((uin == NULL) || (uin->active == 0) || (uin->pid == 0) || (kill(uin->pid, 0) !=0)){ */
        if (mode == 0)
            return -2;
        strcpy(msgerr, "∂‘∑Ω“—æ≠¿Îœﬂ....\n");   
        return -1; /* ∂‘∑Ω“—æ≠¿Îœﬂ */
        /*} */
    }

    /*sethomefile(buf,uident,"msgfile");
    if((fp=fopen(buf,"a"))==NULL)
        return -2;
    fputs(msgbuf,fp);
    fclose(fp);*/
	if (store_msgfile(uident, msgbuf) < 0)
		return -2;

    /*Haohmaru.99.6.03.ªÿµƒmsg“≤º«¬º*/
    if(strcmp(currentuser->userid,uident)){
        /*sethomefile(buf,currentuser->userid,"msgfile");
        if((fp=fopen(buf,"a"))==NULL)
            return -2;
        fputs(msgbak,fp);
        fclose(fp);*/
		if (store_msgfile(currentuser->userid, msgbak) < 0)
			return -2;
    }
    if(uentp->pid !=1 && kill(uin->pid,SIGUSR2)==-1&&msgstr==NULL)
    {
    	
        strcpy(msgerr,"\n∂‘∑Ω“—æ≠¿Îœﬂ.....\n") ; 
        return -1;
    }
    sethomefile(buf,uident,"msgcount");
    fp=fopen(buf, "wb");
    if (fp!=NULL)
    {
        msg_count++;
        fwrite(&msg_count,sizeof(int),1,fp);
        fclose(fp);
    }
    return 1 ;
}

