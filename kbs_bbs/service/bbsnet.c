// NJU tinybbsnet, Preview Version, zhch@dii.nju.edu.cn, 2000.3.23 //
#include "bbs.h"
#include "tcplib.h"
#include <netdb.h>

#undef printf
#undef perror

#define TIME_OUT	15
#define MAX_PROCESS_BAR_LEN 30
#define BBSNET_LOG_BOARD "bbsnet"
#define DATAFILE "etc/bbsnet.ini"

char host1[100][40], host2[100][40], ip[100][40];
int port[100], counts= 0; 
char str[]= "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()";

char user[21];
int sockfd;
jmp_buf jb;

extern char fromhost[];
extern struct userec *currentuser;
extern int msg_count;
	
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

void init_data()
{
    FILE *fp;
    char t[256], *t1, *t2, *t3, *t4;

    fp = fopen(DATAFILE, "r");
    if(fp== NULL) return;
    while(fgets(t, 255, fp)&& counts <= 72)
	{
        t1= strtok(t, " \t");
        t2= strtok(NULL, " \t\n");
        t3= strtok(NULL, " \t\n"); 
        t4= strtok(NULL, " \t\n");
        if(t1[0]== '#'|| t1== NULL|| t2== NULL|| t3== NULL)
			continue;
        strncpy(host1[counts], t1, 16);
        strncpy(host2[counts], t2, 36);
        strncpy(ip[counts], t3, 36);
        port[counts]= t4? atoi(t4): 23;
        counts++;
    } 
    fclose(fp);
}

void locate(int n)
{
    int x, y;
    char buf[20];
    if(n>= counts)
		return;
    y= n% 19+ 2;
    x= n/ 19* 24+ 4;
	good_move(y, x);
}

void sh(int n)
{
    static int oldn= -1;
    if(n>= counts) return;
    if(oldn >=0) 
	{
        locate(oldn);
        prints("[1;32m %c.[m%s", str[oldn], host1[oldn]);
    }
    oldn = n; 
    locate(n);
    prints("[%c][1;42m%s[m", str[n], host1[n]);
	good_move(22, 3);
    prints("[1;37mµ¥Î»: [1;33m%s                   ", host2[n]);
	good_move(22, 32);
	prints("[1;37m Õ¾Ãû: [1;33m%s              ", host1[n]);
	good_move(23, 3);
    printf("[1;37mÁ¬Íù: [1;33m%s                   ", ip[n]);
	good_move(1, 1);
}

void show_all()
{
    int n;

	clear();
    prints("¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ\n");
    for(n= 1; n< 23; n++)
    prints("¡õ                                                                            ¡õ\n");
    prints("¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ");
	good_move(21, 3);
    prints("----------------------------------------------------------------------------");
    for(n= 0; n< counts; n++)
	{
        locate(n);
        prints("[1;32m %c.[m%s", str[n], host1[n]);
    }
}

static void 
display_sites()
{
	int i;
	int j;
	int k;
	const int rows = 20;

	clear();
	prints("                          ---== [0;1;36mÒøºÓ´©ËóÁÐ±í [m==---[m\n\n");
	for (i = 0; i < rows; i++)
	{
		for (k = 0; k < 3; k++)
		{
			j = k * rows + i;
			if (j < counts)
				prints("      [1;32m%02d. [m%-16s", j+1, host1[j]);
		}
		prints("[m\n");
	}
}

int getch()
{
	return igetkey();
}

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
			fflush(stdout);
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
			  raw_write(0,&c,1);
	}

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

	good_move(4, 0);
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
	redoscr();
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
			if ((rc = raw_read(0, buf, BUFSIZ)) < 0)
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
	return ret;
}

void main_loop()
{
	char buf[STRLEN];
	int n;

	display_sites();
	for(;;)
	{
		getdata(23, 0, "ÇëÊäÈë´©ËóÕ¾ÐòºÅ(Ö±½ÓÊäÈë»Ø³µ¿ÉÍË³ö´©Ëó·þÎñ): ", buf, 3, DOECHO, NULL, true);
		if (buf[0] == '\0')
			break;
		n = atoi(buf);
		if (n > 0 && n <= counts)
		{
			bbsnet(n-1);
			display_sites();
		}
	}
/*
    int p= 0;
    int c, n;

    show_all();
    sh(p);
    while(1)
	{
        c= getch();
        if(c== 3|| c== 4|| c== 27|| c< 0)
			break;
        if(c== 257&& p> 0)
			p--;
        if(c== 258&& p< counts- 1)
			p++;
        if(c== 259&& p< counts- 19)
			p+=19;
        if(c== 260&& p>= 19)
			p-=19; 
        if(c== 13|| c== 10)
		{ 
            bbsnet(p);
			show_all();
			sh(p);
			continue;
        }
        for(n=0; n< counts; n++)
		{
			if(str[n]== c)
				p= n;
		}
        sh(p);
    } 
*/
}

int bbsnet_main()
{
	strncpy(user, currentuser->userid, 20);
	modify_user_mode(BBSNET);

    init_data();
    main_loop();
	return 0;
}
