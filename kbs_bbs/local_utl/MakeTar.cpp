#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include	<sys/wait.h>

#include "Mutex.h"
#include "bbs.h"
#define FTPSCRIPT BBSHOME "/etc/saveanno"
extern "C" int my_system(const char *cmdstring);
typedef	void	Sigfunc(int);	/* for signal handlers */

CThreadMutex	g_mutex;
CThreadCondVar  g_cond(&g_mutex);
int processcount;
#define MAXPROCESS	5


Sigfunc*
Signal(int signo, Sigfunc *func)
{
	struct sigaction	act, oact;

	act.sa_handler = func;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	if (signo == SIGALRM) {
#ifdef	SA_INTERRUPT
		act.sa_flags |= SA_INTERRUPT;	/* SunOS */
#endif
	} else {
#ifdef	SA_RESTART
		act.sa_flags |= SA_RESTART;		/* SVR4, 44BSD */
#endif
	}
	if (sigaction(signo, &act, &oact) < 0)
		return(SIG_ERR);
	return(oact.sa_handler);
}

void __dochild(int t)
{
  int status;
  while (waitpid(-1,&status,WNOHANG)>0) {
	  printf("One process exit...\n");
	  processcount--;
  }
}

void __dochild2(int t)
{
  int status;
  while (waitpid(-1,&status,WNOHANG)>=0) {
  }
}

main(int argc,char** argv)
{
  FILE* filein;
  char* filename="boardlist";
  char* savepath="/backup/bbs/Announce";
  char  buf[1024];

  if ((argc>1)&&!strcmp(argv[1],"-h")) {
	printf("usage:%s tgzpath boardlistfile");
	return 0;
  }
  if (argc>1)  savepath=argv[1];
  if (argc>2)  filename=argv[2];
  processcount=0;
  Signal(SIGCHLD,__dochild);
  if ((filein=fopen(filename,"r"))==NULL) {
	perror("can't open boardlist");
	return -1;
  }

  while (fgets(buf,1024,filein)) {
	if (buf[0]==0) continue;
	buf[strlen(buf)-1]=0;
	while (processcount>=MAXPROCESS) {
		printf("Wait for process exit...\n");
		sleep(40);
	}
	if (!fork()) {
	  char newbuf[1024];
	  FILE* SearchFile;
  	  Signal(SIGCHLD,__dochild2);

	  fclose(filein);
	  chdir(BBSHOME);
	  chdir("0Announce");
	  printf("create new process for %s\n",buf);
	  
  	  if ((SearchFile=fopen(".Search","r"))==NULL) {
		perror("can't open .Search");
		return -1;
	  };
	  
	  strcat(buf,": ");
  	  while (fgets(newbuf,1024,SearchFile)) {
		if (newbuf[0]==0) continue;
		newbuf[strlen(newbuf)-1]=0;
		if (!strncmp(newbuf,buf,strlen(buf))) {
		  char cmd[1024];
		  char* p=newbuf+strlen(buf);
		  buf[strlen(buf)-2]=0;
		  sprintf(cmd,"/usr/local/bin/tar zcf %s/%s.tgz %s",savepath,buf,p);
		  printf("do tar:%s\n",cmd);
		  system(cmd);
		  chdir(savepath);
		  sprintf(cmd,"cat %s|sed -e 's/%%filename%%/%s.tgz/g' | sh",FTPSCRIPT,buf);
		  printf("do ftp:%s\n",cmd);
		  system(cmd);
		  printf("end of %s\n",buf);
		  return 0;
		}
	  }
	  printf("error:can't find %s Announce\n",buf);
	  fclose(SearchFile);
	  return 0;
	}
	processcount++;
  }
  fclose(filein);
  int status;
  while (waitpid(-1,&status,WNOHANG)>0);
}
