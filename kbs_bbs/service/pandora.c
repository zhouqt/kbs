// NJU bbsnet, preview version, zhch@dii.nju.edu.cn, 2000.3.23 //
// HIT bbsnet, Changed by Sunner, sun@bbs.hit.edu.cn, 2000.6.11
// zixia bbsnet, Changed by zdh, dh_zheng@hotmail.com,2001.12.04
// zixia bbsnet, Changed by zixia, zixia@zixia.net, 2002.12.25
// zixia pandora, Changed by zixia, zixia@zixia.net, 2003.1.25
// zixia pandora, Change by roy, roy@zixia.net 2003.6.08
#include "bbs.h"
#include "select.h"
#include "tcplib.h"
#include <netdb.h>

#define TIME_OUT	15
#define MAX_PROCESS_BAR_LEN 30
#define BBSNET_LOG_BOARD "bbsnet"
#define DATAFILE BBSHOME "/etc/pandora.ini"
#define MAXSTATION  100


char host1[MAXSTATION][19], host2[MAXSTATION][40], ip[MAXSTATION][40];
int port[MAXSTATION]; 

char str[]= "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
char ip_zdh[40]; 
char user[21];
int sockfd;
jmp_buf jb;

extern char fromhost[];
extern struct userec *currentuser;
extern int msg_count;
extern struct user_info uinfo;
extern int utmpent;

struct _select_def bbsnet_conf;

locate(int n)
{
    int x, y;
    char buf[20];

    if (n >= bbsnet_conf.item_count)
        return;
    y = n % 19+1 ;
    x = n / 19 * 24 + 3;
	
	move(y, x);
}	

// added by flyriver, 2001.3.2
// ´©ËóÈÕ¼Ç
// mode == 0, ¿ªÊ¼´©Ëó
//         1, ½áÊø´©Ëó

int bbsnet_report(char *station, char *addr, long id, int mode)
{
	struct fileheader fh;
	char buf[1024];
	char fname[256];
	int fd;
	time_t now;
	FILE *fp;

	bzero(&fh, sizeof(fh));
	setbpath(buf, BBSNET_LOG_BOARD);
	GET_POSTFILENAME(fname, buf);
	strncpy(fh.filename, fname, sizeof(fh.filename)-1);
	fh.filename[sizeof(fh.filename)-1] = '\0';
	setbfile(fname, BBSNET_LOG_BOARD, fh.filename);
	now = time(NULL);
	strcpy(fh.owner, "deliver");
	fh.innflag[0] = 'L';
	fh.innflag[1] = 'L';
	if (mode == 0)
		snprintf(fh.title, STRLEN, "[%ld]%s´©Ëóµ½%s", id, user, station);
	else
		snprintf(fh.title, STRLEN, "[%ld]%s½áÊøµ½%sµÄ´©Ëó", id, user, station);
	
	if ((fp = fopen(fname, "w")) == NULL)
		return -1;
	fprintf(fp, "·¢ÐÅÈË: deliver (×Ô¶¯·¢ÐÅÏµÍ³), ÐÅÇø: %s\n", BBSNET_LOG_BOARD);
	fprintf(fp, "±ê  Ìâ: %s\n", fh.title);
	fprintf(fp, "·¢ÐÅÕ¾: %s (%24.24s)\n\n", BBS_FULL_NAME, ctime(&now));
	fprintf(fp, "    [1;33m%s[m ÓÚ [1;37m%24.24s[m ÀûÓÃ±¾Õ¾bbsnet³ÌÐò,\n",
			user, ctime(&now));
	if (mode == 0)
	{
		fprintf(fp, "    ´©Ëóµ½ [1;32m%s[m Õ¾, µØÖ·Îª[1;31m%s[m.\n",
				station, addr);
	}
	else
	{
		int t;
		int h;
		fprintf(fp, "    ½áÊøµ½ [1;32m%s[m Õ¾µÄ´©Ëó, µØÖ·Îª[1;31m%s[m.\n",
				station, addr);
		t = now - id;
		if (t < 2400)
			sprintf(buf, "[1;32m%d[m·ÖÖÓ", t/60);
		else
		{
			h = t / 2400;
			t -= h * 2400;
			sprintf(buf, "[1;32m%d[mÐ¡Ê±[1;32m%d[m·ÖÖÓ", h, t/60);
		}
		fprintf(fp, "    ±¾´Î´©ËóÒ»¹²ÓÃÁË %s.\n", buf);
	}
	fprintf(fp, "    ¸ÃÓÃ»§´Ó [1;31m%s[m µÇÂ¼±¾Õ¾.\n", fromhost);
	fclose(fp);
	
	return after_post(NULL, &fh, BBSNET_LOG_BOARD, NULL);
}



static bool bbsnet_redraw=true;

// from Maple-hightman
// added by flyriver, 2001.3.3
int telnetopt(int fd, char* buf, int max)
{
	unsigned char c,d,e;
	int pp=0;
	unsigned char tmp[30];
	while(pp<max)
	{
		c=buf[pp++];
		if(c==255)
		{
			d=buf[pp++];
			e=buf[pp++];
			oflush();
			if((d==253)&&(e==3||e==24))
			{
				tmp[0]=255;
				tmp[1]=251;
				tmp[2]=e;
				write(fd,tmp,3);
				continue;
			}
			if((d==251||d==252)&&(e==1||e==3||e==24))
			{
				tmp[0]=255;
				tmp[1]=253;
				tmp[2]=e;
				write(fd,tmp,3);
				continue;
			}
			if(d==251||d==252)
			{
				tmp[0]=255;
				tmp[1]=254;
				tmp[2]=e;
				write(fd,tmp,3);
				continue;
			}
			if(d==253||d==254)
			{
				tmp[0]=255;
				tmp[1]=252;
				tmp[2]=e;
				write(fd,tmp,3);
				continue;
			}
			if(d==250)
			{
				while(e!=240&&pp<max)
					e=buf[pp++];
				tmp[0]=255;
				tmp[1]=250;
				tmp[2]=24;
				tmp[3]=0;
				tmp[4]=65;
				tmp[5]=78;
				tmp[6]=83;
				tmp[7]=73;
				tmp[8]=255;
				tmp[9]=240;
				write(fd,tmp,10);
			}
		}
		else
		ochar(c);
	}
    oflush();
	return 0;
}

// ±ØÐëÓÃ¶àÏß³Ì²ÅÄÜÊµÏÖ½ø¶ÈÌõÁË£¬sigh
// UPDATE: ²»ÐèÒª¶àÏß³Ì£¬by flyriver, 2002.8.10
void bbsnet_timeout(int signo)
{
	longjmp(jb, signo);
}

static void process_bar(int n, int len)
{
	char buf[256];
	char buf2[256];
	char *ptr;
	char *ptr2;
	char *ptr3;

	move(4, 0);
	prints("©°©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©´\n");
	sprintf(buf2, "            %3d%%              ", n * 100 / len);
	ptr = buf;
	ptr2 = buf2;
	ptr3 = buf + n;
	while (ptr != ptr3)
		*ptr++ = *ptr2++;
	*ptr++ = '\x1b';
	*ptr++ = '[';
	*ptr++ = '4';
	*ptr++ = '4';
	*ptr++ = 'm';
	while (*ptr2 != '\0')
		*ptr++ = *ptr2++;
	*ptr++ = '\0';
	prints("©¦[46m%s[m©¦\n", buf);
	prints("©¸©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¼\n");
	redoscr();
}

static time_t last_refresh;

static int 
bbsnet_read(int fd, char *buf, int len)
{
	int rc;
	time_t now;

	rc = raw_read(fd, buf, len);
	if (rc > 0)
	{
		now = time(NULL);
		if (now - last_refresh > 60)
		{
			uinfo.freshtime = now;
			UPDATE_UTMP(freshtime, uinfo);
			last_refresh = now;
		}
	}
	return rc;
}


int bbsnet(int n)
{
	time_t now;
	struct hostent *pHost = NULL;
	struct sockaddr_in remote;
	unsigned char buf[BUFSIZ];
	int rc;
	int rv;
	int maxfdp1;
	fd_set readset;
	struct timeval tv;
	int tos = 020; /* Low delay bit */
	int i;
	sig_t oldsig;
	int ret;

    char buf1[40], buf2[39], c, buf3[2];        //Ôö¼ÓµÄ±äÁ¿
    int l;                      //ÅÐ¶ÏÊÇ²»ÊÇport
    int j, m;

    if (strcmp(host2[n], "°ãÈô²¨Á_ÃÜ") == 0) {  //Èç¹ûÊÇ×Ô¶¨ÒåÕ¾µã£¬µÈ´ýÊäÈëip»òÓòÃû

        for (i = 0; i < 25; i++) {
            buf1[i] = '\0';
            buf2[i] = '\0';
        }
		move(22,2);
        prints("[1;32mÁ¬Íù: [m");
        refresh();
        j = 0;
        l = 0;
        for (i = 0; i < 30; i++) {
            c = igetch();
            if (c == ' ' || c == '\015' || c == '\0' || c == '\n')
                break;
            if (c == ':') {
                l = 1;
                sprintf(buf3, "%c", c);
                prints("[0;1m");
                prints(buf3);
                prints("[1;33m");
                refresh();
                strncpy(ip_zdh, buf1, 40);
                strncpy(ip[n], ip_zdh, 40);
                for (m = 0; m < 20; m++) {
                    buf1[m] = '\0';
                    buf2[m] = '\0';
                }
                j = 0;
            }
            if ((c >= '0' && c <= '9') || (c == '.') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
                sprintf(buf3, "%c", c);
                sprintf(buf2, "%s%c", buf1, c);
                sprintf(buf1, "%s", buf2);
                prints(buf3);
                refresh();
                j = j + 1;
            }
            if ((c == 8) && (j >= 1)) {
                for (m = j - 1; m < 20; m++) {
                    buf1[m] = '\0';
                    buf2[m] = '\0';
                }
                sprintf(buf3, "%c", c);
                prints(buf3);
                refresh();
                i--;
                j--;
            }

            refresh();
        }
        if (l == 0) {
            strncpy(ip_zdh, buf1, 40);
            strcpy(ip[n], ip_zdh);
        } else {
            port[n] = atoi(buf1);
        }
    }


	now = time(NULL);
	clear();
	prints("[1;32mÕýÔÚ²âÊÔÍù %s (%s) µÄÁ¬½Ó£¬ÇëÉÔºò... [m\n", 
			host1[n], ip[n]);
	prints("[1;32mÈç¹ûÔÚ %d ÃëÄÚÎÞ·¨Á¬ÉÏ£¬´©Ëó³ÌÐò½«·ÅÆúÁ¬½Ó¡£[m\n",
			TIME_OUT);
	if (setjmp(jb) == 0)
	{
		oldsig = signal(SIGALRM, bbsnet_timeout);
		alarm(TIME_OUT);
		pHost = gethostbyname(ip[n]);
		alarm(0);
	}
	signal(SIGALRM, oldsig);
	if (pHost == NULL)
	{
		prints("[1;31m²éÕÒÖ÷»úÃûÊ§°Ü£¡[m\n");
		pressreturn();
		return -1;
	}
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		prints("[1;31mÎÞ·¨´´½¨socket£¡[m\n");
		pressreturn();
		return -1;
	}
	bzero(&remote, sizeof(remote));
	remote.sin_family = AF_INET;
	remote.sin_port = htons(port[n]);
	remote.sin_addr = *(struct in_addr *)pHost->h_addr_list[0];

	prints("[1;32m´©Ëó½ø¶ÈÌõÌáÊ¾Äúµ±Ç°ÒÑÊ¹ÓÃµÄÊ±¼ä¡£[m\n");
	process_bar(0, MAX_PROCESS_BAR_LEN);
	for (i = 0; i < MAX_PROCESS_BAR_LEN; i++)
	{
		if (i == 0)
			rv = NonBlockConnectEx(sockfd, (struct sockaddr *)&remote, 
				sizeof(remote), 500, 1);
		else
			rv = NonBlockConnectEx(sockfd, (struct sockaddr *)&remote, 
				sizeof(remote), 500, 0);
		if (rv == ERR_TCPLIB_TIMEOUT)
		{
			process_bar(i+1, MAX_PROCESS_BAR_LEN);
			continue;
		}
		else if (rv == 0)
			break;
		else
		{
			prints("[1;31mÁ¬½ÓÊ§°Ü£¡[m\n");
			pressreturn();
			ret = -1;
			goto on_error;
		}
	}
	if (i == MAX_PROCESS_BAR_LEN)
	{
		prints("[1;31mÁ¬½Ó³¬Ê±£¡[m\n");
		pressreturn();
		ret =  -1;
		goto on_error;
	}
	setsockopt(sockfd, IPPROTO_IP, IP_TOS, &tos, sizeof(int));	
	prints("[1;31mÁ¬½Ó³É¹¦£¡[m\n");
	bbsnet_report(host1[n], ip[n], now, 0);
	clear();
	refresh();
	for (;;)
	{
		FD_ZERO(&readset);
		FD_SET(0, &readset);
		FD_SET(sockfd, &readset);
		maxfdp1 = sockfd + 1;
		tv.tv_sec = 1200;
		tv.tv_usec = 0;

		/*if ((rv = SignalSafeSelect(maxfdp1, &readset, NULL, NULL, &tv)) == -1)*/
		if ((rv = select(maxfdp1, &readset, NULL, NULL, &tv)) == -1)
		{
			if (errno == EINTR)
			{
				while (msg_count)
				{
					msg_count--;
					r_msg();
				}
				continue;
			}
			ret = -1;
			goto on_error;
		}
		if (rv == 0)
		{
			ret -1;
			goto on_error;
		}

		if (FD_ISSET(sockfd, &readset))
		{
			if ((rc = read(sockfd, buf, BUFSIZ)) < 0)
			{
				ret = -1;
				goto on_error;
			}
			else if (rc == 0)
				break;
			else if (strchr(buf, 255))	/* ²éÕÒÊÇ·ñº¬ÓÐTELNETÃüÁîIAC */
				telnetopt(sockfd, buf, rc);
			else
			{
				output(buf, rc);
				oflush();
			}
		}
		if (FD_ISSET(0, &readset))
		{
			if ((rc = bbsnet_read(0, buf, BUFSIZ)) < 0)
			{
				ret = -1;
				goto on_error;
			}
			if (rc == 0)
				break;
			write(sockfd, buf, rc);
		}
	}
	bbsnet_report(host1[n], ip[n], now, 1);
	ret = 0;
on_error:
	close(sockfd);
	clear();
	redoscr();
	return ret;
}

static int bbsnet_onselect(struct _select_def *conf)
{
	bbsnet(conf->pos-1);
	bbsnet_redraw=true;
	return SHOW_REFRESH;
}

static int bbsnet_show(struct _select_def *conf, int pos)
{
    return SHOW_CONTINUE;
}

static int bbsnet_key(struct _select_def *conf, int command)
{
    char* ptr;
    if ((ptr=strchr(str,command))!=NULL) {
	  conf->new_pos = (ptr-str) + 1;
	  return SHOW_SELCHANGE;
    }
    switch (command) {
    case Ctrl('C'):
    case Ctrl('A'):
    	   return SHOW_QUIT;
    case '?':
        show_help("help/bbsnethelp");
		bbsnet_redraw=true;
        return SHOW_REFRESH;
    case '$':
    	  conf->new_pos=conf->item_count;
    	  return SHOW_SELCHANGE;
    case '^':
    	  conf->new_pos=1;
    	  return SHOW_SELCHANGE;
    }
    return SHOW_CONTINUE;
}

static void bbsnet_refresh(struct _select_def *conf)
{

	int n;
	clear();
    prints("©³©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥[1;35m ÔÂ  ¹â  ±¦  ºÐ [m©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©·");
	for (n = 1; n < 22; n++) {
		move(n,0);
		prints("©§                                                                            ©§");
	}
	move(22,0);
	prints("©§                                                               [1;36m°´[1;33mCtrl+C[1;36mÍË³ö[m ©§");
	move(23,0);
	prints("©»©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¿");
	move(20,2);
	prints("©¤©¤©¤[1;36m°ãÈô²¨Á_ÃÜ[m©¤©¤©¤©¤©¤©¤©¤[1;36m°ã-Èô-²¨-Á_-ÃÛ~~[m©¤©¤©¤©¤©¤©¤©¤[1;36m°ãÈô²¨Á_ÃÜ[m©¤©¤©¤");
	for (n = 0; n < conf->item_count; n++) {
		locate(n);
		prints("[1;32m %c.[m%s", str[n], host2[n]);
	}
    bbsnet_selchange(conf,conf->pos);

}

int bbsnet_selchange(struct _select_def* conf,int new_pos)
{
    static oldn = -1;

    if (oldn >= 0) {
        locate(oldn);
        prints("[1;32m %c.[m%s", str[oldn], host2[oldn]);
    }
    oldn = new_pos-1;
    if (strcmp(host2[new_pos-1], "°ãÈô²¨Á_ÃÜ") == 0) {  //ÅÐ¶Ï×Ô¶¨ÒåÕ¾µã
		move(21,2);
        prints("[1;37mÊ¹ÓÃ·½·¨: »Ø³µºóÊäÈëip[:port]¡£[1;33m[22;32H[1;37m Õ¾Ãû: [1;33m×Ô¶¨ÒåÕ¾µã              ");
		move(22,2);
        prints("[1;37m[23;3HÁ¬Íù: [1;33m__________________           [21;1H");
    } else {
		move(21,2);
        prints("[1;37mµ¥Î»: [1;33m%s                   [22;32H[1;37m Õ¾Ãû: [1;33m%s              ", host1[new_pos-1], host2[new_pos-1]);
		move(22,2);
		prints("[1;37m[23;3HÁ¬Íù: [1;33m%s                   [21;1H", ip[new_pos-1]);
    }
    locate(new_pos-1);
    prints("[%c][1;42m%s[m", str[new_pos-1], host2[new_pos-1]);
    return SHOW_CONTINUE;
}
int load_data(struct _select_def* conf,int pos,int len)
{
    FILE *fp;
    char t[256], *t1, *t2, *t3, *t4;





    fp = fopen(DATAFILE, "r");
	conf->item_count=0;
    if (fp == NULL)
        return SHOW_REFRESH;
    while (fgets(t, 255, fp) && conf->item_count < MAXSTATION) {
        t1 = strtok(t, " \t");
        t2 = strtok(NULL, " \t\n");
        t3 = strtok(NULL, " \t\n");
        t4 = strtok(NULL, " \t\n");
        if (t1[0] == '#' || t1 == NULL || t2 == NULL || t3 == NULL)
            continue;
        strncpy(host1[conf->item_count], t1, 16);
        strncpy(host2[conf->item_count], t2, 36);
        strncpy(ip[conf->item_count], t3, 36);
        port[conf->item_count] = t4 ? atoi(t4) : 23;
        conf->item_count++;
    }
    fclose(fp);

  conf->item_per_page = conf->item_count;
  bbsnet_redraw=true;
  return SHOW_REFRESH;
}


void main_loop()
{
	char buf[STRLEN];
	int i;
	POINT pts[MAXSTATION];

   for (i = 0; i < MAXSTATION; i++) {
	   pts[i].x = i / 19 * 24 + 2;
	   pts[i].y = i % 19+1;
   };
	bzero(&bbsnet_conf,sizeof(bbsnet_conf));
    load_data(&bbsnet_conf,0,MAXSTATION);
	bbsnet_conf.flag = LF_FORCEREFRESHSEL | LF_BELL | LF_LOOP;     //|LF_HILIGHTSEL;
	bbsnet_conf.prompt = NULL;
	bbsnet_conf.item_pos = pts;
	bbsnet_conf.arg = NULL;
	bbsnet_conf.title_pos.x = 0;
	bbsnet_conf.title_pos.y = 0;
	bbsnet_conf.pos = 1;
	bbsnet_conf.page_pos = 1;

	bbsnet_conf.on_select = bbsnet_onselect;
	bbsnet_conf.show_data = bbsnet_show;
	bbsnet_conf.key_command = bbsnet_key;
	bbsnet_conf.show_title = bbsnet_refresh;
	bbsnet_conf.get_data = load_data;
	bbsnet_conf.on_selchange = bbsnet_selchange;
	bbsnet_conf.item_per_page = bbsnet_conf.item_count;

	list_select_loop(&bbsnet_conf);
}



int pandora()
{
	strncpy(user, currentuser->userid, 20);
	modify_user_mode(BBSNET);

    main_loop();
	return 0;
}
