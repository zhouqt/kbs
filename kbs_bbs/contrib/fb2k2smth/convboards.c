/*
 * This program converts .BOARDS and boards data from Firebird 2000 to smth_bbs
 */
#include "bbs.h"
#include "fb2kstruct.h"

#define MY_BBSHOME BBSHOME
#define BOARDS_FILE ".BOARDS.OLD"

#undef printf
#undef perror

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
    brd->flag = flag;
}

extern char seccode[SECNUM][5];

const char oldseccode[SECNUM][5] = {
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "a", "m"
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

static void set_article_ids(struct oldfileheader *base, int id, struct fileheader *fh, struct oldfileheader *oldfh)
{
    int i;
    struct fileheader *ptr;
    int index;
    int len;

    fh->id = id;
    if (id == 1 || strncmp(oldfh->title, "Re: ", 4) != 0) {
        fh->reid = id;
        fh->groupid = id;
        return;
    }
    len = strlen(oldfh->title + 4);
    index = 0;
    for (i = id - 1; i > 0; i--) {
        ptr = (struct fileheader *) base + i - 1;
        if (strncmp(ptr->title, "Re: ", 4) != 0) {
            if (strncmp(ptr->title, oldfh->title + 4, len) != 0)
                continue;
            else {
                index = i;
                break;
            }
        } else {
            if (strncmp(ptr->title + 4, oldfh->title + 4, len) != 0)
                continue;
            else {
                index = i;
                break;
            }
        }
    }
    if (index == 0) {
        fh->reid = id;
        fh->groupid = id;
    } else {
        ptr = (struct fileheader *) base + index - 1;
        fh->reid = ptr->groupid;
        fh->groupid = ptr->groupid;
    }
}

static void convert_article_flag(struct oldfileheader *oldfh, struct fileheader *fh)
{
    char flag1, flag2;
    char oldflag;

    flag1 = flag2 = 0;
    oldflag = oldfh->accessed[0];
    if (oldflag & MY_FILE_READ)
        flag1 |= FILE_READ;
    if (oldflag & MY_FILE_OWND)
        flag1 |= FILE_OWND;
    if (oldflag & MY_FILE_VISIT)
        flag1 |= FILE_IMPORTED;
    if (oldflag & MY_FILE_MARKED)
        flag1 |= FILE_MARKED;
    if (oldflag & MY_FILE_DIGEST)
        flag1 |= FILE_DIGEST;
    fh->accessed[0] = flag1;
    fh->accessed[1] = flag2;
}

static int convert_board_articles(struct boardheader *brd)
{
    char dir[256];
    struct stat fs;
    int records;
    int i;
    int fd;
    struct oldfileheader *oldfh;
    struct oldfileheader *ptr;
    struct fileheader fh;

    setbdir(DIR_MODE_NORMAL, dir, brd->filename);
    if ((fd = open(dir, O_RDWR, 0644)) < 0) {
        fprintf(stderr, "Converting %s's articles failed.\n", brd->filename);
        return;
    }
    fstat(fd, &fs);
    records = fs.st_size / sizeof(struct oldfileheader);
    oldfh = mmap(NULL, fs.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (oldfh == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return;
    }
    for (i = 0; i < records; i++) {
        ptr = oldfh + i;
        bzero(&fh, sizeof(fh));
        strncpy(fh.filename, ptr->filename, FILENAME_LEN - 1);
        fh.filename[FILENAME_LEN - 1] = '\0';
        strncpy(fh.title, ptr->title, ARTICLE_TITLE_LEN - 1);
        fh.title[ARTICLE_TITLE_LEN - 1] = 0;
        set_article_ids(oldfh, i + 1, &fh, ptr);
        fh.innflag[0] = ptr->filename[STRLEN - 2];
        fh.innflag[1] = ptr->filename[STRLEN - 1];
        strncpy(fh.owner, ptr->owner, OWNER_LEN - 1);
        fh.owner[OWNER_LEN - 1] = '\0';
        /*
         * fh.level = ptr->level;
         */
        convert_article_flag(ptr, &fh);
        memcpy(ptr, &fh, sizeof(fh));
    }
    munmap(oldfh, fs.st_size);
    close(fd);
    /* brd->nowid = records; TODO correct it. */
    printf("Converting board %s's articles successfully\n", brd->filename);
}

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
    strcpy(brd->BM, oldbrd->BM);
    convert_flag(oldbrd, brd);
    convert_title(oldbrd, brd);
    convert_permission(oldbrd, brd);
//    convert_board_articles(brd);
}

int main()
{
    struct oldboardheader *oldbrd = NULL;
    struct oldboardheader *ptr = NULL;
    struct boardheader board;
    int fd;
    struct stat fs;
    int records;
    int i;
    int allocid;

    chdir(BBSHOME);
    printf("resolve boards...\n");
    resolve_boards();
    printf("resolve ucache...\n");
    resolve_ucache();
    printf("resolve utmp...\n");
    resolve_utmp();
    printf("resolve OK...\n");

    chdir(MY_BBSHOME);
    if ((fd = open(BOARDS_FILE, O_RDONLY, 0644)) < 0) {
        perror("open");
        return -1;
    }
    fstat(fd, &fs);
    oldbrd = mmap(NULL, fs.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    if (oldbrd == MAP_FAILED) {
        perror("mmap");
        return -1;
    }
    records = fs.st_size / sizeof(struct oldboardheader);
    for (i = 0; i < records; i++) {
        ptr = oldbrd + i;
        if (ptr->filename[0] == '\0')
            continue;
        bzero(&board, sizeof(board));
        convert_board(ptr, &board);
        if (add_board(&board) < 0) {
            fprintf(stderr, "New board %s failed\n", board.filename);
            continue;
        } else {
            printf("Creating board %s successfully\n", board.filename);
        }
        bbslog("user", "add brd %s", board.filename);
    }
    munmap(oldbrd, fs.st_size);

    return 0;
}
