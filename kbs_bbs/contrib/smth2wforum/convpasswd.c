/*
 * This program converts .PASSWD from smth to smth_with_wforum 
 */

#include "bbs.h"

#define OLD_PASSWDS_FILE ".PASSWDS.OLD"
#define NEW_PASSWDS_FILE ".PASSWDS.NEW"
#undef printf
#undef perror

struct olduserec {                 /* Structure used to hold information in */
    char userid[IDLEN + 2];     /* PASSFILE */
    char flags[2];
    time_t firstlogin;
    char lasthost[16];
    unsigned int numlogins;
    unsigned int numposts;
  #ifdef CONV_PASS
    char passwd[OLDPASSLEN];
	char unused_padding[2];
#endif
    char username[NAMELEN];
    unsigned int club_read_rights[MAXCLUB>>5];
    unsigned int club_write_rights[MAXCLUB>>5];
    unsigned char md5passwd[MD5PASSLEN];
    unsigned userlevel;
    time_t lastlogin;
    time_t stay;
    int signature;
    unsigned int userdefine;
    time_t notedate;
    int noteline;
    int notemode;
    time_t exittime;
	/* 生日数据转移到 userdata 结构中 */
    unsigned int usedspace;     /* used space of user's mailbox, in bytes */
};

static void convert_userec(struct olduserec *olduser, struct userec *user)
{
    bzero(user, sizeof(struct userec));
    memcpy(user,olduser,sizeof(struct olduserec));
    user->userdefine &= ~DEF_SHOWREALUSERDATA;
}

int main()
{
    struct olduserec *olduser = NULL;
    struct olduserec *ptr = NULL;
    struct userec user;
    int fd, fd2;
    struct stat fs;
    int records;
    int i;
    int allocid;

    chdir(BBSHOME);

    if ((fd = open(OLD_PASSWDS_FILE, O_RDONLY, 0644)) < 0) {
        perror("open");
        return -1;
    }
    if ((fd2 = open(NEW_PASSWDS_FILE, O_WRONLY | O_CREAT, 0644)) < 0) {
        perror("open");
        return -1;
    }
    fstat(fd, &fs);
    olduser = mmap(NULL, fs.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    if (olduser == MAP_FAILED) {
        perror("mmap");
        return -1;
    }
    records = fs.st_size / sizeof(struct olduserec);

    for (i = 0; i < records; i++) {
        ptr = olduser + i;
        if (ptr->userid[0] == '\0')
            continue;
        bzero(&user, sizeof(user));
        convert_userec(ptr, &user);
        write(fd2, &user, sizeof(user));
    }
    close(fd2);
    munmap(olduser, fs.st_size);

    return 0;
}
