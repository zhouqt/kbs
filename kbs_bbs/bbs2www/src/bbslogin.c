#include "bbslib.h"

int
count_user()
{
	int num;

	num = searchuser(getcurruserid());
    return apply_utmpuid( NULL, num, 0);
}

/* to be Continue to fix kick problem */
void
multi_user_check()
{
    struct user_info    uin;
    int         curr_login_num;
    char        buffer[40];

    if (count_user()<1)
		RemoveMsgCountFile(currentuser->userid);

    if (HAS_PERM(currentuser,PERM_MULTILOG)) 
        return;  /* don't check sysops */
    curr_login_num = get_utmp_number();
    /* Leeward: 97.12.22 BMs may open 2 windows at any time */
    /* Bigman: 2000.8.17 智囊团能够开2个窗口 */
    /* stephen: 2001.10.30 仲裁可以开两个窗口 */
    if ((HAS_PERM(currentuser,PERM_BOARDS) || HAS_PERM(currentuser,PERM_CHATOP)|| HAS_PERM(currentuser,PERM_JURY) || HAS_PERM(currentuser,PERM_CHATCLOAK)) && count_user() < 2)
        return;
    if ( (curr_login_num<700)&&(count_user()>=2) 
           || (curr_login_num>=700)&& (count_user()>=1) ) /*user login limit*/
    {  
		http_fatal("您同时上线的窗口数过多。为了保证他人利益，此次连线将被取消。");
    }
}

int main(int argc,char** argv)
{
	int pid, n, t;
	char buf[256], id[20], pw[40];
	struct userec *x = NULL;
	FILE *fp;

	init_all();
	strsncpy(id, getparm("id"), 13);
    strsncpy(pw, getparm("pw"), sizeof(pw));
/*
	strsncpy(id, argv[1], 13);
    strsncpy(pw, argv[2], 13);
*/
	if(loginok && strcasecmp(id, currentuser->userid)) {
		http_fatal("系统检测到目前你的计算机上已经登录有一个帐号 %s，请先退出.(%s)", 
			currentuser->userid, "选择正常logout, 或者关闭所有浏览器窗口");
	}
	getuser(id, &x);
	if(x==0) http_fatal("错误的使用者帐号");
	if(strcasecmp(id, "guest"))
	{
		/*if(!checkpasswd(x->passwd, pw)) {*/
		if (!checkpasswd2(pw, x))
		{
			if(pw[0]!=0) sleep(2);
			sprintf(buf, "%s %s %s\n", wwwCTime(time(0))+4, id, fromhost);
			f_append(WWW_BADLOGIN, buf);
			http_fatal("密码错误");
		}
		setcurrusr(x);
		multi_user_check();
		if(!user_perm(x, PERM_BASIC))
			http_fatal("此帐号已被停机, 若有疑问, 请用其他帐号在sysop版询问.");
		if(file_has_word(".bansite", fromhost)) {
			http_fatal("对不起, 本站不欢迎来自 [%s] 的登录. <br>若有疑问, 请与SYSOP联系.", fromhost);
		}
		t=x->lastlogin;
		x->lastlogin=time(0);
		save_user_data(x);
		if(abs(t-time(0))<5) http_fatal("两次登录间隔过密!");
		x->numlogins++;
		strsncpy(x->lasthost, fromhost, IPLEN);
		if (!HAS_PERM(currentuser,PERM_LOGINOK) && !HAS_PERM(currentuser,PERM_SYSOP))
		{
			if (strchr(currentuser->realemail, '@')
				&& valid_ident(currentuser->realemail))
			{
				currentuser->userlevel |= PERM_DEFAULT;
				if (HAS_PERM(currentuser,PERM_DENYPOST)/* && !HAS_PERM(currentuser,PERM_SYSOP)*/)
					currentuser->userlevel &= ~PERM_POST;
			}
		}
	}
	sprintf(buf, "%s %s %s\n", wwwCTime(time(0)), x->userid, fromhost);
	f_append(WWW_LOG, buf);
	sprintf(buf, "%s ENTER %-12s @%s [www]\n", wwwCTime(time(0))+4, x->userid, fromhost);
	f_append("usies", buf);
	n=0;
	if(!loginok && strcasecmp(id, "guest"))	wwwlogin(x);
	redirect(FIRST_PAGE);
}

int wwwlogin(struct userec *user) {
	FILE *fp;
	char buf[80];
	int pid, tmp;
	uinfo_t *u;
	uinfo_t ui;
	int utmpent;

    memset( &ui, 0, sizeof( uinfo_t ) );
    ui.active = YEA ;

    /* Bigman 2000.8.29 智囊团能够隐身 */
    if( (HAS_PERM(currentuser,PERM_CHATCLOAK) || HAS_PERM(currentuser,PERM_CLOAK)) && (user->flags[0] & CLOAK_FLAG))
        ui.invisible = YEA;
    ui.pager = 0;
    if(define(DEF_FRIENDCALL))
    {
        ui.pager|=FRIEND_PAGER;
    }
    if(user->flags[0] & PAGER_FLAG)
    {
        ui.pager|=ALL_PAGER;
        ui.pager|=FRIEND_PAGER;
    }
    if(define(DEF_FRIENDMSG))
    {
        ui.pager|=FRIENDMSG_PAGER;
    }
    if(define(DEF_ALLMSG))
    {
        ui.pager|=ALLMSG_PAGER;
        ui.pager|=FRIENDMSG_PAGER;
    }
    ui.uid = getusernum(user->userid);
    strncpy( ui.from, fromhost, IPLEN );
	*(int*)(ui.from+32)=time(0);	/* for counting user's stay time */
									/* refer to bbsfoot.c for details */
	set_idle_time(&ui, time(0));
	ui.mode = WEBEXPLORE;
    strncpy( ui.userid,   user->userid,   20 );
    strncpy( ui.realname, user->realname, 20 );
    strncpy( ui.username, user->username, 40 );
	set_friends_num(0);
	init_finfo_addr();
    getfriendstr();
    utmpent = getnewutmpent2(&ui) ;
    if (utmpent == -1)
		http_fatal("抱歉，目前在线用户数已达上限，无法登录。请稍后再来。");
	u = get_user_info(utmpent);
	u->pid = 1;
	tmp=rand()%100000000;
	u->utmpkey=tmp;
	setcurruinfo(u);
	if (addto_msglist(get_utmpent_num(getcurruinfo()), getcurruserid()) < 0)
		http_fatal("无法添加当前用户到消息列表中");
	sprintf(buf, "%d", utmpent);
	setcookie("utmpnum", buf);
	sprintf(buf, "%d", tmp);
	setcookie("utmpkey", buf);
	setcookie("utmpuserid", user->userid);
	set_my_cookie();
	return 0;
}

long f_offset=0;
#if 0
int get_msgcount()
{
    char buf2[256] ;
	char fname2[STRLEN];
	int fd;
	int msg_count;

	setuserfile(fname2,currentuser->userid,"msgcount");
	if ((fd = open(fname2, O_RDWR, 0600)) < 0)
	{
		sprintf(buf2, "open %s failed: %s\n", fname2, strerror(errno));
		f_append("debug", buf2);
		return -1;
	}
	read(fd, &msg_count, sizeof(msg_count));
	close(fd);

	return msg_count;
}

void set_msgcount(int msg_count)
{
    char buf2[256] ;
	char fname2[STRLEN];
	int fd;

	setuserfile(fname2,currentuser->userid,"msgcount");
	if ((fd = open(fname2, O_RDWR, 0600)) < 0)
	{
		sprintf(buf2, "open %s failed: %s\n", fname2, strerror(errno));
		f_append("debug", buf2);
		return;
	}
	write(fd, &msg_count, sizeof(msg_count));
	close(fd);

	return;
}

/* 从消息记录(msgfile)中读入消息,
 * 并写入 WWW 消息记录(wwwmsg)中.
*/
void add_msg()
{
	char file[256], file2[256];
    FILE *fp;
    char buf[256];
    char msg[256];
    int  msg_count;
    char fname[STRLEN];
    int i;
    int send_pid;
    char *ptr;
    struct user_info *ui ;
    char msgbuf[STRLEN];
	int fd;

	ui = getcurruinfo();
    setuserfile(fname,currentuser->userid,"msgfile");
    if(!file_exist(fname))
        return;
	sleep(1); /* quick and dirty */
	if ((msg_count = get_msgcount()) < 0)
		return;
   	setuserfile(file,currentuser->userid,"wwwmsg");
	setuserfile(file2, currentuser->userid,".wwwmsg.lock");
	if ((fd = open(file2, O_RDWR | O_CREAT, 0600)) < 0)
		goto failed;
	flock(fd, LOCK_EX);
    while(1)
    {
        if((fp=fopen(fname,"r"))==NULL)
            break;
        i=0;
        if (f_offset==0)
        {
            while( fgets(buf,256,fp)!=NULL)
            {
                ptr=strrchr(buf,'[');
                send_pid=atoi(ptr+1);
                if(send_pid>100)
                    send_pid-=100;
                if (ui->pid == send_pid)
                {
					i=1;
                    strcpy(msg,buf);
                }
            }
        }
        else
		{
			fseek(fp, f_offset, SEEK_SET);
            while(fgets( msg, 256, fp)!=NULL)
            {
                ptr=strrchr(msg,'[');
                send_pid=atoi(ptr+1);
                if(send_pid>100)
                    send_pid-=100;
                if (ui->pid == send_pid)
                {
					i=1;
					break;
				}
            }
        }
        f_offset = ftell(fp);
        fclose(fp);
        if (i==0)
			break;
		fp = fopen(file, "a");
		fputs(msg, fp);
		fclose(fp);
		if (msg_count)
			msg_count--;
    }
	set_msgcount(msg_count);
    if (count_user()<2)
		RemoveMsgCountFile(currentuser->userid);
	flock(fd, LOCK_UN);
	close(fd);

failed:
    signal(SIGUSR2, add_msg);
    return ;
}
#endif 
void
setflags(mask, value)
int mask, value;
{
	struct userec *u;

	u = getcurrusr();
    if (((u->flags[0] & mask) && 1) != value) {
        if (value) u->flags[0] |= mask;
        else u->flags[0] &= ~mask;
    }
}

