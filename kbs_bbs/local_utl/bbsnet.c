/*bbsnet.c*/
// NJU bbsnet, preview version, zhch@dii.nju.edu.cn, 2000.3.23 //
// HIT bbsnet, Changed by Sunner, sun@bbs.hit.edu.cn, 2000.6.11
// zixia bbsnet, Changed by zdh, dh_zheng@hotmail.com,2001.12.04

#include <stdio.h>
#include <termios.h>
#include <string.h>
#include <time.h>

#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/telnet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <math.h>
#include <sys/file.h>

char host1[100][40], host2[100][40], ip[100][40];
char ip_zdh[40];//”√¿¥Ω” ‹”√ªßµƒip
static char buf[100]; // output buffer
int port[100], counts= 0;
char str[]= "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()";

char datafile[80]= BBSHOME "/etc/bbsnet.ini";
char userid[80]=   "unknown";

init_data() {
    FILE *fp;
    char t[256], *t1, *t2, *t3, *t4;
    fp= fopen(datafile, "r");
    if(fp== NULL) return;
    while(fgets(t, 255, fp)&& counts <= 72) {
        t1= strtok(t, " \t");
        t2= strtok(NULL, " \t\n");
        t3= strtok(NULL, " \t\n");
        t4= strtok(NULL, " \t\n");
        if(t1[0]== '#'|| t1== NULL|| t2== NULL|| t3== NULL) continue;
        strncpy(host1[counts], t1, 16);
        strncpy(host2[counts], t2, 36);
        strncpy(ip[counts], t3, 36);
        port[counts]= t4? atoi(t4): 23;
        counts++;
    }
    fclose(fp);
}

sh(int n) {
    static oldn= -1;
    if(n>= counts) return;
    if(oldn >=0) {
        locate(oldn);
        printf("[1;32m %c.[m%s", str[oldn], host2[oldn]);
    }
    oldn= n;
    if (strcmp(host2[n],"∞„»Ù≤®¡_√‹")==0){  //≈–∂œ◊‘∂®“Â’æµ„
    printf("[22;3H[1;37m π”√∑Ω∑®: ∞¥ªÿ≥µ∫Û ‰»Îip°£[1;33m[22;32H[1;37m ’æ√˚: [1;33m◊‘∂®“Â’æµ„              \r\n");
    printf("[1;37m[23;3H¡¨Õ˘: [1;33m__________                   [21;1H");
    }
    else{
    printf("[22;3H[1;37mµ•Œª: [1;33m%s                   [22;32H[1;37m ’æ√˚: [1;33m%s              \r\n", host1[n], host2[n]);
    printf("[1;37m[23;3H¡¨Õ˘: [1;33m%s                   [21;1H", ip[n]);
    }
    locate(n);
    printf("[%c][1;42m%s[m", str[n], host2[n]);
}

show_all() {
    int n;
    printf("[1H[2J[m");
    printf("©≥©•©•©•©•©•©•©•©•©•©•©•©•©•©•©•[1;35m ‘¬  π‚  ±¶  ∫– [m©•©•©•©•©•©•©•©•©•©•©•©•©•©•©•©∑\r\n");
    for(n= 1; n< 22; n++)
        printf("©ß                                                                            ©ß\r\n");
    printf("©ß                                                               [1;36m∞¥[1;33mCtrl+C[1;36mÕÀ≥ˆ[m ©ß\r\n");
    printf("©ª©•©•©•©•©•©•©•©•©•©•©•©•©•©•©•©•©•©•©•©•©•©•©•©•©•©•©•©•©•©•©•©•©•©•©•©•©•©•©ø");
    printf("[21;3H©§©§©§[1;36m∞„»Ù≤®¡_√‹[m©§©§©§©§©§©§©§[1;36m∞„-»Ù-≤®-¡_-√€~~[m©§©§©§©§©§©§©§[1;36m∞„»Ù≤®¡_√‹[m©§©§©§");
    //printf("[21;3H[1;36m∞„»Ù≤®¡_√‹∞„»Ù≤®¡_√‹∞„»Ù≤®¡_√‹∞„»Ù≤®¡_√‹∞„»Ù≤®¡_√‹∞„»Ù≤®¡_√‹∞„»Ù≤®¡_√‹∞„»Ù≤®[m");

    for(n= 0; n< counts; n++) {
        locate(n);
        printf("[1;32m %c.[m%s", str[n], host2[n]);
    }
}

locate(int n) {
    int x, y;
    char buf[20];
    if(n>= counts) return;
    y= n%19 + 2;
    x= n /19 * 24+ 4;

    sprintf(buf, "[%d;%dH", y, x);
    printf(buf);
}

int getch()
{
    int c, d, e;
    static lastc= 0;
    c= getchar();
    if(c== 10&& lastc== 13) c=getchar();
    lastc= c;
    if(c!= 27) return c;
    d= getchar();
    e= getchar();
    if(d== 27) return 27;
    if(e== 'A') return 257;
    if(e== 'B') return 258;
    if(e== 'C') return 259;
    if(e== 'D') return 260;
    return 0;
}


void QuitTime()
{
    reset_tty();
        exit(0);
}

void SetQuitTime()
{
        signal(SIGALRM, QuitTime);
        alarm(60);
}

main_loop() {
    int p= 0;
    int c, n;
L:
    show_all();
    sh(p);
    fflush(stdout);
    while(1) {
        c= getch();
        if(c== 3|| c== 4|| c== 27|| c< 0) break;
        if(c== 257&& p> 0) p--;
        if(c== 258&& p< counts- 1) p++;
        if(c== 259&& p< counts- 19) p+=19;
        if(c== 260&& p>= 19) p-=19; 
        if(c== 13|| c== 10) { 
            bbsnet(p);
            goto L;
        }
        for(n=0; n< counts; n++) if(str[n]== c) p= n;
        sh(p);
        fflush(stdout);
    } 
}

bbsnet(int n) {
    char buf1[40],buf2[39],c,buf3[2];//‘ˆº”µƒ±‰¡ø
    int i;//	
    int l;//≈–∂œ «≤ª «port
    int j,m;
    if(n>= counts) return;
    
    if (strcmp(host2[n],"∞„»Ù≤®¡_√‹")==0){//»Áπ˚ «◊‘∂®“Â’æµ„£¨µ»¥˝ ‰»ÎipªÚ”Ú√˚

    for (i=0;i<25;i++) {buf1[i]='\0';buf2[i]='\0';}
    prints("[1;32m[23;3H¡¨Õ˘: ");
    refresh();
    j=0;
    l=0;
    for (i=0;i<30;i++){
        c= getch();
        if (c==' '||c=='\015'||c=='\0'||c=='\n') break;
        if (c==':'){
                 l=1;
                 sprintf(buf3,"%c",c);
                 prints("[0;1m");
                 prints(buf3);
                 prints("[1;33m");
        	 refresh();
                 strncpy(ip_zdh,buf1,40);
    	         strncpy(ip[n],ip_zdh,40);
                 for (m=0;m<20;m++) {buf1[m]='\0';buf2[m]='\0';}      
                 j=0;
        }
        if ((c>='0'&&c<='9')||(c=='.')||(c>='A'&&c<='Z')||(c>='a'&&c<='z'))
        {
        sprintf(buf3,"%c",c);
        sprintf(buf2,"%s%c",buf1,c);
        sprintf(buf1,"%s",buf2);
        prints(buf3);
        refresh();
        j=j+1;
        }
        if ((c==8)&&(j>=1)){
        for(m=j-1;m<20;m++) {buf1[m]='\0';buf2[m]='\0';}
        sprintf(buf3,"%c",c);
        prints(buf3);
        refresh();
        i--;
        j--;
        }
        
        refresh();
     }
     if(l==0){
        strncpy(ip_zdh,buf1,40);
    	strcpy(ip[n],ip_zdh);
    	}
    	else{port[n]=atoi(buf1);}
     }


    printf("[1H[2J[1;32mo ¡¨Õ˘: %s (%s)\r\n", host2[n], ip[n]);
    printf("%s\r\n\r\n[m", "o ¡¨≤ª…œ ±«Î…‘∫Ú£¨30 √Î∫ÛΩ´◊‘∂ØÕÀ≥ˆ");
    fflush(stdout);
    proc(host2[n],ip[n], port[n]);
}

main(int n, char* cmd[]) {
    SetQuitTime();
    get_tty();
    init_tty();
    if(n>= 2) strcpy(datafile, cmd[1]);
    if(n>= 3) strcpy(userid, cmd[2]);
    init_data();
    main_loop();
    printf("[m");
    reset_tty();
}

syslog(char* s) {
        char buf[512], timestr[16], *thetime, ip[16];
        time_t dtime;
        int fd;

	struct sockaddr_in tmpsin;
	int tmplen = sizeof(struct sockaddr_in);

	getpeername(0, &tmpsin,(int *) &tmplen);
	strncpy( ip, inet_ntoa(tmpsin.sin_addr), 16 );

        time(&dtime);
        thetime = (char*) ctime(&dtime);
        strncpy(timestr, &(thetime[4]), 15);
        timestr[15] = '\0';
        //sprintf(buf,"%s %s@%s %s\n", timestr, userid, ip, s) ;
        snprintf(buf, 512, "%s %-15s %-5d %s\n", timestr, ip, (int)getpid(), s) ;
        fd=open(BBSHOME "/reclog/bbsnet.log", O_CREAT | O_APPEND | O_WRONLY );
	flock( fd, LOCK_EX );
        write( fd, buf, strlen(buf) );
	flock( fd, LOCK_UN );
        close(fd);
}

#define stty(fd, data) tcsetattr( fd, TCSANOW, data )
#define gtty(fd, data) tcgetattr( fd, data )
struct termios tty_state, tty_new ;

get_tty()
{
    if(gtty(1,&tty_state) < 0)
        return 0;
    return 1 ;
}

init_tty()
{
    long vdisable;

    memcpy( &tty_new, &tty_state, sizeof(tty_new)) ;
    tty_new.c_lflag &= ~(ICANON|ECHO|ECHOE|ECHOK|ISIG);
    tty_new.c_cflag &= ~CSIZE;
    tty_new.c_cflag |= CS8;
    tty_new.c_cc[ VMIN ] = 1;
    tty_new.c_cc[ VTIME ] = 0;
    if ((vdisable = fpathconf(STDIN_FILENO, _PC_VDISABLE)) >= 0)
      {
      tty_new.c_cc[VSTART] = vdisable;
      tty_new.c_cc[VSTOP]  = vdisable;
      tty_new.c_cc[VLNEXT] = vdisable;
      }
    tcsetattr(1, TCSANOW, &tty_new);
}

reset_tty()
{
    stty(1,&tty_state) ;
}

proc(char *hostname, char *server,int port)
{
        int fd;
        struct sockaddr_in blah;
        struct hostent *he;
        int result;
        unsigned char buf[2048];
        fd_set readfds;
        struct timeval tv;

  	struct sockaddr_in tmpsin;
  	int tmplen = sizeof(struct sockaddr_in);
  	getpeername(0, &tmpsin,(int *) &tmplen);

        signal(SIGALRM, QuitTime);
        alarm(30);
        bzero((char *)&blah,sizeof(blah));
        blah.sin_family=AF_INET;
        blah.sin_addr.s_addr=inet_addr(server);
        blah.sin_port=htons(port);
        fflush(stdout);
        fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if ((he = gethostbyname(server)) != NULL)
            bcopy(he->h_addr, (char *)&blah.sin_addr, he->h_length);
        else
            if ((blah.sin_addr.s_addr = inet_addr(server)) < 0) return;

  	if( (tmpsin.sin_addr.s_addr&0xff00)==(blah.sin_addr.s_addr&0xff00) ){
          	printf("\n\n\n[1;31m∞›Õ–~ ‘¬π‚±¶∫–‘ı√¥Àµ“≤ «±¶ŒÔ£¨ƒ˙ƒ‹÷±Ω”»•µƒµÿ∑Ωªπ «÷±Ω”»•∞…... :P[m\n\n\n\n\n");
  		fflush(stdout);
  		sleep( 3 );
  		return;
  	}

        if(connect(fd,(struct sockaddr *)&blah,16)<0) return;

        signal(SIGALRM, SIG_IGN);
        printf("“—æ≠¡¨Ω”…œ÷˜ª˙£¨∞¥'ctrl+]'øÏÀŸÕÀ≥ˆ°£\n");
		sprintf(buf, "Login %s (%s)", hostname, server);
		syslog(buf);

        while(1)
        {
                tv.tv_sec = 2400;
                tv.tv_usec = 0;
                FD_ZERO(&readfds) ;
                FD_SET(fd,&readfds);
                FD_SET(0,&readfds);

                result = select(fd + 1,&readfds,NULL,NULL,&tv);
                if (result <= 0) break;
                if (FD_ISSET(0,&readfds))
                {
                        result = read(0,buf,2048);
                        if (result<=0) break;
                        if(result==1&&(buf[0]==10||buf[0]==13)){
							buf[0]=13;
							buf[1]=10;
							result=2;
						}
                        //if(buf[0]==29){close(fd);return;}
                        if(buf[0]==29){close(fd);break;}
                        write(fd,buf,result);
                }else{
                        result=read(fd,buf,2048);
                        if (result<=0) break;
                        if(strchr(buf, 255))
                          telnetopt(fd, buf,result);
                        else
                          write(0,buf,result);
                }
        } 
		sprintf(buf, "Logout %s (%s)", hostname, server);
		syslog(buf);
}

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
                      if((d==253)&&(e==3||e==24)) {
                       tmp[0]=255;tmp[1]=251;tmp[2]=e;write(fd,tmp,3);continue;}
                      if((d==251||d==252)&&(e==1||e==3||e==24)) {
                       tmp[0]=255;tmp[1]=253;tmp[2]=e;write(fd,tmp,3);continue;}
                      if(d==251||d==252) {
                       tmp[0]=255;tmp[1]=254;tmp[2]=e;write(fd,tmp,3);continue;}
                      if(d==253||d==254) {
                       tmp[0]=255;tmp[1]=252;tmp[2]=e;write(fd,tmp,3);continue;}
                      if(d==250) {
                         while(e!=240&&pp<max)e=buf[pp++];
                         tmp[0]=255;tmp[1]=250;tmp[2]=24;tmp[3]=0;tmp[4]=65;
                         tmp[5]=78;tmp[6]=83;tmp[7]=73;tmp[8]=255;tmp[9]=240;
                         write(fd,tmp,10);
                      }
                   }
                   else
                      write(0,&c,1);
                }
}
int refresh() {
    write(0, buf, strlen(buf));
    buf[0]= 0;
}

int prints(char* b) {
    strcat(buf, b);
}
