#include "bbs.h"
#define BLOCKFILE   "/bbs/.blockmail"

/* copy from flyriver qmail2bbs.c */
void my_ansi_filter(char *source)
{
    char result[500];
    int i, flag = 0, loc = 0;
    int len;

    len = strlen(source);
    len = len >= sizeof(result) ? sizeof(result) - 1 : len;
    for (i = 0; i < len; i++) {
        if (source[i] == '\x1B') {
            flag = 1;
            continue;
        } else if (flag == 1 && isalpha(source[i])) {
            flag = 0;
            continue;
        } else if (flag == 1) {
            continue;
        } else {
            result[loc++] = source[i];
        }
    }
    result[loc] = '\0';
    strncpy(source, result, loc + 1);
}

#ifdef MAIL2BOARD
void strmov(char *source, int offset)
{
    int i;
    int len;

    if (source == NULL)
        return;
    if (offset <= 0)
        return;
    len = strlen(source);
    if (len <= offset)
        source[0] = '\0';
    else {
        for (i = offset; i < len; i++)
            source[i - offset] = source[i];
        source[i - offset] = '\0';
    }
}

void eat_forward(char *source)
{
    char *ptr;
    char *strfwd[] = {
        "[转寄]",
        "(转寄)",
        NULL
    };
    int i;

    for (i = 0; strfwd[i] != NULL; i++) {
        ptr = strstr(source, strfwd[i]);
        if (ptr == source) {
            if (source[6] == ' ') {
                strmov(source, 7);
                return;
            } else {
                strmov(source, 6);
                return;
            }
        } else if (ptr != NULL) {
            *ptr = '\0';
            return;
        }
    }
}

int my_after_post(struct fileheader *fh, char *boardname)
{
    char buf[256];
    int fd, err = 0, nowid = 0;
    char *p;

    if (!strncmp(fh->title, "Re:", 3)) {
        strncpy(fh->title, fh->title + 4, STRLEN);
    }
    setbfile(buf, boardname, DOT_DIR);

    if ((fd = open(buf, O_WRONLY | O_CREAT, 0664)) == -1) {
        err = 1;
    }

    if (!err) {
        flock(fd, LOCK_EX);
        nowid = get_nextid(boardname);
        fh->id = nowid;
        fh->groupid = fh->id;
        fh->reid = fh->id;
        set_posttime(fh);
        lseek(fd, 0, SEEK_END);
        if (safewrite(fd, fh, sizeof(fileheader)) == -1) {
            err = 1;
        }
        flock(fd, LOCK_UN);
        close(fd);
    }
    if (err) {
        setbfile(buf, boardname, fh->filename);
        unlink(buf);
        return 1;
    }
    updatelastpost(boardname);

    if (fh->id == fh->groupid)
        setboardorigin(boardname, 1);
    setboardtitle(boardname, 1);
    return 0;
}

int append_board(fin, sender1, sender, bname, title, received)
FILE *fin;
char *bname, *sender1, *sender, *title, *received;
{
    time_t now;
    FILE *fout;
    struct fileheader newmessage;
    char fname[512], buf[256], boardpath[256], conv_buf[256];
    struct boardheader *brd;
    char *user, *userptr;
    char *lesssym, *nameptrleft, *nameptrright;
    char author[STRLEN];

/* check if the board is in our bbs now */
    if ((brd = getbcache(bname)) == NULL)
        return -1;
    strcpy(bname, brd->filename);

/* check for the dir for the board */
    setbpath(boardpath, bname);
    printf("OK, board dir is %s\n", boardpath);

    str_decode(conv_buf, title);
/* copy from flyriver qmailpost.c */
    my_ansi_filter(conv_buf);
    if (conv_buf[0] == '\0')
        strcpy(conv_buf, "无标题");
    eat_forward(conv_buf);

/* allocate a record for the new mail */
    bzero(&newmessage, sizeof(newmessage));
    GET_POSTFILENAME(fname, boardpath);
    strcpy(newmessage.filename, fname);
    setbfile(buf, bname, fname);
    strcpy(fname, buf);
    if (!dashd(boardpath))
        return -1;
    strncpy(newmessage.title, conv_buf, sizeof(newmessage.title) - 1);
    newmessage.title[sizeof(newmessage.title) - 1] = '\0';

/* parse the owner address */
    if (strchr(sender, '<') && (sender[strlen(sender) - 1] == '>'))
        user = (char *) strrchr(sender, '@');
    else
        user = (char *) strchr(sender, '@');
    lesssym = (char *) strchr(sender, '<');
    nameptrleft = NULL, nameptrright = NULL;
    if (lesssym == NULL || lesssym >= user) {
        lesssym = sender;
        nameptrleft = strchr(sender, '(');
        if (nameptrleft != NULL)
            nameptrleft++;
        nameptrright = strrchr(sender, ')');
    } else {
        nameptrleft = sender;
        nameptrright = strrchr(sender, '<');
        lesssym++;
    }
    if (user != NULL) {
        *user = '\0';
        userptr = (char *) strchr(sender, '.');
        if (userptr != NULL) {
            *userptr = '\0';
            strncpy(author, lesssym, sizeof(author));
            *userptr = '.';
        } else
            strncpy(author, lesssym, sizeof(author));
        *user = '@';
    } else
        strncpy(author, lesssym, sizeof(author));
    if (!isalnum(author[0]))
        strcpy(author, "Unknown");
    strcat(author, ".");

    strncpy(newmessage.owner, author, sizeof(newmessage.owner) - 1);
    newmessage.owner[sizeof(newmessage.owner) - 1] = '\0';
    newmessage.innflag[0] = 'L';
    newmessage.innflag[1] = 'L';
    printf("OK, the file is %s\n", fname);

/* copy the stdin to the specified file */
    if ((fout = fopen(fname, "w")) == NULL) {
        printf("Cannot open %s\n", fname);
        return -1;
    }
    now = time(NULL);
    fprintf(fout, "发信人: %s (%s), 信区: %s\n", newmessage.owner, sender, bname);
    fprintf(fout, "标  题: %s\n", conv_buf);
    fprintf(fout, "发信站: %s (%24.24s), 站内\n", BBS_FULL_NAME, ctime(&now));
    strcat(received, "\n");
    fprintf(fout, "来  源: %s\n", received);
    while (fgets(buf, 255, fin) != NULL) {
        fputs(buf, fout);
    }
    fclose(fout);

    return my_after_post(&newmessage, bname);
}
#endif

append_mail(fin, sender1, sender, userid, title, received)
FILE *fin;
char *userid, *sender1, *sender, *title, *received;
{
    struct fileheader newmessage;
    char fname[512], buf[256], genbuf[256], fff[80], fff2[80];
    char maildir[256];
    struct stat st;
    FILE *fout, *dp, *rmail;
    int yyyy, zzzz, passcheck = 0;
    char conv_buf[256];
    char *ptr;
    struct userec *user;

/* check if the userid is in our bbs now */
    if (getuser(userid, &user) == 0)
        return -1;

/* check for the mail dir for the userid */
    setmailpath(maildir, user->userid);
    if (stat(maildir, &st) == -1) {
        if (mkdir(maildir, 0755) == -1)
            return -1;
    } else {
        if (!(st.st_mode & S_IFDIR))
            return -1;
    }
    printf("Ok, dir is %s\n", maildir);

    str_decode(conv_buf, title);
/* copy from flyriver qmail2bbs.c */
    my_ansi_filter(conv_buf);
    if (conv_buf[0] == '\0')
        strcpy(conv_buf, "无标题");

/* check for mail register */
    if (!strcmp(userid, "SYSOP") && strstr(conv_buf, " mail check.")) {
        passcheck = 1;
        if ((!strstr(sender, "bbs")) && (strchr(conv_buf, '@'))) {
            yyyy = 0;
            zzzz = 0;
            while ((conv_buf[yyyy] != '@') && (yyyy < NAMELEN))
                yyyy = yyyy + 1;
            yyyy = yyyy + 1;
            while ((conv_buf[yyyy] != '@') && (yyyy < NAMELEN)) {
                sender1[zzzz] = conv_buf[yyyy];
                yyyy = yyyy + 1;
                zzzz = zzzz + 1;
            }
            sender1[zzzz] = '\0';
            strcpy(userid, sender1);
            sprintf(fff, "%s/home/%c/%s/mailcheck", BBSHOME, toupper(sender1[0]), sender1);
            if ((dp = fopen(fff, "r")) != NULL) {
                printf("open mailcheck\n");
                fgets(fff2, sizeof(fff2), dp);
                fclose(dp);
                sprintf(fff2, "%9.9s", fff2);
                if (getuser(sender1, NULL) && strstr(conv_buf, fff2)) {
                    printf("pass1\n");
                    unlink(fff);
                    passcheck = 5;
                    sprintf(genbuf, "%s", sender);
                    sprintf(buf, "%s/home/%c/%s/register", BBSHOME, toupper(sender1[0]), sender1);
                    if (dashf(buf)) {
                        sprintf(buf, "%s/home/%c/%s/register.old", BBSHOME, toupper(sender1[0]), sender1);
                        rename(buf, conv_buf);
                    }
                    if ((fout = fopen(buf, "w")) != NULL) {
                        fprintf(fout, "%s\n", genbuf);
                        fclose(fout);
                    }
                }
            }
        }
    }

/* allocate a record for the new mail */
    bzero(&newmessage, sizeof(newmessage));
    GET_MAILFILENAME(fname, maildir);
    strcpy(newmessage.filename, fname);
    setmailfile(buf, user->userid, fname);
    strcpy(fname, buf);
    strncpy(newmessage.title, conv_buf, sizeof(newmessage.title) - 1);
    newmessage.title[sizeof(newmessage.title) - 1] = '\0';
    ptr = strchr(sender, '@');
    if (ptr == NULL || ptr == sender)
        return -1;
    strncpy(newmessage.owner, sender, sizeof(newmessage.owner) - 1);
    newmessage.owner[sizeof(newmessage.owner) - 1] = '\0';
    printf("Ok, the file is %s\n", fname);

/* copy the stdin to the specified file */
    if ((fout = fopen(fname, "w")) == NULL) {
        printf("Cannot open %s \n", fname);
        return -1;
    } else {
        time_t tmp_time;

        time(&tmp_time);
        fprintf(fout, "寄信人: %-.70s \n", sender);
        fprintf(fout, "标  题: %-.70s\n", conv_buf);
        fprintf(fout, "发信站: %s 信差\n", BBS_FULL_NAME);
        if (received != NULL && received[0] != '\0')
            fprintf(fout, "来  源: %-.70s\n", received);
        fprintf(fout, "日  期: %s\n", ctime(&tmp_time));
        if (passcheck >= 1) {
            fprintf(fout, "亲爱的 %s:\n", sender1);
            sprintf(maildir, "%s/etc/%s", BBSHOME, (passcheck == 5) ? "smail" : "fmail");
            if ((rmail = fopen(maildir, "r")) != NULL) {
                while (fgets(genbuf, 255, rmail) != NULL)
                    fputs(genbuf, fout);
                fclose(rmail);
            }
        } else {
            while (fgets(genbuf, 255, fin) != NULL)
                fputs(genbuf, fout);
        }
        fclose(fout);
    }

/* append the record to the MAIL control file */
    if (chkusermail(user) == 0) {
        setmailfile(buf, user->userid, DOT_DIR);
        if (append_record(buf, &newmessage, sizeof(newmessage)) == 0) {
            struct stat fs;

            stat(fname, &fs);
            update_user_usedspace(fs.st_size, user);
            return 0;
        }
    }
    unlink(fname);
    return -1;
}

int block_mail(addr)
char *addr;
{
    FILE *fp;
    char temp[STRLEN];

    if ((fp = fopen(BLOCKFILE, "r")) != NULL) {
        while (fgets(temp, STRLEN, fp) != NULL) {
            strtok(temp, "\n");
            if (strstr(addr, temp)) {
                fclose(fp);
                return 1;
            }
        }
        fclose(fp);
    }
    return 0;
}

main(argc, argv)
int argc;
char *argv[];
{

    char sender[256];
    char username[256];
    char receiver[256];
    char nettyp[256];
    int xxxx;

#ifdef MAIL2BOARD
    char *ptr;
    int mail2board = 0;
#endif

    strcpy(receiver, argv[2]);
#ifdef MAIL2BOARD
    ptr = strchr(receiver, '.');
    if (ptr == NULL) {
        mail2board = 0;
    } else if (!strncasecmp(ptr + 1, "board", 5)) {
        *ptr = '\0';
        mail2board = 1;
    } else
        return 1;
#endif

/* argv[ 1 ] is original sender */
/* argv[ 2 ] is userid in bbs   */
/* argv[ 3 ] is the mail title  */
/* argv[ 4 ] is the message-id  */
    if (argc != 5) {
        char *p = (char *) rindex(argv[0], '/');

        printf("Usage: %s sender receiver_in_bbs mail_title\n", p ? p + 1 : argv[0]);
        return 1;
    }
    if (chroot(BBSHOME) == 0) {
        chdir("/");
#ifdef DEBUG
        printf("Chroot ok!\n");
#endif
    } else {
/* assume it is in chrooted in bbs */
/* if it is not the case, append_main() will handle it */
        chdir(BBSHOME);
#ifdef DEBUG
        printf("Already chroot\n");
#endif
    }

    setreuid(BBSUID, BBSUID);
    setregid(BBSGID, BBSGID);
#ifdef MAIL2BOARD
    if (mail2board == 1)
        resolve_boards();
    else
#endif
        resolve_ucache();

    if (argv[1] == NULL || strlen(argv[1]) == 0) {
        fprintf(stderr, "Error: Unknown sender\n");
        return -2;
    }
    if (strchr(argv[1], '@')) {
        strncpy(sender, argv[1], sizeof(sender) - 1);
/* added by netty */
        xxxx = 0;
        while (sender[xxxx] != '@') {
            nettyp[xxxx] = sender[xxxx];
            xxxx = xxxx + 1;
        }
        nettyp[xxxx] = '\0';    /* added by netty */
    } else {
        char *p, *l, *r;
        char buf[256];

        strcpy(buf, argv[1]);
        p = strtok(buf, " \t\n\r");
        l = strchr(argv[1], '(');
        r = strchr(argv[1], ')');
        if (l < r && l && r)
            strncpy(username, l, r - l + 1);
        sprintf(sender, "%s@%s %s", p, MAIL_BBSDOMAIN, username);
        strcpy(nettyp, p);
    }

    if (block_mail(sender))
        return -2;

#ifdef MAIL2BOARD
    if (mail2board == 1) {
        return append_board(stdin, nettyp, sender, receiver, argv[3], argv[4]);
    } else {
#endif
        return append_mail(stdin, nettyp, sender, receiver, argv[3], argv[4]);
#ifdef MAIL2BOARD
    }
#endif
}
