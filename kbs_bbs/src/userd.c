#include "bbs.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include <signal.h>
/* use  bbslib to provide the services */
static int m_socket;
static char cmdbuf[255];
int getnewuserid2(char * userid)
{
	int result = getnewuserid3(userid);
#ifdef BBSMAIN
	log( "1system", "APPLY: uid %d from %s", result, fromhost );
#endif
	if (result>=0) return result;
#ifdef BBSMAIN
       prints( "抱歉, 由于某些系统原因, 无法注册新的帐号.\n\r" );
       oflush();
       sleep(2);
#endif
       return -1;
}
int getnewuserid3(char * userid)
{
	
	struct sockaddr_in sin;
	fd_set rfds;
	int result;
	struct  timeval tv;
      m_socket = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
      	if (m_socket<0) return -1;
	sin.sin_family=AF_INET;
	sin.sin_port=htons(60000);
	inet_aton("127.0.0.1",&sin.sin_addr);
	if (connect(m_socket,(struct sockaddr*)&sin,sizeof(sin))!=0) {
		close(m_socket);
		return -1;
	}
	sprintf(cmdbuf,"NEW %s",userid);
	write(m_socket,cmdbuf,strlen(cmdbuf));
	FD_ZERO(&rfds);
	FD_SET(m_socket,&rfds);
	tv.tv_sec=5;
	tv.tv_usec=0;
	result = select(m_socket+1,&rfds,NULL,NULL,&tv);
	if (result) 
	{
		int len=read(m_socket,&result,sizeof(result));
		close(m_socket);
		if (len!=sizeof(result)) return -1;
		return result;
	}
	close(m_socket);
	return -1;
}

#ifdef USERD
char *username;
void init(void)
{
	struct sockaddr_in sin;
	int sinlen = sizeof(sin);
	int opt=1;
	if (( m_socket = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP))<0) exit(-1);
	setsockopt(m_socket,SOL_SOCKET,SO_REUSEADDR,&opt,4);
	sin.sin_family = AF_INET;
	sin.sin_port = htons(60000);
	inet_aton("127.0.0.1",&sin.sin_addr);
	if  (0!=bind(m_socket,(struct sockaddr *)&sin,sizeof(sin))) exit(-1);
	if (0!=listen(m_socket,5)) exit(-1);
	resolve_ucache();
}

int getrequest(void)
{
	int len;
	struct sockaddr_in sin;
	int s;
	for (s = accept(m_socket,&sin,&len);;s = accept(m_socket,&sin,&len)) {
		if (s<=0) exit(-1);
		memset(cmdbuf,0,255);
		len = read(s,cmdbuf,255);
		if (len<=0) {close (s) ;continue;}
		strtok(cmdbuf," ");
		username = strtok(NULL," ");
		if (strcmp(cmdbuf,"QUIT")==0) exit(0);
		if (strcmp(cmdbuf,"NEW") == 0) break;
		close(s);
	}
	return s;
}
void putrequest(int sock,int id)
{
	write(sock,&id,sizeof(id));
	close(sock);
}
void godaemon()
{ 
    printf("going daemon\n");
     switch (fork()) {
         case -1: printf("faint, i can't fork.\n"); exit(0); break;
         case 0: break;
         default : exit(0); break;
     }
     setsid();
     setpgrp();
     signal(SIGPIPE,SIG_IGN);
}
int main(int argc,char *argv[])
{
	int sock,id;
	init();
	godaemon();
	while (1) {
		sock = getrequest();
		id = getnewuserid(username);
		putrequest(sock,id);
	}
	return 0;
}

#endif

