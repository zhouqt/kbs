#include "bbs.h"

int get_postfilename(char* filename,char* direct)
{
    static const char post_sufix[]="0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int fp;
    time_t now;
    int i;
    char fname[255];
    int pid=getpid();
    /* 自动生成 POST 文件名 */
    now = time(NULL);
    for (i=0;i<10;i++) {
        sprintf(filename,"M.%d.%c%c",now,post_sufix[(pid+i)%62],post_sufix[(pid*i)%62]);
	sprintf(fname,"%s/%s",direct,filename);
    	if ((fp = open(fname,O_CREAT|O_EXCL|O_WRONLY,0644)) != -1) {
		break;
	};
    }
    if (fp==-1) return -1;
    close(fp) ;
    return 0;
}
