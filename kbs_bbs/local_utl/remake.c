#include        <stdio.h>
#include        <sys/types.h>
#include        <sys/stat.h>
#include        <dirent.h>
#include        <limits.h>
#include        "bbs.h"
#define TRUE  1
#define FALSE 0

int  fileflag=1;
int  totalfound=0,totalfile=0;
char    control[80];

struct postnode
{
char filename[20];
int num;
};

report()
{}

main(argc,argv)
int argc;
char *argv[];
{
        char dir[80];

        if(argc<2)
        {
                printf("Usage: %s <BoardName>\n",argv[0]);
                exit(1);
        }
        sprintf(dir,"%s/boards/%s",BBSHOME,argv[1]);
        myftw(dir);
}  

do_remake(path,file)
char *path,*file;
{
        FILE *fp;
        char *ptr,*ptr2;
        char filename[80];
        char buf[256];
        struct fileheader fh;
        int step=0;

        sprintf(filename,"%s/%s",path,file);
        if( (fp=fopen(filename,"r")) == NULL)
        {
                printf("Open error.. \n");
                return;
        }
        strcpy(fh.filename,file);
        fh.level=0;
        memset(&fh.accessed,0,sizeof(fh.accessed));
        while( fgets(buf,256,fp)!=NULL)
        {
                if(strstr(buf,"