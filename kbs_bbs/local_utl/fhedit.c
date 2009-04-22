
#include <stdlib.h>
#include <getopt.h>
#include "bbs.h"

int fd;
char *optarg = NULL;
static int binary = 0;
static int simple = 0;
static int operate = 0;
static int accessed9 = 0;
struct fileheader fhopt, fhbool;
char indexfile[STRLEN] = ".DIR";

int ruler(char* accessed)
{
    int aid, bid, i, j, flag, show;
    char newa[4], key;
    char atext[4][8][20] = {
        {"#标记", "OWND标记", "%标记", "m标记", "g标记", "信件R标记", "信件F标记", "绿豆糕"},
        {";标记", "X标记", "回文转寄", "已推荐", "_标记", "@标记", "", "TeX标记"},
        {"", "", "", "", "", "", "", ""},
        {"", "", "", "", "", "", "", ""}
    };
    aid = 0;
    bid = 7;
    memcpy(newa, accessed, 4);
    flag = 1;
    show = 1;
    while (flag) {
        if (show) {
            printf("A                 B                 C                 D                      SPACE:switch  Q:exit\n");
            printf("7 6 5 4 3 2 1 0   7 6 5 4 3 2 1 0   7 6 5 4 3 2 1 0   7 6 5 4 3 2 1 0     %s\n", atext[aid][bid]);
            for (i=0; i<4; i++) {
                for (j=7; j>=0; j--) {
                    if ((i == aid) && (j == bid))
                        printf("\033[42m");
                    if ((newa[i] & (1 << j)) != (accessed[i] & (1 << j)))
                        printf("\033[33m");
                    if (newa[i] & (1 << j))
                        printf("1 ");
                    else
                        printf("0 ");
                    printf("\033[m");
                }
                printf("  ");
            }
            printf("\n");
        }
        key = getchar();
        show = 1;
        switch (key) {
            case 'a':
            case 'A':
                aid = 0;
                break;
            case 'b':
            case 'B':
                aid = 1;
                break;
            case 'c':
            case 'C':
                aid = 2;
                break;
            case 'd':
            case 'D':
                aid = 3;
                break;
            case ' ':
                if (newa[aid] & (1 << bid))
                    newa[aid] &= ~(1 << bid);
                else
                    newa[aid] |= 1 << bid;
                break;
            case 10:
                show = 0;
                break;
            case 'q':
            case 'Q':
                flag = 0;
                break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
                bid = key - '0';
                break;
        }
    }
    memcpy(accessed, newa, 4);
    return 1;
}

int fh_count(char* dirfile)
{
    struct stat st;
    stat(dirfile, &st);
    printf("%ld\n", st.st_size / sizeof(struct fileheader));
    return 1;
}

int fh_list(int num, int count)
{
    struct fileheader fh;
    int i, posttime;
    char timetext[STRLEN];
    if (num <= 0)
        num = 1;
    lseek(fd, (num - 1) * sizeof(struct fileheader), SEEK_SET);
    for (i=0; i<count; i++) {
        if (read(fd, &fh, sizeof(struct fileheader)) > 0) {
            if (fhbool.filename[0])
                if (strcmp(fh.filename, fhopt.filename) != 0)
                    continue;
            if (fhbool.id)
                if (fh.id != fhopt.id)
                    continue;
            if (fhbool.groupid)
                if (fh.groupid != fhopt.groupid)
                    continue;
            if (fhbool.reid)
                if (fh.reid != fhopt.reid)
                    continue;
            if (fhbool.o_bid)
                if (fh.o_bid != fhopt.o_bid)
                    continue;
            if (fhbool.o_id)
                if (fh.o_id != fhopt.o_id)
                    continue;
            if (fhbool.o_groupid)
                if (fh.o_groupid != fhopt.o_groupid)
                    continue;
            if (fhbool.o_reid)
                if (fh.o_reid != fhopt.o_reid)
                    continue;
            if (fhbool.innflag[0])
                if ((fh.innflag[0] != fhopt.innflag[0]) || (fh.innflag[1] != fhopt.innflag[1]))
                    continue;
            if (fhbool.owner[0])
                if (strcmp(fh.owner, fhopt.owner) != 0)
                    continue;
            if (fhbool.eff_size)
                if (fh.eff_size != fhopt.eff_size)
                    continue;
            if (fhbool.attachment)
                if (fh.attachment != fhopt.attachment)
                    continue;
            if (fhbool.title[0])
                if (strcmp(fh.title, fhopt.title) != 0)
                    continue;
            if (fhbool.accessed[0])
                if (fh.accessed[0] != fhopt.accessed[0])
                    continue;
            if (fhbool.accessed[1])
                if (fh.accessed[1] != fhopt.accessed[1])
                    continue;
            if (fhbool.accessed[2])
                if (fh.accessed[2] != fhopt.accessed[2])
                    continue;
            if (fhbool.accessed[3])
                if (fh.accessed[3] != fhopt.accessed[3])
                    continue;
            if (binary) {
                fwrite(&fh, sizeof(struct fileheader), 1, stdout);
            } else {
                if (simple)
                    printf("%d\n", num + i);
                else {
                    posttime = get_posttime(&fh);
                    ctime_r((time_t *)&posttime, timetext);
                    timetext[strlen(timetext)-1] = 0;
                    printf("%7d %-14s %-25.25s %s\n", num + i, fh.owner, timetext, fh.title);
                }
            }
        }
    }
    return 1;
}

int fh_view(int num)
{
    struct fileheader fh;
    int posttime, i, j;
    char timetext[STRLEN];
    if (num <= 0)
        num = 1;
    lseek(fd, (num - 1) * sizeof(struct fileheader), SEEK_SET);
    if (read(fd, &fh, sizeof(struct fileheader)) > 0) {
        printf("filename: %s\n", fh.filename);
        printf("id: %d  groupid: %d  reid: %d\n", fh.id, fh.groupid, fh.reid);
        printf("o_bid: %d  o_id: %d  o_groupid: %d  o_reid: %d\n", fh.o_bid, fh.o_id, fh.o_groupid, fh.o_reid);
        printf("innflag: %c%c  owner: %s  eff_size: %d\n", fh.innflag[0], fh.innflag[1], fh.owner, fh.eff_size);
        posttime = get_posttime(&fh);
        ctime_r((time_t *)&posttime, timetext);
        timetext[strlen(timetext)-1] = 0;
        printf("posttime: %d(%s)  attachment: %d\n", posttime, timetext, fh.attachment);
        printf("title: %s\n", fh.title);
        printf("accessed: ");
        for (i=0; i<4; i++) {
            for (j=7; j>=0; j--)
                printf("%c", (fh.accessed[i] & (1 << j)) ? '1' : '0');
            printf(" ");
        }
        printf("\n");
    }
    return 1;
}

int fh_modify(int num)
{
    struct fileheader fh;
    if (num <= 0) {
        printf("number must be specified.\n");
        return 0;
    }
    lseek(fd, (num - 1) * sizeof(struct fileheader), SEEK_SET);
    if (binary) {
        while (fread(&fh, sizeof(struct fileheader), 1, stdin) > 0)
            write(fd, &fh, sizeof(struct fileheader));
    } else {
        if (read(fd, &fh, sizeof(struct fileheader)) <= 0)
            return 0;
        lseek(fd, (num - 1) * sizeof(struct fileheader), SEEK_SET);
        if (fhbool.filename[0])
            strncpy(fh.filename, fhopt.filename, FILENAME_LEN);
        if (fhbool.id)
            fh.id = fhopt.id;
        if (fhbool.groupid)
            fh.groupid = fhopt.groupid;
        if (fhbool.reid)
            fh.reid = fhopt.reid;
        if (fhbool.o_bid)
            fh.o_bid = fhopt.o_id;
        if (fhbool.o_id)
            fh.o_id = fhopt.o_id;
        if (fhbool.o_groupid)
            fh.o_groupid = fhopt.o_groupid;
        if (fhbool.o_reid)
            fh.o_reid = fhopt.o_reid;
        if (fhbool.innflag[0])
            strncpy(fh.innflag, fhopt.innflag, 2);
        if (fhbool.owner[0])
            strncpy(fh.owner, fhopt.owner, OWNER_LEN);
        if (fhbool.eff_size)
            fh.eff_size = fhopt.eff_size;
        if (fhbool.attachment)
            fh.attachment = fhopt.attachment;
        if (fhbool.title[0])
            strncpy(fh.title, fhopt.title, ARTICLE_TITLE_LEN);
        if (accessed9)
            ruler((char *)fh.accessed);
        else {
            if (fhbool.accessed[0])
                fh.accessed[0] = fhopt.accessed[0];
            if (fhbool.accessed[1])
                fh.accessed[1] = fhopt.accessed[1];
            if (fhbool.accessed[2])
                fh.accessed[2] = fhopt.accessed[2];
            if (fhbool.accessed[3])
                fh.accessed[3] = fhopt.accessed[3];
        }
        write(fd, &fh, sizeof(struct fileheader));
    }
    return 1;
}

int fh_append()
{
    struct fileheader fh;
    lseek(fd, 0, SEEK_END);
    if (binary) {
        while (fread(&fh, sizeof(struct fileheader), 1, stdin) > 0)
            write(fd, &fh, sizeof(struct fileheader));
    } else
        write(fd, &fhopt, sizeof(struct fileheader));
    return 1;
}

int fh_insert(int num, int count)
{
    struct fileheader fh;
    off_t filesize;
    int total, rest, i;
    char *ptr;
    struct stat st;
    if (num <= 0) {
        printf("number must be specified.\n");
        exit(0);
    }
    if (!binary)
        count = 1;
    fstat(fd, &st);
    ftruncate(fd, st.st_size + count * sizeof(struct fileheader));
    BBS_TRY {
        if (safe_mmapfile_handle(fd, PROT_READ | PROT_WRITE, MAP_SHARED, &ptr, &filesize) == 0) {
            BBS_RETURN(0);
        }
        total = filesize / sizeof(struct fileheader);
        rest = total - num + 1;
        if (num > total)
            BBS_RETURN(0);
        memmove(ptr + (num - 1 + count) * sizeof(struct fileheader), ptr + (num - 1) * sizeof(struct fileheader), rest * sizeof(struct fileheader));
    }
    BBS_CATCH {
    }
    BBS_END;
    munmap(ptr, filesize);
    lseek(fd, (num - 1) * sizeof(struct fileheader), SEEK_SET);
    if (binary) {
        for (i=0; i<count; i++) {
            if (fread(&fh, sizeof(struct fileheader), 1, stdin) <= 0)
                break;
            write(fd, &fh, sizeof(struct fileheader));
        }
    } else {
        write(fd, &fhopt, sizeof(struct fileheader));
    }
    return 1;
}

int fh_remove(int num, int count)
{
    off_t filesize;
    int total = 0, rest;
    char *ptr;
    if (num <= 0) {
        printf("number must be specified.\n");
        exit(0);
    }
    BBS_TRY {
        if (safe_mmapfile_handle(fd, PROT_READ | PROT_WRITE, MAP_SHARED, &ptr, &filesize) == 0) {
            BBS_RETURN(0);
        }
        total = filesize / sizeof(struct fileheader);
        if (num > total)
            BBS_RETURN(0);
        if (num + count - 1 > total)
            count = total - num + 1;
        rest = total - num - count + 1;
        memmove(ptr + (num - 1) * sizeof(struct fileheader), ptr + (num + count - 1) * sizeof(struct fileheader), rest * sizeof(struct fileheader));
    }
    BBS_CATCH {
    }
    BBS_END;
    munmap(ptr, filesize);
    ftruncate(fd, (total - count) * sizeof(struct fileheader));
    return 1;
}

int fh_recovertitle(char* bname, int num, int count)
{
    off_t filesize;
    int total, i, j;
    char *ptr, tmptitle[ARTICLE_TITLE_LEN], *ch, fname[PATHLEN], buf[1024];
    struct fileheader *pfh;
    FILE *fp;
    if (num <= 0) {
        printf("number must be specified.\n");
        exit(0);
    }
    BBS_TRY {
        if (safe_mmapfile_handle(fd, PROT_READ | PROT_WRITE, MAP_SHARED, &ptr, &filesize) == 0) {
            BBS_RETURN(0);
        }
        total = filesize / sizeof(struct fileheader);
        pfh = (struct fileheader *)ptr;
        for (i=num-1; i<num+count-1; i++) {
            if (i >= total)
                break;
            strcpy(tmptitle, pfh[i].title);
            if ((ch = strrchr(tmptitle, '-')) != NULL) {
                *ch = 0;
                for (j=strlen(tmptitle)-1; j>=0; j--) {
                    if (tmptitle[j] != ' ')
                        break;
                    else
                        tmptitle[j] = 0;
                }
            }
            setbfile(fname, bname, pfh[i].filename);
            fp = fopen(fname, "r");
            if (!fp)
                continue;
            j = 0;
            while ((!feof(fp)) && (j < 2)) {
                skip_attach_fgets(buf, 1024, fp);
                if (feof(fp))
                    break;
                if (strstr(buf, "发信人: ") && strstr(buf, "), 信区: ")) {
                    j++;
                } else if (strstr(buf, "标  题: ")) {
                    j++;
                    strncpy(tmptitle, buf + 8, sizeof(tmptitle));
                    tmptitle[sizeof(tmptitle)-1] = '\0';
                    if ((ch = strchr(tmptitle, '\n')) != NULL)
                        *ch = 0;
                }
            }
            fclose(fp);
            strcpy(pfh[i].title, tmptitle);
        }
    }
    BBS_CATCH {
    }
    BBS_END;
    munmap(ptr, filesize);
    return 1;
}

int openbdir(char* dirfile, int flags, int locktype)
{
    struct flock ldata;
    fd = open(dirfile, flags);
    ldata.l_type = locktype;
    ldata.l_whence = 0;
    ldata.l_start = 0;
    ldata.l_len = 0;
    if (fcntl(fd, F_SETLKW, &ldata) == -1) {
        printf("error in locking file.\n");
        exit(0);
    }
    return 1;
}

int closebdir()
{
    struct flock ldata;
    ldata.l_type = F_UNLCK;
    ldata.l_whence = 0;
    ldata.l_start = 0;
    ldata.l_len = 0;
    fcntl(fd, F_SETLKW, &ldata);
    close(fd);
    return 1;
}

int main(int argc, char* argv[])
{
    static struct option opts[] = {
        {"help", no_argument, &operate, 0},
        {"binary", no_argument, &binary, 1},
        {"simple", no_argument, &simple, 1},
        {"size", no_argument, &operate, 1},
        {"list", no_argument, &operate, 2},
        {"view", no_argument, &operate, 3},
        {"modify", no_argument, &operate, 4},
        {"append", no_argument, &operate, 5},
        {"insert", no_argument, &operate, 6},
        {"remove", no_argument, &operate, 7},
        {"recovertitle", no_argument, &operate, 8},
        {"board", required_argument, 0, 'b'},
        {"user", required_argument, 0, 'u'},
        {"indexfile", required_argument, 0, 'e'},
        {"number", required_argument, 0, 'n'},
        {"count", required_argument, 0, 'c'},
        {"filename", required_argument, 0, 'f'},
        {"id", required_argument, 0, 'i'},
        {"groupid", required_argument, 0, 'g'},
        {"reid", required_argument, 0, 'r'},
        {"o_bid", required_argument, 0, 'B'},
        {"o_id", required_argument, 0, 'I'},
        {"o_groupid", required_argument, 0, 'G'},
        {"o_reid", required_argument, 0, 'R'},
        {"innflag", required_argument, 0, 'N'},
        {"owner", required_argument, 0, 'o'},
        {"effsize", required_argument, 0, 's'},
        {"attachment", required_argument, 0, 'a'},
        {"title", required_argument, 0, 't'},
        {"accessed0", required_argument, 0, '0'},
        {"accessed1", required_argument, 0, '1'},
        {"accessed2", required_argument, 0, '2'},
        {"accessed3", required_argument, 0, '3'},
        {"accessed9", no_argument, &accessed9, 1},
        {0, 0, 0, 0}
    };
    int c, iopt;

    char board[STRLEN] = "", userid[IDLEN+1] = "";
    int num = 0;
    int count = 1;
    char dirfile[PATHLEN];

    fhbool.filename[0] = 0;
    fhbool.id = 0;
    fhbool.groupid = 0;
    fhbool.reid = 0;
    fhbool.o_bid = 0;
    fhbool.o_id = 0;
    fhbool.o_groupid = 0;
    fhbool.o_reid = 0;
    fhbool.innflag[0] = 0;
    fhbool.owner[0] = 0;
    fhbool.eff_size = 0;
    fhbool.posttime = 0;
    fhbool.attachment = 0;
    fhbool.title[0] = 0;
    fhbool.accessed[0] = 0;
    fhbool.accessed[1] = 0;
    fhbool.accessed[2] = 0;
    fhbool.accessed[3] = 0;
    memcpy(&fhopt, &fhbool, sizeof(struct fileheader));

    while ((c = getopt_long(argc, argv, "", opts, &iopt)) != -1) {
        if (opts[iopt].flag != 0)
            continue;
        switch (c) {
            case 'b':
                strncpy(board, optarg, STRLEN - 1);
                break;
            case 'u':
                strncpy(userid, optarg, IDLEN);
                break;
            case 'e':
                strncpy(indexfile, optarg, STRLEN - 1);
                break;
            case 'n':
                num = atoi(optarg);
                break;
            case 'c':
                count = atoi(optarg);
                break;
            case 'f':
                strncpy(fhopt.filename, optarg, FILENAME_LEN);
                fhbool.filename[0] = 1;
                break;
            case 'i':
                fhopt.id = atoi(optarg);
                fhbool.id = 1;
                break;
            case 'g':
                fhopt.groupid = atoi(optarg);
                fhbool.groupid = 1;
                break;
            case 'r':
                fhopt.reid = atoi(optarg);
                fhbool.reid = 1;
                break;
            case 'B':
                fhopt.o_bid = atoi(optarg);
                fhbool.o_bid = 1;
                break;
            case 'I':
                fhopt.o_id = atoi(optarg);
                fhbool.o_id = 1;
                break;
            case 'G':
                fhopt.o_groupid = atoi(optarg);
                fhbool.o_groupid = 1;
                break;
            case 'R':
                fhopt.o_reid = atoi(optarg);
                fhbool.o_reid = 1;
                break;
            case 'N':
                strncpy(fhopt.innflag, optarg, 2);
                fhbool.innflag[0] = 1;
                break;
            case 'o':
                strncpy(fhopt.owner, optarg, OWNER_LEN);
                fhbool.owner[0] = 1;
                break;
            case 's':
                fhopt.eff_size = atoi(optarg);
                fhbool.eff_size = 1;
                break;
            case 'a':
                fhopt.attachment = atoi(optarg);
                fhbool.attachment = 1;
                break;
            case 't':
                strncpy(fhopt.title, optarg, ARTICLE_TITLE_LEN);
                fhbool.title[0] = 1;
                break;
            case '0':
                fhopt.accessed[0] = (unsigned char)strtol(optarg, NULL, 0);
                fhbool.accessed[0] = 1;
                break;
            case '1':
                fhopt.accessed[1] = (unsigned char)strtol(optarg, NULL, 0);
                fhbool.accessed[1] = 1;
                break;
            case '2':
                fhopt.accessed[2] = (unsigned char)strtol(optarg, NULL, 0);
                fhbool.accessed[2] = 1;
                break;
            case '3':
                fhopt.accessed[3] = (unsigned char)strtol(optarg, NULL, 0);
                fhbool.accessed[3] = 1;
                break;
        }
    }

    if (operate == 0) {
        printf("Usage: %s [OPTION]\n", argv[0]);
        printf("\nOptions:\n");
        printf("  --binary          read binary data from stdin or write data to stdout.\n");
        printf("  --simple          display only numbers when listing.\n");
        printf("\nOperations:\n");
        printf("  --size            display article numbers, this only show total article\n");
        printf("                    numbers, ignoring specified properties.\n");
        printf("  --list            display brief list of articles.\n");
        printf("  --view            display detail information of one article.\n");
        printf("  --modify          substitute records under binary mode, or change\n");
        printf("                    properties of an article under non-binary mode.\n");
        printf("  --append          append records to board under binary mode, or append\n");
        printf("                    one new record of article under non-binary mode.\n");
        printf("  --insert          insert records to board under binary mode, or insert\n");
        printf("                    one new record of article under non-binary mode.\n");
        printf("  --remove          remove records from board, only doing range delete,\n");
        printf("                    ignoring specified properties.\n");
        printf("  --recovertitle    recover the original article title as undo-delete.\n");
        printf("\nParameters:\n");
        printf("  --board           specify the board name in order to modify board dir.\n");
        printf("  --user            specify user id in order to modify mail dir.\n");
        printf("  --indexfile       specify index file, default is .DIR\n");
        printf("  --number          specify the starting article number.\n");
        printf("  --count           specify the number of articles to deal with, default\n");
        printf("                    is 1.\n");
        printf("  --filename        specify filename to search for or modify to.\n");
        printf("  --id              specify id to search for or modify to.\n");
        printf("  --groupid         specify groupid to search for or modify to.\n");
        printf("  --reid            specify reid to search for or modify to.\n");
        printf("  --o_bid           specify o_bid to search for or modify to.\n");
        printf("  --o_id            specify o_id to search for or modify to.\n");
        printf("  --o_groupid       specify o_groupid to search for or modify to.\n");
        printf("  --o_reid          specify o_reid to search for or modify to.\n");
        printf("  --innflag         specify innflag to search for or modify to.\n");
        printf("  --owner           specify owner to search for or modify to.\n");
        printf("  --effsize         specify eff_size to search for or modify to.\n");
        printf("  --attachment      specify attachment to search for or modify to.\n");
        printf("  --title           specify title to search for or modify to.\n");
        printf("  --accessedN       specify accessed to search for or modify to, N shall\n");
        printf("                    be replaced by a number between 0 and 3.\n");
        printf("  --accessed9       use Nine-Skies-Calculation-Ruler to modify accessed.\n");
        return 1;
    }

    if (board[0])
        sprintf(dirfile, "boards/%s/%s", board, indexfile);
    else if (userid[0]) {
        if (isalpha(userid[0]))
            sprintf(dirfile, "mail/%c/%s/%s", toupper(userid[0]), userid, indexfile);
        else {
            printf("invalid user id.\n");
            return 0;
        }
    } else {
        printf("no index file is given.\n");
        return 0;
    }

    chdir(BBSHOME);
    init_all();

    switch (operate) {
        case 1:
            fh_count(dirfile);
            break;
        case 2:
            openbdir(dirfile, O_RDONLY, F_RDLCK);
            fh_list(num, count);
            closebdir();
            break;
        case 3:
            openbdir(dirfile, O_RDONLY, F_RDLCK);
            fh_view(num);
            closebdir();
            break;
        case 4:
            openbdir(dirfile, O_RDWR, F_WRLCK);
            fh_modify(num);
            closebdir();
            break;
        case 5:
            openbdir(dirfile, O_WRONLY | O_CREAT, F_WRLCK);
            fh_append();
            closebdir();
            break;
        case 6:
            openbdir(dirfile, O_RDWR | O_CREAT, F_WRLCK);
            fh_insert(num, count);
            closebdir();
            break;
        case 7:
            openbdir(dirfile, O_RDWR, F_WRLCK);
            fh_remove(num, count);
            closebdir();
            break;
        case 8:
            openbdir(dirfile, O_RDWR, F_WRLCK);
            fh_recovertitle(board, num, count);
            closebdir();
    }

    return 1;
}


