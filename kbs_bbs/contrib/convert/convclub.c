/*
 * 上帝仓促拙作，供鉴赏。
 * 少量修改 by atppp
 * WARNING: olduserec 结构确认确认再确认！
 */

#include "bbs.h"

#define OLD_PASSWDS_FILE ".PASSWDS.OLD"
#define NEW_PASSWDS_FILE ".PASSWDS.NEW"
#undef printf
#undef perror

#define OLDMAXCLUB  128
struct olduserec {                 /* Structure used to hold information in */
    char userid[IDLEN + 2];     /* PASSFILE */
    char flags; /*一些标志，戒网，版面排序之类的*/
    unsigned char title; /*用户级别*/
    time_t firstlogin;
    char lasthost[IPLEN];
    unsigned int numlogins;
    unsigned int numposts;
#ifdef CONV_PASS
    char passwd[OLDPASSLEN];
	char unused_padding[2];
#endif
    char username[NAMELEN];
    unsigned int club_read_rights[OLDMAXCLUB>>5];
    unsigned int club_write_rights[OLDMAXCLUB>>5];
    unsigned char md5passwd[MD5PASSLEN];
#ifndef OS_64BIT
    unsigned int userlevel;
#endif
    time_t lastlogin;
    time_t stay;
#ifdef OS_64BIT /*  align 8 bytes... */
    unsigned int userlevel;
#endif
    int signature;
    unsigned int userdefine[2];
    time_t notedate;
    int noteline;
    int unused_atppp;
    time_t exittime;
	/* 生日数据转移到 userdata 结构中 */
    unsigned int usedspace;     /* used space of user's mailbox, in bytes */

    int unused[7];
};
struct olduserec * olduser_ptr;

static void convert_userec(struct olduserec *olduser, struct userec *user)
{
    int i;
    bzero(user, sizeof(struct userec));
    memcpy(user,olduser,((char*)(olduser->club_read_rights)-(char*)olduser));
    for (i=0;i<(OLDMAXCLUB>>5);i++) {
	user->club_read_rights[i]=olduser->club_read_rights[i];
	user->club_write_rights[i]=olduser->club_write_rights[i];
    }
    memcpy(&user->md5passwd[0],&olduser->md5passwd[0],sizeof(*olduser)-((char*)&olduser->md5passwd[0]-(char*)olduser));
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
    int count;
    count=0;

//    chdir(BBSHOME);

    if ((fd = open(OLD_PASSWDS_FILE, O_RDONLY, 0644)) < 0) {
        perror("open");
        return -1;
    }
    if ((fd2 = open(NEW_PASSWDS_FILE, O_WRONLY | O_CREAT, 0644)) < 0) {
        perror("open");
        return -1;
    }
    fstat(fd, &fs);
	printf("%d %d %d\n",sizeof(struct olduserec) ,sizeof(struct userec),fs.st_size);
    olduser = mmap(NULL, fs.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    if (olduser == MAP_FAILED) {
        perror("mmap");
        return -1;
    }
    olduser_ptr=olduser;
    records = fs.st_size / sizeof(struct olduserec);

    for (i = 0; i < records; i++) {
        ptr = olduser + i;
        if (ptr->userid[0] == '\0')
            continue;
	if (id_invalid(ptr->userid)) {
            printf("%s\n",ptr->userid);
	    continue;
        }
	count++;
        bzero(&user, sizeof(user));
        convert_userec(ptr, &user);
        write(fd2, &user, sizeof(user));
    }
    close(fd2);
    munmap(olduser, fs.st_size);
    printf("converted :%d\n",count);

    return 0;
}
