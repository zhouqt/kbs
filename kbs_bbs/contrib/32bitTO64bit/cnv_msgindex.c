#include "bbs.h"

struct oldmsghead {
    int pos, len;
    char sent;
    char mode;
    char id[IDLEN+2];
    int time;
    int frompid, topid;
};

struct newmsghead {
    int pos, len;
    char sent;
    char mode;
    char id[IDLEN+2];
    time_t time;
    int frompid, topid;
};
void convindex(char* fname)
{
    int count=0;
    struct newmsghead newd;
    struct oldmsghead oldd;
    int fd,writefd;

    if ((fd = open(fname, O_RDONLY, 0664)) == -1) {
        perror(fname);
        return -1;
    }

    if ((writefd = open("/tmp/msgtemp", O_WRONLY|O_CREAT, 0664)) == -1) {
        perror(fname);
        return -1;
    }
    if (read(fd,&count,sizeof(count))==sizeof(count)) {
        write(writefd,&count,sizeof(count));
    for (count=0;;count++) {
//      printf("%d %s\n",count,oldd.id);
        if (read(fd,&oldd,sizeof(oldd))!=sizeof(oldd))
            break;
        newd.pos=oldd.pos;
        newd.len=oldd.len;
        newd.sent=oldd.sent;
        newd.mode=oldd.mode;
        memcpy(&newd.id,&oldd.id,IDLEN+2);
        newd.time=oldd.time;
        newd.frompid=oldd.frompid;
        newd.topid=oldd.topid;
        if (write(writefd,&newd,sizeof(newd))!=sizeof(newd))
            break;
    }   
    }
    close(fd);
    close(writefd);
    f_mv("/tmp/msgtemp",fname);
}       
    
int changeindex(struct userec* user,char* arg)
{       
    char fname[STRLEN], fname2[STRLEN];
    int fd, fd2, i, count, size;
    struct flock ldata;
    struct stat buf;
        
//    if (strcmp(user->userid,"KCN")) return 0;
    sethomefile(fname, user->userid, "msgindex");
    convindex(fname);
        
    sethomefile(fname, user->userid, "msgindex2");
    convindex(fname);

    sethomefile(fname, user->userid, "msgindex3");
    convindex(fname);
    return 0;
};

main()
{
    chdir(BBSHOME);
    printf("%d %d\n",
        sizeof(struct oldmsghead),
        sizeof(struct newmsghead));
    resolve_ucache();
    resolve_boards();
    apply_users(changeindex,NULL);
}

