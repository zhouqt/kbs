/* Leeward 98.09.24 "borrow" this file from FireBird V 2.66M */

#include "bbs.h"

#define MAX_WELCOME 1 /* By Leeward */

struct FILESHM
{
    char line[FILE_MAXLINE][FILE_BUFSIZE];
    int  fileline;
    int  max;
    time_t  update;
};

struct STATSHM
{
    char line[FILE_MAXLINE][FILE_BUFSIZE];
    time_t update;
};

struct  FILESHM   *welcomeshm=NULL;
struct  FILESHM   *goodbyeshm=NULL;
struct  FILESHM   *issueshm=NULL;
struct  STATSHM   *statshm=NULL;

int
fill_shmfile(mode,fname,shmkey)
int mode;
char *shmkey,*fname;
{
    FILE *fffd ;
    char *ptr;
    char buf[FILE_BUFSIZE];
    struct stat st ;
    time_t  ftime,now;
    int lines=0,nowfn=0,maxnum;
    int iscreate;
    
    struct FILESHM *tmp;

    switch(mode)
    {
    case 1:
        maxnum=MAX_ISSUE;
        break;
    case 2:
        maxnum=MAX_GOODBYE;
        break;
    case 3:
        maxnum=MAX_WELCOME;
        break;
    }
    now=time(0);
    if( stat( fname,&st ) < 0 ) {
        return 0;
    }
    ftime = st.st_mtime;
    tmp =(void *)attach_shm( shmkey, 5000+mode*10, sizeof( struct FILESHM )*maxnum ,&iscreate);
    switch(mode)
    {
    case 1:
        issueshm=tmp;
        break;
    case 2:
        goodbyeshm=tmp;
        break;
    case 3:
        welcomeshm=tmp;
        break;
    }

    if(abs(now-tmp[0].update)<86400&&ftime<tmp[0].update)
    {
        return 1;
    }
    if ((fffd = fopen( fname , "r" )) == NULL)
    {
        return 0;
    }
    while ((fgets( buf,FILE_BUFSIZE, fffd )!=NULL)&&nowfn<maxnum)
    {
        if(lines>FILE_MAXLINE)
            continue;
        if(strstr(buf,"@logout@")||strstr(buf,"@login@"))
        {
            tmp[nowfn].fileline=lines;
            tmp[nowfn].update=now;
            nowfn++;
            lines=0;
            continue;
        }
        ptr=tmp[nowfn].line[lines];
        memcpy( ptr, buf, sizeof(buf) );
        lines++;
    }
    fclose(fffd);
    tmp[nowfn].fileline=lines;
    tmp[nowfn].update=now;
    nowfn++;
    tmp[0].max=nowfn;
    return 1;
}

int
fill_statshmfile(fname,mode)
char *fname;
int  mode;
{
    FILE    *fp;
    time_t  ftime;
    char *ptr;
    char buf[FILE_BUFSIZE];
    struct stat st ;
    time_t  now;
    int lines=0;
    int iscreate;

    if( stat( fname,&st ) < 0 ) {
        return 0;
    }
    ftime = st.st_mtime;
    now=time(0);

    /*     if(mode==0)
         {
    */
    if (statshm==NULL)
        statshm =(void *)attach_shm("STAT_SHMKEY", 5100, sizeof(struct STATSHM)*2,&iscreate);
    /*
         }
    */

    if(abs(now-statshm[mode].update)<86400 && ftime<statshm[mode].update)
    {
        return 1;
    }
    if ((fp = fopen( fname , "r" )) == NULL)
    {
        return 0;
    }
    while ((fgets( buf,FILE_BUFSIZE, fp )!=NULL)&&lines<FILE_MAXLINE)
    {
        ptr=statshm[mode].line[lines];
        memcpy( ptr, buf, sizeof(buf) );
        lines++;
    }
    fclose(fp);
    statshm[mode].update=now;
    return 1;
}

void
show_shmfile(fh)
struct FILESHM *fh;
{
    int i;
    char buf[FILE_BUFSIZE];

    for(i=0;i<fh->fileline;i++)
    {
        strcpy(buf,fh->line[i]);
        showstuff(buf);
    }
}

int
show_statshm(fh,mode)
FILE *fh;
int mode;
{
    int i;
    char buf[FILE_BUFSIZE];

    if (fill_statshmfile(fh,mode))
    {
        clear();

        for(i=0;i<=24;i++)
        {
            if(statshm[mode].line[i]==NULL)
                break;
            strcpy(buf,statshm[mode].line[i]);
            prints(buf);
        }
        if(mode==1)
            shmdt(statshm);
        return 1;
    }
    return 0;
}

void
show_goodbyeshm()
{
    int logouts;

    logouts=goodbyeshm[0].max;
    clear();
    show_shmfile(&goodbyeshm[(currentuser->numlogins%((logouts<=1)?1:logouts))]);
    shmdt(goodbyeshm);
}

void
show_welcomeshm()
{
    int welcomes;

    welcomes=welcomeshm[0].max;
    clear();
    show_shmfile(&welcomeshm[(currentuser->numlogins%((welcomes<=1)?1:welcomes))]);
    pressanykey();
    shmdt(welcomeshm);
}


void
show_issue()
{
    int issues=issueshm[0].max;

    show_shmfile(&issueshm[(issues<=1)?0:
                           ((time(0)/86400)%(issues))]);
    shmdt(issueshm);
}
