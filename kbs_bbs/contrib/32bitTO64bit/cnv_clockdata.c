#include "bbs.h"
#include <sys/types.h>
#include <sys/stat.h>

struct oldclock_struct
{
        int type;
        int clock_time;
        char memo[40];
};

struct newclock_struct
{
        int type;
        time_t clock_time;
        char memo[40];
};

int convindex(char* fname)
{
    int count=0;
    struct newclock_struct newd;
    struct oldclock_struct oldd;
    int fd,writefd;
    struct stat buf;

    if ((fd = open(fname, O_RDONLY, 0664)) == -1) {
//        perror(fname);
        return -1;
    }
    if (fstat(fd,&buf)==-1) {
//        perror(fname);
        return -1;
    }
    if (((double)buf.st_size)/sizeof(oldd)!=buf.st_size/sizeof(oldd)) {
        if (((double)buf.st_size)/sizeof(newd)!=buf.st_size/sizeof(newd))
          printf("fail data %s",fname);
	close(fd);
        return -1;
    }
    
    if ((writefd = open("/tmp/clocktemp", O_WRONLY|O_CREAT, 0664)) == -1) {
        perror(fname);
	close(fd);
        return -1;
    }
    printf("do %s\n",fname);
    for (count=0;;count++) {
//      printf("%d %s\n",count,oldd.id);
        if (read(fd,&oldd,sizeof(oldd))!=sizeof(oldd)) {
            printf("can't read %d\n",count);
            break;
        }
        newd.type=oldd.type;
        newd.clock_time=oldd.clock_time;
        memcpy(&newd.memo,&oldd.memo,40);
        if (write(writefd,&newd,sizeof(newd))!=sizeof(newd)) {
            printf("can't write %d\n",count);
            break;
        }
    }   
    close(fd);
    close(writefd);
    f_mv("/tmp/clocktemp",fname);
}       
    
int changeindex(struct userec* user,char* arg)
{       
    char fname[STRLEN], fname2[STRLEN];
    int fd, fd2, i, count, size;
    struct flock ldata;
    struct stat buf;
        
//    if (strcmp(user->userid,"KCN")) return 0;
    sethomefile(fname, user->userid, "clock.data");
    convindex(fname);
    return 0;
};

main()
{
    chdir(BBSHOME);
    printf("%d %d\n",
        sizeof(struct oldclock_struct),
        sizeof(struct newclock_struct));
    resolve_ucache();
    resolve_boards();
    apply_users(changeindex,NULL);
}

