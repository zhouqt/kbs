#include "bbs.h"

char *curuserid;

#define DATALEN 100

int check_BM(struct boardheader *bptr)
{
    int fd, data[DATALEN];
    struct flock ldata;
    struct stat buf;
    char direct[PATHLEN];
    
    if ((bptr->level != 0) && !(bptr->level & PERM_POSTMASK))
        return 0;
    if (!chk_BM_instr(bptr->BM, curuserid)) return 0;
    sprintf(direct, "boards/%s/.bm.%s", bptr->filename, curuserid);
    	if ((fd = open(direct, O_RDWR | O_CREAT, 0644)) == -1) return 0;
       ldata.l_type = F_RDLCK;
       ldata.l_whence = 0;
       ldata.l_len = 0;
       ldata.l_start = 0;
       if (fcntl(fd, F_SETLKW, &ldata) == -1) {
            close(fd);
            return 0;
       }
       fstat(fd, &buf);
       if(buf.st_size<DATALEN*sizeof(int)){
       	memset(data, 0, sizeof(int)*DATALEN);
       }
       else
       	read(fd, data, sizeof(int)*DATALEN);
   	ldata.l_type = F_UNLCK;
   	fcntl(fd, F_SETLKW, &ldata);
   	close(fd);
    
    printf("版面名称: %s        版主: %s        停留时间: %d 秒", bptr->filename, bptr->title + 12);
    return 0;
}

int query_BM(struct userec *user, char *arg)
{
    if (!(user->userlevel & PERM_BOARDS))
        return 0;
    printf("%s: ", user->userid);
    curuserid = user->userid;
    apply_boards(check_BM);
    printf("\n");
}

main()
{
    chdir(BBSHOME);
    resolve_ucache();
    resolve_boards();
    apply_users(query_BM, NULL);
}
