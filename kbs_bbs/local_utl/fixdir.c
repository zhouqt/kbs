#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
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

main(int argc,char** argv)
{
        DIR* pdir;
        char* name;
	char buf1[256],buf2[256];
        struct dirent* ent;
	int i;
	FILE* art;
	int file,file1,flag;
	struct fileheader fh;

        if (argc>1) name=argv[1];
        else name=".";
	if (argc == 2) flag=0;
	if (argc == 3) flag=1;
        pdir = opendir(name);
	chdir(name);
	if(!flag){
	sprintf(buf1,"chmod 744 %s",name);
	system(buf1);}
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
	if(!flag){
	sprintf(buf1,"chmod 755 %s",name);
	system(buf1);
	sprintf(buf1,"%s/.DIR",name);
	printf("ok=%d\n",f_mv(".tmpfile", buf1));
	}
}
