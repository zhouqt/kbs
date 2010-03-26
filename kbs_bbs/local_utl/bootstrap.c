#include <pwd.h>
#include "bbs.h"

/* 版面属性，见doc/README.SYSOP 1.2 */
enum boardtype {
    nil,
    V,
    X,                  /* 写限制为PERM_SYSOP */
    o,                  /* 读限制为PERM_BASIC */
};

typedef struct board_s {
    enum boardtype type;
    char *name;
} board_t;

board_t boards[] = {
    {V, "bbsnet"},
    {V, "denypost"},
    {V, "Filter"},
    {V, "GiveupNotice"},
    {V, "Goodbye"},
    {V, "Recommend"},
    {V, "Registry"},
    {V, "reject_registry"},
    {V, "syssecurity"},
    {V, "undenypost"},
    {V, "sysmail"},
    {X, "BBSLists"},
    {X, "newcomers"},
    {X, "notepad"},
    {X, "vote"},
    {o, "Blessing"},
    {nil, ""},
};

void usage(char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    fprintf(stdout, "\n");
    fflush(stdout);
    va_end(args);
}

void error(char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    fprintf(stderr, "error: ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
    exit(EXIT_FAILURE);
}

void clean_ipc(void)
{
    char cmd[PATHLEN];
    struct passwd *pw;

    system("killall bbslogd");
    system(BBSHOME "/bin/miscd flush");
    system("killall miscd");

    /* 清除共享内存 */
    pw = getpwuid(BBSUID);
    if (pw) {
        sprintf(cmd, "ipcs -m|awk '$0~/%s/{system(sprintf(\"ipcrm -m %%s\",$2));}'", pw->pw_name);
        system(cmd);
        sprintf(cmd, "ipcs -q|awk '$0~/%s/{system(sprintf(\"ipcrm -q %%s\",$2));}'", pw->pw_name);
        system(cmd);
    }
}

/* 清空.PASSWDS文件和.BOARDS文件 */
void clean_ids_boards(void)
{
    char path[PATHLEN];
    int fd;

    strcpy(path, BBSHOME "/.PASSWDS");
    if ((fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1)
        error("clean the .PASSWDS error");
    close(fd);
    usage("file .PASSWDS is cleaned");
    strcpy(path, BBSHOME "/.BOARDS");
    if ((fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1)
        error("clean the .BOARDS error");
    close(fd);
    usage("file .BOARDS is cleaned");
}

/* 创建以username为名，以passwd为密码的用户 */
void add_bbsid(char *username, char *passwd)
{
    struct userec *newuser;
    int unum;
    char path[PATHLEN];
    char buf[PATHLEN];

    unum = getnewuserid(username);
    if (unum > MAXUSERS || unum <= 0) {
        clean_ipc();
        error("%s: unum error", username);
    }
    strcpy(buf, username);
    unum = getuser(buf, &newuser);
    setpasswd(passwd, newuser);

    if (!strcmp(username, "SYSOP")) {
        newuser->flags = (char)1;
        strcpy(newuser->username, "系统管理员");
        newuser->userdefine[0] = (unsigned int)3221224447U;
        newuser->userdefine[1] = (unsigned int)4294967295U;
        newuser->userlevel = (unsigned int)0xDAFFDFFF;
    } else if (!strcmp(username, "guest")) {
        newuser->flags = (char)9;
        newuser->userlevel = (unsigned int)536879104U;
        strcpy(newuser->username, "和谐");
        newuser->userdefine[0] = (unsigned int)3221224447U;
        newuser->userdefine[1] = (unsigned int)4294967295U;
    }

    update_user(newuser, unum, 1);

    newbbslog(BBSLOG_USIES, "APPLY: %s uid %d from %s",
            newuser->userid, unum, "127.0.0.1");
    bbslog("user", "%s", "new account");
    usage("add user %s", username);

    sethomepath(path, newuser->userid);
    sprintf(buf, "/bin/mv -f %s %s/homeback/%s", path, BBSHOME, newuser->userid);
    system(buf);
    sprintf(buf, "/bin/mv -f %s %s/mailback/%s", path, BBSHOME, newuser->userid);
    system(buf);
    if (strcmp(username, "guest"))      /* 不给 guest 发信 */
        mail_file(DELIVER, "etc/tonewuser", newuser->userid, "致新注册用户的信", 0, NULL);
}

int wrapped_mkdir(const char *path, mode_t mode)
{
    int ret = mkdir(path, mode);

    if (ret == -1 && errno == EEXIST)
        return 0;
    return ret;
}

void add_bbsboard(const board_t *pbt)
{
    struct boardheader newboard;
    char vbuf[PATHLEN], buf[PATHLEN];
    int bid;

    if (!pbt)
        return;
    memset(&newboard, 0, sizeof(newboard));
    strcpy(newboard.filename, pbt->name);
    strcpy(newboard.title, "0[待定]      版面中文名称待定");
    if (getbid(newboard.filename, NULL) > 0) {
        clean_ipc();
        error("%s: getbid error", newboard.filename);
    }
    strcpy(vbuf, "vote/");
    strcat(vbuf, newboard.filename);
    setbpath(buf, newboard.filename);
    /*
     * keep old files.
     * f_rm(buf);
     * f_rm(vbuf);
     */
    if (wrapped_mkdir(buf, 0755) == -1 || wrapped_mkdir(vbuf, 0755) == -1) {
        clean_ipc();
        error("%s: mkdir error", newboard.filename);
    }
    snprintf(newboard.ann_path, 127, "%s/%s", groups[0], newboard.filename);
    newboard.ann_path[127] = '\0';

    /* 默认权限 */
    newboard.level = PERM_SYSOP;

    if (pbt->type == X)
        newboard.level |= PERM_POSTMASK;
    else if (pbt->type == o)
        newboard.level = 0;

    if (add_board(&newboard) == -1) {
        clean_ipc();
        error("%s: add_board error", newboard.filename);
    }
    if ((bid = getbid(newboard.filename, NULL)) == 0) {
        clean_ipc();
        error("%s: getbid error", newboard.filename);
    }

    sprintf(buf, "add board %s", newboard.filename);
    bbslog("user", "%s", buf);
    usage(buf);
}

void add_allboards(void)
{
    board_t *pbt = boards;
    char buf1[PATHLEN], buf2[PATHLEN];

    for (; pbt && pbt->type != nil; ++pbt)
        add_bbsboard(pbt);

    sprintf(buf1, "%s/boards/sysmail", BBSHOME);
    sprintf(buf2, "%s/mail/S/SYSOP", BBSHOME);
    remove(buf1);
    symlink(buf2, buf1);
}

void bootstrap(void)
{
    int ret;
    char *prompt[] = {
        "Input SYSOP's passwd: ",
        "Input SYSOP's passwd again: ",
    };
    char *pw[2];        /* store SYSOP's password */

    /*
     * About function getpass():
     * lots of codes still use the function, so we use it,
     * though it is obsolete.
     * XXX: memory leak here is harmless.
     */
    while (1) {
        pw[0] = strdup(getpass(prompt[0]));
        if (strlen(pw[0]) > PASSLEN) {
            fprintf(stderr, "password too long, reinput\n");
            continue;
        }
        if (strlen(pw[0]) < 4) {
            fprintf(stderr, "password too short, reinput\n");
            continue;
        }
        if (!strcmp(pw[0], "SYSOP")) {
            fprintf(stderr, "password same as BBS ID, reinput\n");
            continue;
        }
        pw[1] = strdup(getpass(prompt[1]));
        if (strcmp(pw[0], pw[1])) {
            fprintf(stderr, "passwords don't match, reinput\n");
            continue;
        }
        break;
    }

    clean_ipc();
    clean_ids_boards();

    if ((ret = system(BBSHOME "/bin/miscd daemon")) != 0)
        error("error running miscd, return value = %d", ret);
    if ((ret = system(BBSHOME "/bin/bbslogd")) != 0)
        error("error running bbslogd, return value = %d", ret);

    chdir(BBSHOME);
    resolve_boards();
    resolve_utmp();
    load_ucache();

    add_bbsid("SYSOP", pw[0]);
    add_bbsid("guest", "anypassword");
    add_allboards();

    clean_ipc();
}

int main(int argc, char *argv[])
{
    uid_t uid = geteuid();
    char buf[STRLEN];
    size_t len;

    if (uid != BBSUID)
        if (setgid(BBSGID) == -1 || setuid(BBSUID) == -1)
            error("Please run %s as BBSUSER or root.", argv[0]);

    if (system("pgrep bbsd > /dev/null") == 0 ||
            system("pgrep sshbbsd > /dev/null") == 0 ||
            system("pgrep miscd > /dev/null") == 0
       )
            // sshdbbsd or bbsd or miscd is running
        error("Please shutdown BBS first.");

    printf("Danger! This may destroy ALL OF YOUR USER AND BOARD DATA!\n");
    printf("Type `Go ahead!' exactly without quotes and press ENTER to continue\n");
    printf("Are you ready? ");
    fgets(buf, STRLEN, stdin);
    len = strlen(buf);
    if (buf[len-1] == '\n' || buf[len-1] == '\r')
        buf[len-1] = '\0';
    if (!strcmp("Go ahead!", buf))
        bootstrap();
    else
        printf("User aborted.\n");
    return 0;
}
