/*
 * This program converts .BOARDS and boards data from Firebird 2000 to smth_bbs
 */
#include "bbs.h"
//#include "fb2kstruct.h"

#define MY_BBSHOME BBSHOME
#define OLD_BOARD_FILE ".BOARDS"
#define NEW_BOARD_FILE ".BOARDS.NEW"

#undef printf
#undef perror

struct oldboardheader {             /* This structure is used to hold data in */
        char filename[STRLEN];   /* the BOARDS files */
        unsigned int nowid;
		char unused[16];
        char BM[ 60 - 1];
        char flag;
        char title[STRLEN ];
        unsigned level;
        unsigned char accessed[ 12 ];
};

#define MY_VOTE_FLAG    0x1         
#define MY_NOZAP_FLAG   0x2
#define MY_OUT_FLAG     0x4
#define MY_ANONY_FLAG   0x8
#define MY_NOREPLY_FLAG 0x10 /* No reply board */
#define MY_BOARDJUNK_FLAG  0x20

#define MY_PERM_BASIC      000001
#define MY_PERM_CHAT       000002
#define MY_PERM_PAGE       000004
#define MY_PERM_POST       000010
#define MY_PERM_LOGINOK    000020
#define MY_PERM_DENYPOST   000040
#define MY_PERM_CLOAK      000100
#define MY_PERM_SEECLOAK   000200
#define MY_PERM_XEMPT      000400
#define MY_PERM_WELCOME    001000
#define MY_PERM_BOARDS     002000
#define MY_PERM_ACCOUNTS   004000
#define MY_PERM_CHATCLOAK  010000
#define MY_PERM_OVOTE      020000
#define MY_PERM_SYSOP      040000
#define MY_PERM_POSTMASK  0100000
#define MY_PERM_ANNOUNCE  0200000
#define MY_PERM_OBOARDS   0400000
#define MY_PERM_ACBOARD   01000000
#define MY_PERM_NOZAP     02000000
#define MY_PERM_FORCEPAGE 04000000
#define MY_PERM_EXT_IDLE  010000000
#define MY_PERM_SPECIAL1  020000000
#define MY_PERM_SPECIAL2  040000000
#define MY_PERM_SPECIAL3  0100000000
#define MY_PERM_SPECIAL4  0200000000
#define MY_PERM_SPECIAL5  0400000000
#define MY_PERM_SPECIAL6  01000000000
#define MY_PERM_SPECIAL7  02000000000
#define MY_PERM_SPECIAL8  04000000000

static void convert_flag(struct oldboardheader *oldbrd, struct boardheader *brd)
{
    char oldflag;
    char flag;

    oldflag = oldbrd->flag;
    flag = 0;
    if (oldflag & MY_VOTE_FLAG)
        flag |= BOARD_VOTEFLAG;
    if (oldflag & MY_NOZAP_FLAG)
        flag |= BOARD_NOZAPFLAG;
    if (oldflag & MY_OUT_FLAG)
        flag |= BOARD_OUTFLAG;
    if (oldflag & MY_ANONY_FLAG)
        flag |= BOARD_ANNONY;
    if (oldflag & MY_BOARDJUNK_FLAG)
        flag |= BOARD_JUNK;
    brd->flag = flag;
}

extern char seccode[SECNUM][5];

const char oldseccode[SECNUM][5] = {
		"ab", "cd", "ef", "gh", "ij", "kl", "mn", "opq"
};

static void convert_title(struct oldboardheader *oldbrd, struct boardheader *brd)
{
    int i;

    for (i = 0; i < SECNUM; i++) {
        if (strchr(oldseccode[i], oldbrd->title[0]) != NULL) {
            brd->title[0] = seccode[i][0];
            snprintf(brd->title + 1, STRLEN - 1, "%6.6s      %s", oldbrd->title + 1, oldbrd->title + 11);
        }
    }
}

static void convert_permission(struct oldboardheader *oldbrd, struct boardheader *brd)
{
    unsigned int oldperm;
    unsigned int perm;

    oldperm = oldbrd->level;
    perm = 0;
    if (oldperm & MY_PERM_BASIC)
        perm |= PERM_BASIC;
    if (oldperm & MY_PERM_CHAT)
        perm |= PERM_CHAT;
    if (oldperm & MY_PERM_PAGE)
        perm |= PERM_PAGE;
    if (oldperm & MY_PERM_POST)
        perm |= PERM_POST;
    if (oldperm & MY_PERM_LOGINOK)
        perm |= PERM_LOGINOK;
    if (oldperm & MY_PERM_CLOAK)
        perm |= PERM_CLOAK;
    if (oldperm & MY_PERM_SEECLOAK)
        perm |= PERM_SEECLOAK;
    if (oldperm & MY_PERM_XEMPT)
        perm |= PERM_XEMPT;
    if (oldperm & MY_PERM_WELCOME)
        perm |= PERM_WELCOME;
    if (oldperm & MY_PERM_BOARDS)
        perm |= PERM_BOARDS;
    if (oldperm & MY_PERM_ACCOUNTS)
        perm |= PERM_ACCOUNTS;
    if (oldperm & MY_PERM_CHATCLOAK)
        perm |= PERM_CHATCLOAK;
    if (oldperm & MY_PERM_SYSOP)
        perm |= PERM_ADMIN;     /* convert it */
    if (oldperm & MY_PERM_POSTMASK)
        perm |= PERM_POSTMASK;
    if (oldperm & MY_PERM_ANNOUNCE)
        perm |= PERM_ANNOUNCE;
    if (oldperm & MY_PERM_OBOARDS)
        perm |= PERM_OBOARDS;
    if (oldperm & MY_PERM_ACBOARD)
        perm |= PERM_ACBOARD;
    if (oldperm & MY_PERM_NOZAP)
        perm |= PERM_NOZAP;
    brd->level = perm;
}

#undef time

static void convert_board(struct oldboardheader *oldbrd, struct boardheader *brd)
{
    FILE *fp3;
    char buf[1024];
    int len;
    char *ptr;

    if ((fp3 = fopen("0Announce/.Search", "r")) == NULL) {
        printf("can't open .Search file!");
        exit(0);
    }

    len = strlen(oldbrd->filename);
    while (fgets(buf, sizeof(buf), fp3) != NULL) {
        if (strncasecmp(buf, oldbrd->filename, len) == 0 && buf[len] == ':' && (ptr = strtok(&buf[len + 1], "\t\n")) != NULL) {
            strncpy(brd->ann_path, ptr + 8, 128);       //dont'save groups/
            break;
        }
    }
    fclose(fp3);
    strcpy(brd->filename, oldbrd->filename);
    strncpy(brd->BM, oldbrd->BM, BM_LEN);
	brd->BM[BM_LEN-1]='\0';
    convert_flag(oldbrd, brd);
    convert_title(oldbrd, brd);
    convert_permission(oldbrd, brd);
    brd->createtime=time(0);
}

int main()
{
    struct oldboardheader *olduser = NULL;
    struct oldboardheader *ptr = NULL;
    struct boardheader user;
    int fd, fd2;
    struct stat fs;
    int records;
    int i;
    int allocid;

    chdir(BBSHOME);
    if ((fd = open(OLD_BOARD_FILE, O_RDONLY, 0644)) < 0) {
        perror("open");
        return -1;
    }
    if ((fd2 = open(NEW_BOARD_FILE, O_WRONLY | O_CREAT, 0644)) < 0) {
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
    records = fs.st_size / sizeof(struct oldboardheader);
    for (i = 0; i < records; i++) {
        ptr = olduser + i;
        if (ptr->filename[0] == '\0')
            continue;
        bzero(&user, sizeof(user));
        convert_board(ptr, &user);
        write(fd2, &user, sizeof(user));
    }
    close(fd2);
    munmap(olduser, fs.st_size);

    return 0;
}
