#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>
#include "bbs.h"
extern int errno;

struct fileheader data[20000];
int len=0;

int cmpfile(f1,f2)
struct fileheader *f1,*f2;
{
        time_t t1,t2;
        t1=atoi(f1->filename+2);
	t2=atoi(f2->filename+2);
	return t1-t2;
}

void insertfile(fh)
struct fileheader fh;
{
   data[len]=fh;
   len++;
/*
   int i,j;
   for (i=0;i<len;i++)
   {
        time_t t1,t2;
        t1=atoi(fh.filename+2);
	t2=atoi(data[i].filename+2);
	if (t1<t2) break;
   }
   for (j=len;j<i;j--)
	data[j]=data[j-1];	
   data[i]=fh;
   len++;
*/
}

extern char *optarg;
extern int optind, opterr, optopt;

int print_usage(char* prog)
{
	printf("\
Usage: %s [OPTION]... DIR\n\
	-m fix mail mode\n\
	-t test mode. generate .tmpfile only\n\
        -h show this message\n\
examples:\n\
	To fix a user's mail directory:\n\
		%s -m /home/bbs/mail/K/kxn\n\
notice:\n\
	In mailmode it will set all the mail readed.\n\
",prog,prog);
}

int main(int argc,char** argv)
{
        DIR* pdir;
        char* name;
	char buf1[256],buf2[256];
        struct dirent* ent;
	int i;
	FILE* art;
	int file,file1;
	struct fileheader fh;
	int mailmode;
	int flag; /* test mode flag */
	mode_t old_dir_mode; 
	char cwd[255];

	mailmode=0;
	while (1) {
		int c;
        	c=getopt(argc,argv, "mth");
		if (c==-1) break;
				;
		switch (c) {
			case 'm':
				mailmode=1;
				break;
			case 't':
				flag=0;
				break;
			case 'h':
				print_usage(argv[0]);
				return 0;
			case '?':
				printf("%s:Unknown argument.\nTry `%s -h' for more information.\n",
					argv[0],argv[0]);
				return 0;
			default:
				printf ("?? getopt returned character code 0%o ??\n", c);
				return 0;
		}
	}

	if (optind < argc) {
	        struct stat st;
		name=argv[optind++];
		if (optind < argc) {
			printf("%s:Too many arguments.\nTry `%s -h' for more information.\n",
					argv[0],argv[0]);
			return 0;
		}
		if (stat(name,&st)==-1) {
			perror("Can't stat directory:");
			return 0;
		}
		if (!S_ISDIR(st.st_mode)) {
			printf("%s isn't directory:",name);
			return 0;
		}
		old_dir_mode=st.st_mode;
	} else return print_usage(argv[0]);

	if(!flag)
	  chmod(name,0744);
        pdir = opendir(name);
	getcwd(cwd,255);
	chdir(name);
	file=open(".tmpfile",O_CREAT|O_TRUNC|O_WRONLY,0600);
	if (file==0) {
	    perror("open .tmpfile error!\n");
	    closedir(pdir);
	    return -1;
	}
	i=1;
        while (ent=readdir(pdir)) {
	  if ((strcmp(ent->d_name,".DIR"))
	     &&(strcmp(ent->d_name,"."))
	     &&(strcmp(ent->d_name,".."))
	     &&(ent->d_name[0]=='M'))
	  {
		struct stat st;
		if (stat(ent->d_name,&st)) continue;
		if (art=fopen(ent->d_name,"r")) {
			char* p;
			bzero(&fh,sizeof(fh));
			fgets(buf1,256,art);
			if (buf1==0) continue;
			p=strchr(buf1+8,' ');
			if (p)
				*p=0;
			if (p=strchr(buf1+8,'(')) 
				*p=0;
			if (p=strchr(buf1+8,'\n')) 
				*p=0;
			strncpy(fh.owner,buf1+8,STRLEN);
			fgets(buf2,256,art);
			if (buf2==0) continue;
			if (p=strchr(buf2+8,'\n')) 
				*p=0;
			if (p=strchr(buf2+8,'\r')) 
				*p=0;
			strcpy(fh.filename,ent->d_name);
			strncpy(fh.title,buf2+8,STRLEN);
			if (mailmode) {
				fh.accessed[0] |= FILE_READ;
			}
			if (strncmp(buf1,"发信站",6)&&strncmp(buf1,"寄信人: ",8) && strncmp(buf1,"发信人: ",8))
					continue;
			if ((strncmp(buf2,"标  题: ",8))
			   && (strncmp(buf2,"标　题: ",8)))
				continue;
			insertfile(fh);
			fclose(art);
		}
	  }
        }
	qsort(data,len,sizeof(struct fileheader),cmpfile);
	printf("end.len=%d %d",len,len*sizeof(struct fileheader));
	if (write(file,data,len*sizeof(struct fileheader))==-1)
		perror("write error");
	if (close(file))
		printf("close error=%d\n",errno);
        closedir(pdir);
	if(!flag) {
	  struct stat st;
	  int restore=0;
	  struct utimbuf ut;
	  if (stat(".DIR",&st)!=-1)
		  restore=1;
	  printf("\nwrite .DIR ok=%d\n",f_mv(".tmpfile", ".DIR"));
	  chmod(".DIR",st.st_mode);
	  ut.actime=st.st_atime;
	  ut.modtime=st.st_mtime;
	  utime(".DIR",&ut);
	}
	chdir(cwd);
	if(!flag)
	  chmod(name,old_dir_mode);
	return 0;
}
