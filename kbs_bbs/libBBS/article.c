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

/*added by alex, 96.9.12*/
void postreport(const char * posttitle, int post_num, char *board) 
{
    struct posttop
    {
        char author[13];              /* author name */
        char board[IDLEN+6];               /* board name */
        char title[66];               /* title name */
        time_t date;                  /* last post's date */
        int number;                   /* post number */
    }    postlog;

    int fd ;
    static int disable = NA ;
    char* buf;

    if(disable)
        return ;
    if(!strcmp(board, "test") || !strcmp(board,"junk") || !strcmp(board,"WaterWorld"))
        return;
    /*if((fd = open(".post",O_WRONLY|O_CREAT,0644)) != -1 ) {*/
    if((fd = open(".post.X",O_WRONLY|O_CREAT,0644)) != -1 ){
        memset(&postlog, 0, sizeof(postlog));
        time(&(postlog.date));
        strcpy(postlog.author, currentuser->userid);
        strcpy(postlog.board, board);
        if( strncasecmp( posttitle, "Re:", 3 ) == 0 )
            strcpy(postlog.title, posttitle+4);
        else
            strcpy(postlog.title, posttitle);
        postlog.number = post_num;
        flock(fd,LOCK_EX) ;
        lseek(fd,0,SEEK_END) ;
        write(fd, (char *)&postlog, sizeof(postlog));
        flock(fd,LOCK_UN) ;
        close(fd) ;
        return ;
    }
    disable = YEA ;
    return ;
}


