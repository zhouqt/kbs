/*
 * $Id$
 */
#include "types.h"
#include "bbslib.h"
#include <assert.h>

/*
time_t update_time = 0;
int showexplain = 0, freshmode = 0;
int mailmode, numf;
int usercounter, real_user_names = 0;
int page, readplan, num;
*/

time_t set_idle_time(struct user_info * uentp, time_t t);

struct user_info *u_info;

char fromhost[IPLEN + 1];
char parm_name[256][80], *parm_val[256];
int parm_num = 0;

/*
int badnum = 0;
int loginok = 0;
friends_t bbb[MAXREJECTS];
struct UTMPFILE *shm_utmp;
struct UCACHE *shm_ucache;
extern struct favbrd_struct favbrd_list[FAVBOARDNUM];
extern int favbrd_list_t;
extern int favnow;
friends_t fff[200];
*/
        
int f_append(char *file, char *buf)
{
    FILE *fp;
    char *ptr;

    fp = fopen(file, "a");
    if (fp == 0)
        return -1;
    ptr = buf;
    while (*ptr != '\0') {
        if (*ptr == '\x09')     /* horizontal tab */
            fprintf(fp, "    ");
        else
            fputc(*ptr, fp);
        ptr++;
    }
    fclose(fp);

    return 0;
}

time_t file_time(char *file)
{
    struct stat st;

    if (stat(file, &st) == -1)
        return 0;
    return st.st_mtime;
}

struct stat *f_stat(char *file)
{
    static struct stat buf;

    bzero(&buf, sizeof(buf));
    if (stat(file, &buf) == -1)
        bzero(&buf, sizeof(buf));
    return &buf;
}

char *wwwCTime(time_t t)
{
    static char s[80];

    sprintf(s, "%24.24s", ctime(&t));
    return s;
}

char *nohtml(char *s)
{
    char *buf = calloc(strlen(s) + 1, 1);
    int i = 0, mode = 0;

    while (s[0] && i < 1023) {
        if (mode == 0) {
            if (s[0] == '<') {
                mode = 1;
            } else {
                buf[i] = s[0];
                i++;
            }
        } else {
            if (s[0] == '>')
                mode = 0;
        }
        s++;
    }
    buf[i] = 0;
    return buf;
}

int strsncpy(char *s1, char *s2, int n)
{
    int l = strlen(s2);

    if (n < 0)
        return;
    if (n > l + 1)
        n = l + 1;
    strncpy(s1, s2, n - 1);
    s1[n - 1] = 0;
}

char *ltrim(char *s)
{
    char *ptr;
    char *ptr2;

    if (s[0] == '\0')
        return s;
    ptr = ptr2 = s;
    while (*ptr != '\0') {
        if ((*ptr != ' ') && (*ptr != '\t') && (*ptr != '\r')
            && (*ptr != '\n')) {
            break;
        }
        ptr++;
    }
    if (ptr == s)
        return s;
    if (*ptr == '\0') {
        s[0] = '\0';
        return s;
    }
    while (*ptr != '\0') {
        *ptr2 = *ptr;
        ptr++;
        ptr2++;
    }
    *ptr2 = '\0';

    return s;
}

char *rtrim(char *s)
{
    char *ptr;
    char *ptr2;

    if (s[0] == '\0')
        return s;
    ptr = s;
    ptr2 = s + strlen(s) - 1;
    while (ptr2 != ptr) {
        if ((*ptr2 == ' ') || (*ptr2 == '\t') || (*ptr2 == '\r')
            || (*ptr2 == '\n')) {
            *ptr2 = '\0';
            ptr2--;
            continue;
        }
        break;
    }

    return s;
}

char *getsenv(char *s)
{
    char *t = getenv(s);

    if (t)
        return t;
    return "";
}

int http_quit()
{
    printf("\n</html>\n");
    exit(0);
}

int http_fatal(char *fmt, ...)
{
    char buf[1024];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(buf, 1023, fmt, ap);
    va_end(ap);
    buf[1023] = 0;
    printf("错误! %s! <br><br>\n", buf);
    printf("<a href=\"javascript:history.go(-1)\">快速返回</a>");
    http_quit();
}

int strnncpy(char *s, int *l, char *s2)
{
    strncpy(s + (*l), s2, strlen(s2));
    (*l) += strlen(s2);
}

int hsprintf(char *s, char *fmt, ...)
{
    char buf[1024], ansibuf[80], buf2[80];
    char *tmp;
    int c, bold, m, i, l, len;
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(buf, 1023, fmt, ap);
    va_end(ap);
    s[0] = 0;
    l = strlen(buf);
    len = 0;
    bold = 0;
    for (i = 0; i < l; i++) {
        c = buf[i];
        if (c == '&') {
            strnncpy(s, &len, "&amp;");
        } else if (c == '<') {
            strnncpy(s, &len, "&lt;");
        } else if (c == '>') {
            strnncpy(s, &len, "&gt;");
        } else if (c == 27) {
            if (buf[i + 1] != '[')
                continue;
            for (m = i + 2; m < l && m < i + 24; m++)
                if (strchr("0123456789;", buf[m]) == 0)
                    break;
            strsncpy(ansibuf, &buf[i + 2], m - (i + 2) + 1);
            i = m;
            if (buf[i] != 'm')
                continue;
            if (strlen(ansibuf) == 0) {
                bold = 0;
                strnncpy(s, &len, "<font class=\"c37\">");
            }
            tmp = strtok(ansibuf, ";");
            while (tmp) {
                c = atoi(tmp);
                tmp = strtok(0, ";");
                if (c == 0) {
                    strnncpy(s, &len, "<font class=\"c37\">");
                    bold = 0;
                }
                if (c >= 30 && c <= 37) {
                    if (bold == 1)
                        sprintf(buf2, "<font class=\"d%d\">", c);
                    if (bold == 0)
                        sprintf(buf2, "<font class=\"c%d\">", c);
                    strnncpy(s, &len, buf2);
                }
            }
        } else {
            s[len] = c;
            len++;
        }
    }
    s[len] = 0;
}

int hprintf(char *fmt, ...)
{
    char buf[8096], buf2[1024];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(buf2, 1023, fmt, ap);
    va_end(ap);
    hsprintf(buf, "%s", buf2);
    printf("%s", buf);
}

int hhprintf(char *fmt, ...)
{
    char buf0[1024], buf[1024], *s, *getparm();
    int len = 0;
    int my_link_mode;
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(buf, 1023, fmt, ap);
    va_end(ap);
    buf[1023] = 0;
    s = buf;
    my_link_mode = atoi(getparm("my_link_mode"));
    if (my_link_mode == 1)
        return hprintf("%s", buf);
    if (!strcasestr(s, "http://") && !strcasestr(s, "ftp://") && !strcasestr(s, "mailto:"))
        return hprintf("%s", buf);
    while (s[0]) {
        if (!strncasecmp(s, "http://", 7) || !strncasecmp(s, "mailto:", 7) || !strncasecmp(s, "ftp://", 6)) {
            char *tmp;

            if (len > 0) {
                buf0[len] = 0;
                hprintf("%s", buf0);
                len = 0;
            }
            tmp = strtok(s, "\'\" \r\t)(,;\n");
            if (tmp == 0)
                break;
            /*
             * if(1) {
             * if(strstr(tmp, ".gif") || strstr(tmp, ".jpg") || strstr(tmp, ".bmp")) {
             * printf("<img src=\"%s\">", nohtml(tmp));
             * tmp=strtok(0, "");
             * if(tmp==0) return -1;
             * return hhprintf("%s",tmp);
             * }
             * } 
             */
            printf("<a target=\"_blank\" href=\"%s\">%s</a>", nohtml(tmp), nohtml(tmp));
            tmp = strtok(0, "");
            if (tmp == 0)
                return printf("\n");
            return hhprintf("%s", tmp);
        } else {
            buf0[len] = s[0];
            if (len < 1000)
                len++;
            s++;
        }
    }
    return 0;
}

int parm_add(char *name, char *val)
{
    int len = strlen(val);

    if (parm_num >= 255)
        http_fatal("too many parms.");
    parm_val[parm_num] = calloc(len + 1, 1);
    if (parm_val[parm_num] == 0)
        http_fatal("memory overflow2 %d %d", len, parm_num);
    strsncpy(parm_name[parm_num], name, 78);
    strsncpy(parm_val[parm_num], val, len + 1);
    parm_num++;
}

void html_init()
{
    int style;

    printf("Content-type: text/html; charset=%s\n\n\n", CHARSET);
    printf("<html>\n");
    printf("<head>\n");
    printf("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=%s\">\n", CHARSET);
    style = atoi(getparm("STYLE"));
    switch (style) {
    case 0:                    /* default color, big font */
        printf("<link rel=\"stylesheet\" type=\"text/css\" href=\"%s\">\n", CSS_FILE_BIGFONT);
        printf("<link rel=\"stylesheet\" type=\"text/css\" href=\"/ansi-web-middle.css\">\n");
        break;
    case 1:
    default:                   /* default color, small font */
        printf("<link rel=\"stylesheet\" type=\"text/css\" href=\"%s\">\n", CSS_FILE);
        printf("<link rel=\"stylesheet\" type=\"text/css\" href=\"/ansi-web-small.css\">\n");
    }
    printf("</head>");
}

int http_init()
{
    char *buf, buf2[1024], *t2, *t3;
    int n;

    n = atoi(getsenv("CONTENT_LENGTH"));
    if (n > 5000000)
        n = 5000000;
    buf = calloc(n + 1, 1);
    if (buf == 0)
        http_fatal("memory overflow");
    fread(buf, 1, n, stdin);
    buf[n] = 0;
    t2 = strtok(buf, "&");
    while (t2) {
        t3 = strchr(t2, '=');
        if (t3 != 0) {
            t3[0] = 0;
            t3++;
            __unhcode(t3);
            __unhcode(trim(t2));
            parm_add(t2, t3);
        }
        t2 = strtok(0, "&");
    }
    strsncpy(buf2, getsenv("QUERY_STRING"), 1024);
    t2 = strtok(buf2, "&");
    while (t2) {
        t3 = strchr(t2, '=');
        if (t3 != 0) {
            t3[0] = 0;
            t3++;
            __unhcode(t3);
            __unhcode(trim(t2));
            parm_add(t2, t3);
        }
        t2 = strtok(0, "&");
    }
    strsncpy(buf2, getsenv("HTTP_COOKIE"), 1024);
    /*
     * printf("HTTP_COOKIE = %s\n", buf2); 
     */
    t2 = strtok(buf2, ";");
    while (t2) {
        t3 = strchr(t2, '=');
        if (t3 != 0) {
            t3[0] = 0;
            t3++;
            parm_add(trim(t2), t3);
        }
        t2 = strtok(0, ";");
    }
/*#ifdef SQUID_ACCL*/
    {
        char *ptr, *p;

        ptr = getsenv("HTTP_X_FORWARDED_FOR");
        p = strrchr(ptr, ',');
        if (p != NULL) {
            while (!isdigit(*p) && *p)
                p++;
            if (*p)
                strncpy(fromhost, p, IPLEN);
            else
                strncpy(fromhost, ptr, IPLEN);
        } else
            strncpy(fromhost, ptr, IPLEN);
        fromhost[IPLEN] = 0;
    }
    if (fromhost[0] == '\0')
/*#endif*/
        strsncpy(fromhost, getsenv("REMOTE_ADDR"), IPLEN);
}

int __to16(char c)
{
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    if (c >= '0' && c <= '9')
        return c - '0';
    return 0;
}

int __unhcode(char *s)
{
    int m, n;

    for (m = 0, n = 0; s[m] != 0; m++, n++) {
        if (s[m] == '+') {
            s[n] = ' ';
            continue;
        }
        if (s[m] == '%') {
            s[n] = __to16(s[m + 1]) * 16 + __to16(s[m + 2]);
            m += 2;
            continue;
        }
        s[n] = s[m];
    }
    s[n] = 0;
}

char *getparm(char *var)
{
    int n;

    for (n = 0; n < parm_num; n++)
        if (!strcasecmp(parm_name[n], var))
            return parm_val[n];
    return "";
}

/* smh_init() 需要修改 */
int shm_init()
{
    resolve_ucache();
    resolve_utmp();
    resolve_boards();
    www_data_init();
}

int user_init(struct userec **x, struct user_info **y)
{
    char id[20], num[20];
    int key;

    strsncpy(id, getparm("UTMPUSERID"), 13);
    strsncpy(num, getparm("UTMPNUM"), 12);
    /*
     * printf("utmpuserid = %s\n", id); 
     */
    /*
     * printf("utmpnum = %s\n", num); 
     */
    key = atoi(getparm("UTMPKEY"));
    getSession()->utmpent = atoi(num);
    if (id[0] == '\0')
        return -1;

    if (www_user_init(getSession()->utmpent, id, key, x, y, 0) == 0 && strcasecmp("guest", (*x)->userid))
        return 1;
    return 0;
}

int del_mail(int ent, struct fileheader *fh, char *direct)
{
    char buf[PATHLEN];
    char *t;
    char genbuf[PATHLEN];
    struct stat st;

    if (strstr(direct, ".DELETED") || HAS_MAILBOX_PROP(u_info, MBP_FORCEDELETEMAIL)) {
        strcpy(buf, direct);
        t = strrchr(buf, '/') + 1;
        strcpy(t, fh->filename);
		if (lstat(buf, &st) == 0 && S_ISREG(st.st_mode) && st.st_nlink == 1)
            getCurrentUser()->usedspace -= st.st_size;
    }

    strcpy(buf, direct);
    if ((t = strrchr(buf, '/')) != NULL)
        *t = '\0';
    if (!delete_record(direct, sizeof(*fh), ent, (RECORD_FUNC_ARG) cmpname, fh->filename)) {
        sprintf(genbuf, "%s/%s", buf, fh->filename);
        if (strstr(direct, ".DELETED") || HAS_MAILBOX_PROP(u_info, MBP_FORCEDELETEMAIL))
            unlink(genbuf);
        else {
            strcpy(buf, direct);
            t = strrchr(buf, '/') + 1;
            strcpy(t, ".DELETED");
            append_record(buf, fh, sizeof(*fh));
        }
        return 0;
    }
    return 1;
}

int post_mail(char *userid, char *title, char *file, char *id, char *nickname, char *ip, int sig, int backup)
/* @parm backup - whether save this mail to sent box - atppp */
{
    FILE *fp, *fp2;
    char buf3[256];
    char fname[STRLEN], filepath[STRLEN], sent_filepath[STRLEN];
    struct fileheader header;
    struct stat st;
    struct userec *touser;      /*peregrine for updating used space */
    int unum;
    
    if (false == canIsend2(getCurrentUser(), userid)) {
        return -2;
    }

    unum = getuser(userid, &touser);
    if (!HAS_PERM(getCurrentUser(), PERM_SYSOP) && chkusermail(touser)) {    /*Haohamru.99.4.05 */
        return -3;
    }

    bzero(&header, sizeof(header));
    strcpy(header.owner, id);
    strncpy(header.title, title, ARTICLE_TITLE_LEN - 1);
	header.title[ARTICLE_TITLE_LEN - 1] = '\0';
    setmailpath(filepath, userid);
    if (stat(filepath, &st) == -1) {
        if (mkdir(filepath, 0755) == -1)
            return -1;
    } else {
        if (!(st.st_mode & S_IFDIR))
            return -1;
    }
    if (GET_MAILFILENAME(fname, filepath) < 0)
        return -1;
    strcpy(header.filename, fname);
    setmailfile(filepath, userid, fname);

    fp = fopen(filepath, "w");
    if (fp == NULL)
        return -1;
    fp2 = fopen(file, "r");
    fprintf(fp, "寄信人: %s (%s)\n", id, nickname);
    fprintf(fp, "标  题: %s\n", title);
    fprintf(fp, "发信站: %s (%s)\n", BBSNAME, wwwCTime(time(0)));
    fprintf(fp, "来  源: %s\n\n", ip);
    if (fp2) {
        while (fgets(buf3, 256, fp2) != NULL) {
            fprintf2(fp, buf3);
        }
        fclose(fp2);
    }
    fprintf(fp, "\n--\n");
    sig_append(fp, id, sig);
    fprintf(fp, "\n\033[1;%dm※ 来源:．%s %s．[FROM: %s]\033[m\n", 31 + rand() % 7, BBSNAME, NAME_BBS_ENGLISH, SHOW_USERIP(getCurrentUser(), ip));
    fclose(fp);
    
    if (stat(filepath, &st) != -1)
        header.eff_size = st.st_size;
    setmailfile(buf3, userid, ".DIR");
    if (append_record(buf3, &header, sizeof(header)) == -1)
        return -5;
    touser->usedspace += header.eff_size;
	setmailcheck(userid);
	    
   /* 添加Log Bigman: 2003.4.7 */
    newbbslog(BBSLOG_USER, "mailed(www) %s %s", userid,title);

    if (backup) {
        strcpy(header.owner, userid);
        setmailpath(sent_filepath, id);
        if (GET_MAILFILENAME(fname, sent_filepath) < 0) return -6;
        strcpy(header.filename, fname);
        setmailfile(sent_filepath, id, fname);

        f_cp(filepath, sent_filepath, 0);
        if (stat(sent_filepath, &st) != -1) {
            getCurrentUser()->usedspace += st.st_size;
            header.eff_size = st.st_size;
        } else {
            return -6;
        }
        header.accessed[0] |= FILE_READ;
        setmailfile(buf3, id, ".SENT");
        if (append_record(buf3, &header, sizeof(header)) == -1)
            return -6;
        newbbslog(BBSLOG_USER, "mailed(www) %s ", id);
    }
    return 0;
}

int outgo_post2(struct fileheader *fh, char *board, char *userid, char *username, char *title)
{
    FILE *foo;

    if (foo = fopen("innd/out.bntp", "a")) {
        fprintf(foo, "%s\t%s\t%s\t%s\t%s\n", board, fh->filename, userid, username, title);
        fclose(foo);
    }
}

void add_loginfo2(FILE * fp, char *board, struct userec *user, int anony)
{
    FILE *fp2;
    int color;
    char fname[STRLEN];

    color = (user->numlogins % 7) + 31; /* 颜色随机变化 */
    sethomefile(fname, getCurrentUser()->userid, "signatures");
    if ((fp2 = fopen(fname, "r")) == NULL ||    /* 判断是否已经 存在 签名档 */
        user->signature == 0 || anony == 1) {
        fputs("\n--\n", fp);
    } else {                    /*Bigman 2000.8.10修改,减少代码 */
        fprintf(fp, "\n");
    }
    /*
     * 由Bigman增加:2000.8.10 Announce版匿名发文问题 
     */
    if (!strcmp(board, "Announce"))
        fprintf(fp, "\033[m\033[%2dm※ 来源:·%s http://%s·[FROM: %s]\033[m\n", color, BBS_FULL_NAME, BBS_FULL_NAME, NAME_BBS_CHINESE " BBS站");
    else
        fprintf(fp, "\n\033[m\033[%2dm※ 来源:·%s http://%s·[FROM: %s]\033[m\n", color, BBS_FULL_NAME, NAME_BBS_ENGLISH, (anony) ? NAME_ANONYMOUS_FROM : SHOW_USERIP(getCurrentUser(), user->lasthost));

    if (fp2)
        fclose(fp2);
    return;
}

void write_header2(FILE * fp, char *board, char *title, char *userid, char *username, int anony, int local)
{
    if (!strcmp(board, "Announce"))
        fprintf(fp, "发信人: %s (%s), 信区: %s\n", "SYSOP", NAME_SYSOP, board);
    else
        fprintf(fp, "发信人: %s (%s), 信区: %s\n", anony ? board : userid, anony ? NAME_ANONYMOUS : username, board);
    if (local == 1)
        fprintf(fp, "标  题: %s\n发信站: %s (%24.24s), 站内\n\n", title, "BBS " NAME_BBS_CHINESE "站", Ctime(time(0)));
    else
        fprintf(fp, "标  题: %s\n发信站: %s (%24.24s), 转信\n\n", title, "BBS " NAME_BBS_CHINESE "站", Ctime(time(0)));
}

/* fp 		for destfile*/
/* fp2		for srcfile*/
int write_file2(FILE * fp, FILE * fp2)
{
    char buf3[1024];
    char *ptr;

    while (1) {
        if (fgets(buf3, sizeof(buf3), fp2) == NULL)
            break;
		buf3[1023]='\0';
        ptr = strrchr(buf3, '\r');
        if (ptr != NULL) {
            if (*(ptr + 1) == '\n') {
                *ptr = '\n';
                *(ptr + 1) = '\0';
            }
        }
//        fprintf2(fp, buf3);
		fputs(buf3,fp);
    }
}

/* return value:
   >0		success
   -1		write .DIR failed*/
int post_article(char *board, char *title, char *file, struct userec *user, char *ip, int sig, int local_save, int anony, struct fileheader *oldx, char *attach_dir, int mailback, int is_tex)
{
    struct fileheader post_file;
    char filepath[MAXPATH];
    char buf[256];
    int fd, anonyboard;
	int retvalue;
    FILE *fp, *fp2;

    memset(&post_file, 0, sizeof(post_file));
    anonyboard = anonymousboard(board); /* 是否为匿名版 */

    /*
     * 自动生成 POST 文件名 
     */
    setbfile(filepath, board, "");
    if (GET_POSTFILENAME(post_file.filename, filepath) != 0) {
        return -1;
    }
    setbfile(filepath, board, post_file.filename);

    anony = anonyboard && anony;
    strncpy(post_file.owner, anony ? board : getcurruserid(), OWNER_LEN);
    post_file.owner[OWNER_LEN - 1] = 0;

    if ((!strcmp(board, "Announce")) && (!strcmp(post_file.owner, board)))
        strcpy(post_file.owner, "SYSOP");
    fp = fopen(filepath, "w");
    fp2 = fopen(file, "r");
#ifndef RAW_ARTICLE
    write_header2(fp, board, title, user->userid, user->username, anony, local_save);
#endif
    write_file2(fp, fp2);
    fclose(fp2);
    if (!anony)
        addsignature(fp, user, sig);
#ifndef RAW_ARTICLE
    add_loginfo2(fp, board, user, anony);       /*添加最后一行 */
#endif

    strncpy(post_file.title, title, ARTICLE_TITLE_LEN - 1);
	post_file.title[ARTICLE_TITLE_LEN - 1] = '\0';
    if (local_save == 1) {      /* local save */
        post_file.innflag[1] = 'L';
        post_file.innflag[0] = 'L';
    } else {
        post_file.innflag[1] = 'S';
        post_file.innflag[0] = 'S';
        outgo_post2(&post_file, board, user->userid, user->username, title);
    }

    if (mailback) post_file.accessed[1] |= FILE_MAILBACK;
    if (is_tex) post_file.accessed[1] |= FILE_TEX;
    
    setbfile(buf, board, DOT_DIR);

    /*
     * 在boards版版主发文自动添加文章标记 Bigman:2000.8.12 
     */
    if (!strcmp(board, "Board") && !HAS_PERM(getCurrentUser(), PERM_OBOARDS)
        && HAS_PERM(getCurrentUser(), PERM_BOARDS)) {
        post_file.accessed[0] |= FILE_SIGN;
    }

	if (attach_dir != NULL) {
		char tmp[STRLEN];
    	struct stat st;
        snprintf(tmp, MAXPATH, "%s/.index", attach_dir);
    	if (stat(tmp, &st) >= 0 && st.st_size > 0)
			post_file.attachment = 1;
	}
    fclose(fp);
    post_file.eff_size = get_effsize(filepath);
	retvalue = after_post(getCurrentUser(), &post_file, board, oldx, !(anonyboard && anony), getSession());

    if (attach_dir != NULL) {

#ifdef FILTER
		if(retvalue == 2)
    		setbfile(filepath, FILTER_BOARD, post_file.filename);
#endif
	  if( (fp = fopen(filepath, "a")) != NULL){
        snprintf(filepath, MAXPATH, "%s/.index", attach_dir);
        if ((fp2 = fopen(filepath, "r")) != NULL) {
            fputs("\n", fp);
            while (!feof(fp2)) {
                char *name;
                long begin, save_size;
                char *ptr;
                off_t size;

                fgets(buf, 256, fp2);
                name = strchr(buf, ' ');
                if (name == NULL)
                    continue;
                *name = 0;
                name++;
                ptr = strchr(name, '\n');
                if (ptr)
                    *ptr = 0;

                if (-1 == (fd = open(buf, O_RDONLY)))
                    continue;
                if (post_file.attachment == 0) {
                    /*
                     * log the attachment begin 
                     */
                    post_file.attachment = ftell(fp) + 1;
                }
                fwrite(ATTACHMENT_PAD, ATTACHMENT_SIZE, 1, fp);
                fwrite(name, strlen(name) + 1, 1, fp);
                BBS_TRY {
                    if (safe_mmapfile_handle(fd,  PROT_READ, MAP_SHARED, (void **) &ptr, & size) == 0) {
                        size = 0;
                        save_size = htonl(size);
                        fwrite(&save_size, sizeof(save_size), 1, fp);
                    } else {
                        save_size = htonl(size);
                        fwrite(&save_size, sizeof(save_size), 1, fp);
                        begin = ftell(fp);
                        fwrite(ptr, size, 1, fp);
                    }
                }
                BBS_CATCH {
                    ftruncate(fileno(fp), begin + size);
                    fseek(fp, begin + size, SEEK_SET);
                }
                BBS_END end_mmapfile((void *) ptr, size, -1);

                close(fd);
            }
			fclose(fp2);
        }
	  fclose(fp);
	  }
    }
    if (retvalue == 0) {
#ifdef WWW_GENERATE_STATIC
        generate_static(DIR_MODE_NORMAL,&post_file,board,oldx);
#endif
    }

    return post_file.id;
}

int sig_append(FILE * fp, char *id, int sig)
{
    FILE *fp2;
    char path[256];
    char buf[256];
    int i = 0, skip_lines = 0;
    struct userec *x = NULL;

    if (sig <= 0 || sig > MAX_SIGNATURES)
        return -1;
    getuser(id, &x);
    if (x == 0)
        return -1;
    sethomefile(path, x->userid, "signatures");
    fp2 = fopen(path, "r");
    if (fp2 == NULL)
        return -1;
    /*
     * 跳过前面的 (sig - 1)*6 行  
     */
    do {
        if (skip_lines == (sig - 1) * 6)
            break;
        skip_lines++;
    } while ((fgets(buf, sizeof(buf), fp2)) != NULL);
    /*
     * 检查是否可以读入第 sig 个签名档 
     */
    if (skip_lines == (sig - 1) * 6) {
        /*
         * 读入签名档并写入 fp 对应的文件中 
         */
        for (i = skip_lines; i < skip_lines + 6; i++) {
            if (fgets(buf, sizeof(buf), fp2) == NULL)
                break;
            unix_string(buf);
            if (buf[0] != '\n')
                fprintf(fp, "%s", buf);
        }
        /*
         * 读入签名档成功，设置默认签名档为当前使用的签名档 
         * 改成由 phpbbslib 来设置这个，因为随机签名档在这里判断不出来 - atppp
         */
        /*
        if (i > skip_lines)
            x->signature = sig;
        */
    }
    fclose(fp2);
    if (i > skip_lines)
        return sig;
    return 0;
}

int has_BM_perm(struct userec *user, char *board)
{
    bcache_t *x;
    char buf[256], *bm;

    x = getbcache(board);
    if (x == 0)
        return 0;
    if (user_perm(user, PERM_BLEVELS) || user_perm(user,PERM_SYSOP))
        return 1;
    if (!user_perm(user, PERM_BOARDS))
        return 0;
    strcpy(buf, x->BM);
    bm = strtok(buf, ",: ;&()\n");
    while (bm) {
        if (!strcasecmp(bm, user->userid))
            return 1;
        bm = strtok(0, ",: ;&()\n");
    }
    return 0;
}

int send_msg(char *srcid, int srcutmp, char *destid, int destpid, char *msg)
{
    int i;
    uinfo_t *uin;

    /*
     * 滤掉特殊字符，应该写成一个函数 
     */
    for (i = 0; i < (int) strlen(msg); i++)
        if ((0 < msg[i] && msg[i] <= 27) || msg[i] == -1)
            msg[i] = 32;
    uin = t_search(destid, destpid);
    if (uin == NULL)
        return -1;
    if (strcasecmp(uin->userid, destid))
        return -1;
    strcpy(getSession()->MsgDesUid, uin->userid);
    return sendmsgfunc(uin, msg, 2, getSession());
}

int user_perm(struct userec *x, int level)
{
    return (x->userlevel & level);
}

int getusernum(char *id)
{
    return searchuser(id);
}

int isfriend(char *id)
{
    return myfriend(searchuser(id), NULL, getSession());
}

void http_redirect(char *url)
{
    printf("Status: 302 Found\n");
    printf("Location: %s\n", url);
    printf("Content-type: text/html; charset=%s\n\n", CHARSET);
}

int initwww_all() //这个函数应该是没用了 - atppp
{
	struct userec * user;
        int loginok = 0;
    srand(time(0) * 2 + getpid());
    chdir(BBSHOME);
    init_sessiondata(getSession());
    http_init();
    /*
     * seteuid(BBSUID);
     * if(geteuid()!=BBSUID) http_fatal("uid error."); 
     */
    shm_init();
    loginok = user_init(&user, &u_info);
	setCurrentUser(user);
    if (loginok < 0) {
        /*
         * http_redirect(NOLOGIN_PAGE);
         * exit(0);
         */
        printf("Content-type: text/html; charset=%s\n\n", CHARSET);
        /*
         * redirect(NOLOGIN_PAGE); 
         */
        printf("<script>top.window.location='/nologin.html';</script>\n");
        exit(0);
    }
    html_init();
    init_bbslog();
}

char *void1(unsigned char *s)
{
    int i;
    int flag = 0;

    for (i = 0; s[i]; i++) {
        if (flag == 0) {
            if (s[i] >= 128)
                flag = 1;
            continue;
        }
        flag = 0;
        if (s[i] < 32)
            s[i - 1] = 32;
    }
    if (flag)
        s[strlen(s) - 1] = 0;
    return s;
}

char *userid_str(char *s)
{
    static char buf[512];
    char buf2[256], tmp[256], *ptr, *ptr2;

    strsncpy(tmp, s, 255);
    buf[0] = 0;
    ptr = strtok(tmp, " ,();\r\n\t");
    while (ptr && strlen(buf) < 400) {
        if (ptr2 = strchr(ptr, '.')) {
            ptr2[1] = 0;
            strcat(buf, ptr);
            strcat(buf, " ");
        } else {
            ptr = nohtml(ptr);
            sprintf(buf2, "<a href=\"/bbsqry.php?userid=%s\">%s</a> ", ptr, ptr);
            strcat(buf, buf2);
        }
        ptr = strtok(0, " ,();\r\n\t");
    }
    return buf;
}

int fprintf2(FILE * fp, char *s)
{
    int i, tail = 0, sum = 0;

    if (s[0] == ':' && s[1] == ' ' && strlen(s) > 79) {
        sprintf(s + 76, "..\n");
        fprintf(fp, "%s", s);
        return;
    }
    for (i = 0; s[i]; i++) {
        fprintf(fp, "%c", s[i]);
        sum++;
        if (tail) {
            tail = 0;
        } else if (s[i] < 0) {
            tail = s[i];
        }
        if (sum >= 78 && tail == 0) {
            fprintf(fp, "\n");
            sum = 0;
        }
    }
}

int get_file_ent(char *board, char *file, struct fileheader *x)
{
    char dir[80];

    sprintf(dir, "boards/%s/.DIR", board);
    return search_record(dir, x, sizeof(struct fileheader), (RECORD_FUNC_ARG)cmpname, file);
}

/* added by flyriver, 2001.12.17
 * using getcurrusr() instead of using getCurrentUser() directly
 */
struct userec *getcurrusr()
{
    return getCurrentUser();
}

char *getcurruserid()
{
    return getCurrentUser()->userid;
}

time_t get_idle_time(struct user_info * uentp)
{
    return uentp->freshtime;
}


bcache_t *getbcacheaddr()
{
    return bcache;
}

uinfo_t *setcurruinfo(uinfo_t * ui)
{
    u_info = ui;

    return u_info;
}

int count_online()
{
    return get_utmp_number();
}

typedef struct _frienduserlistarg{
    int count;
    uinfo_t** user_record;
} frienduserlistarg;

int full_utmp_friend(struct user_info *uentp, frienduserlistarg *pful)
{
    if (!uentp->active || !uentp->pid) {
        return 0;
    }
    if (!HAS_PERM(getCurrentUser(), PERM_SEECLOAK) && uentp->invisible && strcmp(uentp->userid, getcurruserid())) {  /*Haohmaru.99.4.24.让隐身者能看见自己 */
        return 0;
    }
    if (!myfriend(uentp->uid, NULL, getSession())) {
        return 0;
    }
    if (pful->count < MAXFRIENDS) {
        pful->user_record[pful->count] = uentp;
        pful->count++;
    }
    return COUNT;
}

int fill_friendlist(int* range, uinfo_t** user_record)
{
    int i;
    frienduserlistarg ful;
    struct user_info *u;

    ful.count = 0;
    ful.user_record = user_record;
    u = get_utmpent(getSession()->utmpent);
    for (i = 0; i < u->friendsnum; i++) {
        if (u->friends_uid[i])
            apply_utmpuid((APPLY_UTMP_FUNC) full_utmp_friend, u->friends_uid[i], &ful);
    }
    *range = ful.count;
    return ful.count == 0 ? -1 : 1;
}

int add_favboard(char *brdname)
{
    int i;

    if (brdname != NULL && *brdname)
        i = getbnum(brdname);
    else
        return -3;              /* err brdname */
    i--;
    addFavBoard(i, getSession());
}

static int printstatusstr(struct user_info *uentp, char *arg, int pos)
{
    char modebuf[80];
    if (uentp->invisible == 1) {
		arg[0] = '1';
        if (!HAS_PERM(getCurrentUser(), PERM_SEECLOAK))
            return COUNT;
    }
    if (arg[1]==0)
        strcat(arg, "目前在站上，状态如下：\n");
    if (uentp->invisible)
        strcat(arg, "\033[32m隐身中   \033[m");
    else {
        char buf[80];

        sprintf(buf, "\033[1m%s\033[m ", modestring(modebuf,uentp->mode, uentp->destuid, 0, 
                                              (uentp->in_chat ? uentp->chatid : NULL)));
        strcat(arg, buf);
    }
    UNUSED_ARG(pos);
    return COUNT;
}

/* stiger: 获得一个用户的在线状态string */
int get_userstatusstr(char *userid, char *buf)
{
	int tuid=0;
	struct userec *lookupuser;

	if(!(tuid = getuser(userid, &lookupuser)))
		return 0;

	buf[0]='0';
	buf[1]=0;

	return apply_utmp((APPLY_UTMP_FUNC) printstatusstr, 10, lookupuser->userid, buf);
}

/* 获得当前用户的 utmp 号 */
int get_curr_utmpent()
{
    return get_utmpent_num(u_info);
}


/* 以下的代码是cgi和php都使用的*/
static struct user_info www_guest_uinfo;

static int www_guest_calc_hashkey(struct in_addr *fromhostn)
{
	unsigned int i=ntohl(fromhostn->s_addr);
	unsigned int j;
			        
    j =  i & 0x0000FFFF;
    j |= (((i&0xFF000000)>>8) + (i&0x00FF0000)) & 0x000F0000;

    return j;
}

static int www_guest_start_map(int key)
{
	return ( key % MAX_WWW_MAP_ITEM + 1 );
}

#define WWW_GUEST_HASHTAB(key) wwwguest_shm->hashtab[key>>16][(key&0x0000FF00)>>8][key&0x000000FF]

/*   stiger:  1 guest entry per IP
return:
	<0: error
	0: 正常登录,idx
	1: 有重复,使用idx的entry
**************/
static int www_new_guest_entry(struct in_addr *fromhostn, int * idx)
{
    struct public_data *pub;
    int oldidx, num, fd, i, j;
    time_t now;
    struct userec *user;
	int hashkey;
	int startkey;

    fd = www_guest_lock();
    if (fd == -1)
        return -1;
    setpublicshmreadonly(0);
    pub = get_publicshm();
    if (pub->www_guest_count >= MAX_WWW_GUEST) {
    	www_guest_unlock(fd);
        setpublicshmreadonly(1);
        return -1;
    }
    user = getCurrentUser();
    getuser("guest", &getCurrentUser());

    if (getCurrentUser() == NULL){
    	www_guest_unlock(fd);
    	setpublicshmreadonly(1);
        return -1;
	}
    now = time(NULL);
    if ((now > wwwguest_shm->uptime + 240) || (now < wwwguest_shm->uptime - 240)) {
        newbbslog(BBSLOG_USIES, "WWW guest:Clean guest table:%d", wwwguest_shm->uptime);
        wwwguest_shm->uptime = now;
        for (i = 0; i < MAX_WWW_GUEST; i++) {
	    struct user_info guestinfo;
            if (!(wwwguest_shm->use_map[i / 32] & (1 << (i % 32))) || (now - wwwguest_shm->guest_entry[i].freshtime < MAX_WWW_GUEST_IDLE_TIME))
                continue;
            newbbslog(BBSLOG_USIES, "EXIT: Stay:%3ld (guest)[%d %d](www)", now - wwwguest_shm->guest_entry[i].freshtime, wwwguest_shm->guest_entry[i].key);
            /*
             * 清除use_map 
             */
	    guestinfo.currentboard=wwwguest_shm->guest_entry[i].currentboard;
	    do_after_logout(getCurrentUser(), &guestinfo, i, 1,false);
	    do_after_logout(getCurrentUser(), &guestinfo, i, 1,true);

            wwwguest_shm->use_map[i / 32] &= ~(1 << (i % 32));
			/* 清除hashtab */
			WWW_GUEST_HASHTAB(www_guest_calc_hashkey(& wwwguest_shm->guest_entry[i].fromip)) = 0;
            if (pub->www_guest_count > 0) {
                pub->www_guest_count--;
                /*
                 * 清除数据 
                 */
                bzero(&wwwguest_shm->guest_entry[i], sizeof(struct WWW_GUEST_S));
            }
        } //for
    }//if need kick www guest

	hashkey = www_guest_calc_hashkey(fromhostn);
	oldidx = WWW_GUEST_HASHTAB(hashkey);

/* 如果已经有相同的登陆 */
if( oldidx != 0 && fromhostn->s_addr == wwwguest_shm->guest_entry[oldidx].fromip.s_addr ){

	*idx = oldidx;
	num=-1;
}else{

	startkey = www_guest_start_map(hashkey);

/* 如果hashtab有值但是IP不同，遍历 */
	if( oldidx != 0 ){
		for ( num = 0, i = startkey; num < MAX_WWW_MAP_ITEM; num++, i++){
			if( i>= MAX_WWW_MAP_ITEM)
				i=1;
        	if (wwwguest_shm->use_map[i] != 0) {
            	int map = wwwguest_shm->use_map[i];
            	for (j = 0; j < 32; j++){
                	if ((map & 1) != 0) {
						/* 找到相同的IP了 */
						if( wwwguest_shm->guest_entry[i*32+j].fromip.s_addr == fromhostn->s_addr ){
							num = -1;
							*idx = i*32+j;
                    		break;
						}
                	}
				}
        	}
			if( num == -1 )
				break;
		}
	}

/* 如果遍历发现没有相同IP的 */
	if( num != -1 ){
		/* 找一个新的空位 */
	    for (num=0, i = startkey; num < MAX_WWW_MAP_ITEM; num++, i++){
			if( i>= MAX_WWW_MAP_ITEM)
				i=1;
        	if (wwwguest_shm->use_map[i] != 0xFFFFFFFF) {
            	int map = wwwguest_shm->use_map[i];

            	for (j = 0; j < 32; j++)
                	if ((map & 1) == 0) {
                    	wwwguest_shm->use_map[i] |= 1 << j;
                    	wwwguest_shm->guest_entry[i * 32 + j].freshtime = time(0);
						/* 设置hashtab */
						WWW_GUEST_HASHTAB(hashkey) = i*32+j;
                    	/*
                     	* 避免被kick下去 
                     	*/
                    	break;
                	} else
                    	map = map >> 1;
            	break;
        	}
		}
    	if (num != MAX_WWW_MAP_ITEM) {
        	pub->www_guest_count++;
        	if (get_utmp_number() + getwwwguestcount() > get_publicshm()->max_user) {
            	save_maxuser();
        	}
    	}
		*idx = i*32+j;
	}
}
    setCurrentUser(user);
    setpublicshmreadonly(1);
    www_guest_unlock(fd);
    if (num == MAX_WWW_MAP_ITEM)
        return -1;
	if (num == -1)
		return 1;
    return 0;
}

struct WWW_GUEST_S* www_get_guest_entry(int idx)
{
    return  &wwwguest_shm->guest_entry[idx];
}

static int www_free_guest_entry(int idx)
{
    int fd;
    struct public_data *pub;
    struct user_info guestinfo;

    if ((idx < 0) || (idx > MAX_WWW_GUEST))
        return -1;
    guestinfo.currentboard=wwwguest_shm->guest_entry[idx].currentboard;
    do_after_logout(getCurrentUser(), &guestinfo, idx, 1,false);
    do_after_logout(getCurrentUser(), &guestinfo, idx, 1,true);
    setpublicshmreadonly(0);
    pub = get_publicshm();
    fd = www_guest_lock();
    if (wwwguest_shm->use_map[idx / 32] & (1 << (idx % 32))) {
        wwwguest_shm->use_map[idx / 32] &= ~(1 << (idx % 32));
		WWW_GUEST_HASHTAB(www_guest_calc_hashkey(&wwwguest_shm->guest_entry[idx].fromip))=0;
    	bzero(&wwwguest_shm->guest_entry[idx], sizeof(struct WWW_GUEST_S));
        if (pub->www_guest_count > 0)
            pub->www_guest_count--;
    }
    www_guest_unlock(fd);
    setpublicshmreadonly(1);
    return 0;
}

void www_data_detach()
{
    shmdt(wwwguest_shm);
    wwwguest_shm = NULL;
}

int www_data_init()
{
    struct userec *guest;

    /*
     * www_guest_info目前先使用一个全局变量来做，这样
     * 会导致线程不安全:P   但是对于进程模型的cgi 和php
     * 足够了
     */
    bzero(&www_guest_uinfo, sizeof(www_guest_uinfo));
    www_guest_uinfo.active = true;
    www_guest_uinfo.uid = getuser("guest", &guest);
    if (www_guest_uinfo.uid == 0)
        return -1;
    www_guest_uinfo.invisible = true;
    www_guest_uinfo.pid = 1;
    www_guest_uinfo.mode = WEBEXPLORE;
    strcpy(www_guest_uinfo.username, guest->username);
    strcpy(www_guest_uinfo.userid, guest->userid);
    www_guest_uinfo.pager = 0;
    {
        struct userdata ud;

        read_userdata(guest->userid, &ud);
        strcpy(www_guest_uinfo.realname, ud.realname);
    }
    www_guest_uinfo.utmpkey = 0;

    /*
     * destuid 将被用来存放www guest表的入口 
     */
    www_guest_uinfo.destuid = 0;

    if (resolve_guest_table() != 0)
        return -1;
    return 0;
}

int www_user_init(int useridx, char *userid, int key, struct userec **x, struct user_info **y,long compat_telnet)
{
    /*
     * printf("utmpuserid = %s\n", id);
     */
    /*
     * printf("utmpnum = %s\n", num); 
     */
    if (userid&&!strcasecmp(userid, "new"))
        return -1;

    if ((userid==NULL)||strcasecmp(userid, "guest")) {
        /*
         * 非guest在线用户处理 
         */
        if (useridx < 1 || useridx >= MAXACTIVE) {
            return -1;
        }
        (*y) = get_utmpent(useridx);
        if (                    /*(strncmp((*y)->from, fromhost, IPLEN))|| */
               ((*y)->utmpkey != key))
            return -2;

        if ((((*y)->active == 0)) || ((*y)->userid[0] == 0)
            || ((compat_telnet==0)&&((*y)->mode != WEBEXPLORE)))
            return -3;

        if (userid&&strcmp((*y)->userid, userid))
            return -4;
        getuser((*y)->userid, x);

        if (*x == 0)
            return -5;
#ifdef HAVE_BRC_CONTROL
#if USE_TMPFS==1
	    init_brc_cache((*x)->userid,true,getSession());
#endif
#endif
    } else {
        /*
         * guest用户处理 
         */
        struct WWW_GUEST_S *guest_info;

        if ((useridx < 0) || (useridx >= MAX_WWW_GUEST))
            return -1;
        guest_info = &wwwguest_shm->guest_entry[useridx];
        if (guest_info->key != key) {
            return -2;
        }

        strncpy(www_guest_uinfo.from, fromhost, IPLEN);
        www_guest_uinfo.freshtime = guest_info->freshtime;
        www_guest_uinfo.utmpkey = key;
        www_guest_uinfo.destuid = useridx;
        www_guest_uinfo.logintime = guest_info->logintime;

        *y = &www_guest_uinfo;

        getuser("guest", x);
        if (*x == NULL)
            return -1;
    }
    set_idle_time(*y, time(0));
    return 0;
}

int check_ip_acl(char * id, char * sip)
{
    char fn[160];
    int ip[4],rip[4],l,a;
    unsigned int ips, rips;
    FILE* fp;
    sprintf(fn, BBSHOME "/home/%c/%s/ipacl", toupper(id[0]), id);
    sscanf(sip, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);
    ips = (ip[0]<<24)+(ip[1]<<16)+(ip[2]<<8)+ip[3];
    fp = fopen(fn, "r");
    if(fp) {
        while(!feof(fp)) {
            if(fscanf(fp, "%d.%d.%d.%d %d %d", &rip[0], &rip[1], &rip[2], &rip[3], &l, &a)<=0) break;
            rips = (rip[0]<<24)+(rip[1]<<16)+(rip[2]<<8)+rip[3];
            if(((ips>>(32-l))<<(32-l))==((rips>>(32-l))<<(32-l))||l==0) {
                fclose(fp);
                return a;
            }
        }        
        fclose(fp);
    }
    return 0;
}

int www_user_login(struct userec *user, int useridx, int kick_multi, char *fromhost, char *fullfrom, struct user_info **ppuinfo, int *putmpent)
{
    int ret;
    char buf[255];
    struct userdata ud;

    if (user != NULL && strcasecmp(user->userid, "guest")) {
        struct user_info ui;
        int utmpent;
        time_t t;
        int multi_ret = 1;
		char genbuf[STRLEN];
		struct userec* uc;
 
        while (multi_ret != 0) {
            int lres;
            int num;
            struct user_info uin;

            multi_ret = multilogin_user(user, useridx,1);
            if ((multi_ret != 0) && (!kick_multi))
                return -1;
            if (multi_ret == 0)
                break;
            if (!(num = search_ulist(&uin, cmpuids2, useridx)))
                continue;       /* user isn't logged in */
            if (uin.pid == 1) {
                clear_utmp(num, useridx, uin.pid);
                continue;
            }
            if (!uin.active || (kill(uin.pid, 0) == -1)) {
                clear_utmp(num, useridx, uin.pid);
                continue;       /* stale entry in utmp file */
            }
        /*---	modified by period	first try SIGHUP	2000-11-08	---*/
            lres = kill(uin.pid, SIGHUP);
            sleep(1);
            if (lres)
        /*---	---*/
                kill(uin.pid, 9);
            clear_utmp(num, useridx, uin.pid);
        }

        if (!HAS_PERM(user, PERM_BASIC))
            return 3;
        if (check_ban_IP(fromhost, buf) > 0)
            return 4;
        t = user->lastlogin;
        if (abs(t - time(0)) < 5)
            return 5;
        user->lastlogin = time(0);
        user->numlogins++;
        strncpy(user->lasthost, fromhost, IPLEN);
        user->lasthost[IPLEN - 1] = '\0';       /* add by binxun ,fix the bug */
        read_userdata(user->userid, &ud);
	user->userlevel &= (~PERM_SUICIDE);
        if (!HAS_PERM(user, PERM_LOGINOK) && !HAS_PERM(user, PERM_SYSOP)) {
            if (strchr(ud.realemail, '@')
                && valid_ident(ud.realemail)) {
                user->userlevel |= PERM_DEFAULT;
                /*
                 * if (HAS_PERM(user, PERM_DENYPOST)  )
                 * user->userlevel &= ~PERM_POST;
                 */
            }
#ifdef HAVE_TSINGHUA_INFO_REGISTER
	    else if(strlen(ud.realemail) >=3){
		 strncpy(genbuf,ud.realemail+strlen(ud.realemail)-3,3);
		 if(!strncasecmp(genbuf,"#TH",3))
		 {
			 getuser(user->userid,&uc);
			 if(time(NULL) - uc->firstlogin >= REGISTER_TSINGHUA_WAIT_TIME)
				if(auto_register(user->userid,ud.realemail,STRLEN, getSession())==0)user->userlevel |= PERM_DEFAULT;
		 }
		 read_userdata(user->userid,&ud);
	    }
#endif		  
	    
        }

        memset(&ui, 0, sizeof(struct user_info));
        ui.active = true;
        /*
         * Bigman 2000.8.29 智囊团能够隐身
         */
        if ((HAS_PERM(user, PERM_CHATCLOAK)
             || HAS_PERM(user, PERM_CLOAK))
            && (user->flags & CLOAK_FLAG))
            ui.invisible = true;
        ui.pager = 0;
        if (DEFINE(user, DEF_FRIENDCALL)) {
            ui.pager |= FRIEND_PAGER;
        }
        if (user->flags & PAGER_FLAG) {
            ui.pager |= ALL_PAGER;
            ui.pager |= FRIEND_PAGER;
        }
        if (DEFINE(user, DEF_FRIENDMSG)) {
            ui.pager |= FRIENDMSG_PAGER;
        }
        if (DEFINE(user, DEF_ALLMSG)) {
            ui.pager |= ALLMSG_PAGER;
            ui.pager |= FRIENDMSG_PAGER;
        }
        ui.uid = useridx;
        strncpy(ui.from, fromhost, IPLEN);
        ui.logintime = time(0); /* for counting user's stay time */
        /*
         * refer to bbsfoot.c for details 
         */
        ui.freshtime = time(0);
        ui.mode = WEBEXPLORE;
        
        strncpy(ui.userid, user->userid, 20);
        strncpy(ui.realname, ud.realname, 20);
        strncpy(ui.username, user->username, 40);
#ifdef HAVE_WFORUM
		ui.yank=1;
#endif
        utmpent = getnewutmpent2(&ui);
        if (utmpent == -1)
            ret = 1;
        else {
            struct user_info *u;

            u = get_utmpent(utmpent);
            u->pid = 1;
			/*
            if (addto_msglist(utmpent, user->userid) < 0) {
                bbslog("3system", "can't add msg:%d %s!!!\n", utmpent, user->userid);
                *ppuinfo = u;
                *putmpent = utmpent;
                ret = 2;
            } else {
			*/
                *ppuinfo = u;
                *putmpent = utmpent;
                ret = 0;
				/*
            }
			*/
    	    /* Load getCurrentUser()'s mailbox properties, added by atppp */
    	    u->mailbox_prop = load_mailbox_prop(user->userid);

            getfriendstr(getCurrentUser(), u, getSession());
            do_after_login(getCurrentUser(),utmpent,0);
        }
    } else {
        /*
         * TODO:alloc guest table 
         */
		int idx;
		int exist;
		struct in_addr fromhostn;

#ifdef HAVE_INET_ATON
    inet_aton(fromhost, &fromhostn);
#elif defined HAVE_INET_PTON
	inet_pton(AF_INET, fromhost, &fromhostn);
#else
    my_inet_aton(fromhost, &fromhostn);
#endif

		exist = www_new_guest_entry(&fromhostn, &idx);
		/* exist:
<0: error
0: 正常登录,idx
1: 有重复,使用idx的entry
		*/

        if (exist < 0)
            ret = 5;
        else {

			if( ! exist ){
            	int tmp = rand() % 100000000;
            	wwwguest_shm->guest_entry[idx].key = tmp;
				wwwguest_shm->guest_entry[idx].fromip.s_addr = fromhostn.s_addr;
            	wwwguest_shm->guest_entry[idx].logintime = time(0);
			}

            wwwguest_shm->guest_entry[idx].freshtime = time(0);
            www_guest_uinfo.freshtime = wwwguest_shm->guest_entry[idx].freshtime;
            www_guest_uinfo.logintime = wwwguest_shm->guest_entry[idx].logintime;
            www_guest_uinfo.destuid = idx;
            www_guest_uinfo.utmpkey = wwwguest_shm->guest_entry[idx].key;
            *ppuinfo = &www_guest_uinfo;
            *putmpent = idx;
            getuser("guest", &getCurrentUser());
            ret = 0;
			if( ! exist )
            	do_after_login(getCurrentUser(),idx,1);
        }
    }

    if ((ret == 0) || (ret == 2)) {
        snprintf(buf, sizeof(buf), "ENTER ?@%s (ALLOC %d) [www]", fullfrom, *putmpent);
        newbbslog(BBSLOG_USIES, "%s", buf);
    }
    return ret;
}

static void setflags(struct userec *u, int mask, int value)
{
    if (((u->flags & mask) && 1) != value) {
        if (value)
            u->flags |= mask;
        else
            u->flags &= ~mask;
    }
}
int www_user_logoff(struct userec *user, int useridx, struct user_info *puinfo, int userinfoidx)
{
    int stay = 0;

    stay = abs(time(0) - puinfo->logintime);
    /*
     * 上站时间超过 2 小时按 2 小时计 
     */
    if (stay > 7200)
        stay = 7200;
    user->stay += stay;
    user->exittime = time(0);
    if (strcasecmp(user->userid, "guest")) {
        newbbslog(BBSLOG_USIES, "EXIT: Stay:%3ld (%s)[%d %d](www)", stay / 60, user->username, get_curr_utmpent(), useridx);
        if (!puinfo->active)
            return 0;
        setflags(user, PAGER_FLAG, (puinfo->pager & ALL_PAGER));

        if ((HAS_PERM(user, PERM_CHATCLOAK) || HAS_PERM(user, PERM_CLOAK)))
            setflags(user, CLOAK_FLAG, puinfo->invisible);
        clear_utmp(userinfoidx, useridx, 1);
    } else {
        newbbslog(BBSLOG_USIES, "EXIT: Stay:%3ld (guest)[%d %d](www)", stay / 60, puinfo->destuid, useridx);
        www_free_guest_entry(puinfo->destuid);
    }
    return 0;
}

time_t set_idle_time(struct user_info * uentp, time_t t)
{
    if (strcasecmp(uentp->userid, "guest"))
        uentp->freshtime = t;
    else {
        int idx;

        idx = uentp->destuid;
        if (idx >= 1 && idx < MAX_WWW_GUEST)
            wwwguest_shm->guest_entry[uentp->destuid].freshtime = t;
    }

    return t;
}

int can_enter_chatroom()
{
    if (HAS_PERM(getCurrentUser(), PERM_CHAT))
        return 1;
    else
        return 0;
}

int can_send_mail()
{
    if (HAS_PERM(getCurrentUser(), PERM_DENYMAIL))
        return 0;
    else if (HAS_PERM(getCurrentUser(), PERM_LOGINOK)) {
        if (chkusermail(getCurrentUser()))
            return 0;
        return 1;
    } else
        return 0;
}

char bin2hex(int val)
{
    int i;

    i = val & 0x0F;
    if (i >= 0 && i < 10)
        return '0' + i;
    else
        return 'A' + (i - 10);
}

char *encode_url(char *buf, const char *str, size_t buflen)
{
    int i, j;
    int len;
    unsigned char c;
    int buflenm1;

    len = strlen(str);
    buf[buflen - 1] = '\0';
    buflenm1 = buflen - 1;
    for (i = 0, j = 0; i < len && j < buflenm1; i++) {
        c = (unsigned char) str[i];
        if (!isalnum(c)) {
            buf[j++] = '%';
            if (j < buflenm1)
                buf[j++] = bin2hex((c >> 4) & 0x0F);
            if (j < buflenm1)
                buf[j++] = bin2hex(c & 0x0F);
        } else {
            buf[j] = str[i];
            j++;
        }
    }
    buf[j] = '\0';

    return buf;
}

char *encode_html(char *buf, const char *str, size_t buflen)
{
    size_t i, j, k;
    size_t len;

    bzero(buf, buflen);
    len = strlen(str);
    for (i = 0, j = 0; i < len && j < buflen; i++) {
        switch (str[i]) {
        case '\"':
            k = buflen - j;
            string_copy(&buf[j], "&quot;", &k);
            j += k;
            break;
        case '&':
            k = buflen - j;
            string_copy(&buf[j], "&amp;", &k);
            j += k;
            break;
            /*
             * case ' ':
             * snprintf(&buf[j], buflen-j, "&nbsp;");
             * j = strlen(buf);
             * break; 
             */
        case '>':
            k = buflen - j;
            string_copy(&buf[j], "&gt;", &k);
            j += k;
            break;
        case '<':
            k = buflen - j;
            string_copy(&buf[j], "&lt;", &k);
            j += k;
            break;
        default:
            buf[j] = str[i];
            j++;
        }
    }
    buf[buflen - 1] = '\0';

    return buf;
}

int is_BM(const struct boardheader *board,const struct userec *user)
{
    char BM[STRLEN];

    strncpy(BM, board->BM, sizeof(BM) - 1);
    BM[sizeof(BM) - 1] = '\0';
    return chk_currBM(BM, (struct userec *)user);
}

char *http_encode_string(char *str, size_t len)
{
    char *buf;

    if (len == 0)
        return NULL;
    buf = (char *) malloc(len);
    if (buf == NULL)
        return NULL;
    encode_url(buf, str, len);
    strncpy(str, buf, len - 1);
    str[len - 1] = '\0';
    free(buf);

    return str;
}

/* Convert string to Unix format */
char *unix_string(char *str)
{
    char *ptr1, *ptr2;

    ptr1 = ptr2 = str;
    while (*ptr1 != '\0') {
        if (*ptr1 == '\r' && *(ptr1 + 1) == '\n') {
            ptr1++;
            continue;
        }
        if (ptr1 != ptr2)
            *ptr2 = *ptr1;
        ptr1++;
        ptr2++;
    }
    *ptr2 = '\0';

    return str;
}

static void print_font_style(unsigned int style, buffered_output_t * output)
{
    char font_class[8];
    char font_style[STRLEN];
    char font_str[256];
    unsigned int bg;
	int len;

    if (STYLE_ISSET(style, FONT_BG_SET)) {
        bg = 8;
    } else
        bg = STYLE_GET_BG(style);
    sprintf(font_class, "f%01d%02d", bg, STYLE_GET_FG(style));
    font_style[0] = '\0';
    if (STYLE_ISSET(style, FONT_STYLE_UL))
        strcat(font_style, "text-decoration: underline; ");
    if (STYLE_ISSET(style, FONT_STYLE_ITALIC))
        strcat(font_style, "font-style: italic; ");
    if (font_style[0] != '\0')
        sprintf(font_str, "<font class=\"%s\" style=\"%s\">", font_class, font_style);
    else
        sprintf(font_str, "<font class=\"%s\">", font_class);
	len = strlen(font_str);
    BUFFERED_OUTPUT(output, font_str, len);
}

/*
static void html_output(char *buf, size_t buflen, buffered_output_t * output)
*/
#define html_output(buf, buflen, output) \
do { \
    size_t _ho_i; \
	const char *_ho_ptr = buf; \
\
    for (_ho_i = 0; _ho_i < buflen; _ho_i++) { \
        switch (_ho_ptr[_ho_i]) { \
        case '&': \
            BUFFERED_OUTPUT(output, "&amp;", 5); \
            break; \
        case '<': \
            BUFFERED_OUTPUT(output, "&lt;", 4); \
            break; \
        case '>': \
            BUFFERED_OUTPUT(output, "&gt;", 4); \
            break; \
        case ' ': \
            BUFFERED_OUTPUT(output, "&nbsp;", 6); \
            break; \
        default: \
            BUFFERED_OUTPUT(output, &(_ho_ptr[_ho_i]), 1); \
        } \
    } \
} while(0)

static void print_raw_ansi(char *buf, size_t buflen, buffered_output_t * output)
{
    size_t i;

    for (i = 0; i < buflen; i++) {
        if (buf[i] == 0x1b)
		{
            html_output("*", 1, output);
		}
        else if (buf[i]=='\n') {
			BUFFERED_OUTPUT(output, " <br /> ", 8);
        } else {
            html_output(&buf[i], 1, output);
		}
    }
}

static void generate_font_style(unsigned int *style, unsigned int *ansi_val, size_t len)
{
    size_t i;
    unsigned int color;

    for (i = 0; i < len; i++) {
        if (ansi_val[i] == 0)
            STYLE_ZERO(*style);
        else if (ansi_val[i] == 1)
            STYLE_SET(*style, FONT_FG_BOLD);
        else if (ansi_val[i] == 4)
            STYLE_SET(*style, FONT_STYLE_UL);
        else if (ansi_val[i] == 5)
            STYLE_SET(*style, FONT_STYLE_BLINK);
        else if (ansi_val[i] >= 30 && ansi_val[i] <= 37) {
            color = ansi_val[i] - 30;
            STYLE_SET_FG(*style, color);
        } else if (ansi_val[i] >= 40 && ansi_val[i] <= 47) {
            /*
             * user explicitly specify background color 
             */
            /*
             * STYLE_SET(*style, FONT_BG_SET); 
             */
            color = ansi_val[i] - 40;
            STYLE_SET_BG(*style, color);
        }
    }
}

/*
static void print_raw_ansi(char *buf, size_t buflen, buffered_output_t * output)
*/
#define js_output(buf, buflen, output) \
do { \
    size_t _js_i; \
	const char *_js_ptr = buf; \
\
    for (_js_i = 0; _js_i < buflen; _js_i++) { \
        switch (_js_ptr[_js_i]) { \
        case '&': \
            BUFFERED_OUTPUT(output, "&amp;", 5); \
            break; \
        case '<': \
            BUFFERED_OUTPUT(output, "&lt;", 4); \
            break; \
        case '>': \
            BUFFERED_OUTPUT(output, "&gt;", 4); \
            break; \
        case ' ': \
            BUFFERED_OUTPUT(output, "&nbsp;", 6); \
            break; \
        case '\'': \
            BUFFERED_OUTPUT(output, "\\\'", 2); \
            break; \
        case '\\': \
            BUFFERED_OUTPUT(output, "\\\\", 2); \
            break; \
		case '\r': \
            BUFFERED_OUTPUT(output, "\\r", 2); \
			break; \
        default: \
            BUFFERED_OUTPUT(output, &(_js_ptr[_js_i]), 1); \
        } \
    } \
} while (0)

void output_ansi_text(char *buf, size_t buflen, 
							buffered_output_t * output, char* attachlink)
{
    unsigned int font_style = 0;
    unsigned int ansi_state;
    unsigned int ansi_val[STRLEN];
    int ival = 0;
    size_t i;
    char *ansi_begin;
    char *ansi_end;
    int attachmatched;
	long attachPos[MAXATTACHMENTCOUNT];
	long attachLen[MAXATTACHMENTCOUNT];
	char* attachFileName[MAXATTACHMENTCOUNT];
	enum ATTACHMENTTYPE attachType[MAXATTACHMENTCOUNT];
	int attachShowed[MAXATTACHMENTCOUNT];
	char outbuf[512];
	int outbuf_len;
	size_t article_len = buflen;

    if (buf == NULL)
        return;

    STATE_ZERO(ansi_state);
    bzero(ansi_val, sizeof(ansi_val));
    bzero(attachShowed, sizeof(attachShowed));
    attachmatched = 0;
	if (attachlink != NULL)
	{
		long attach_len;
		char *attachptr, *attachfilename;
		char *extension;
		for (i = 0; i < buflen ; i++ )
		{
			if (attachmatched >= MAXATTACHMENTCOUNT)
				break;

			if (((attachfilename = checkattach(buf + i, buflen - i, 
									&attach_len, &attachptr)) != NULL))
			{
				extension = attachfilename + strlen(attachfilename);
				i += (attachptr-buf-i) + attach_len - 1;
				if (i > buflen)
					continue;
				attachPos[attachmatched] = attachfilename - buf;
				attachLen[attachmatched] = attach_len;
				attachFileName[attachmatched] = (char*)malloc(256);
				strncpy(attachFileName[attachmatched], attachfilename, 255);
				attachFileName[attachmatched][255] = '\0';
				attachType[attachmatched] = ATTACH_OTHERS;
				extension--;
				while ((*extension != '.') && (*extension != '\0'))
					extension--;
				if (*extension == '.')
				{
					extension++;
					if (!strcasecmp(extension, "jpg")
						|| !strcasecmp(extension, "ico")
						|| !strcasecmp(extension, "gif"))
					{
						attachType[attachmatched] = ATTACH_IMG;
					}
					else if (!strcasecmp(extension, "swf"))
						attachType[attachmatched] = ATTACH_FLASH;
					else if (!strcasecmp(extension, "jpeg")
						|| !strcasecmp(extension, "png")
						|| !strcasecmp(extension, "pcx")
						|| !strcasecmp(extension, "bmp"))
					{
						attachType[attachmatched] = ATTACH_IMG;
					}
				}
				attachmatched++;
			}
		}
	}

	if (attachmatched > 0)
		article_len = attachPos[0] - ATTACHMENT_SIZE;

    for (i = 0; i < article_len; i++)
	{
        if (STATE_ISSET(ansi_state, STATE_NEW_LINE)) {
            STATE_CLR(ansi_state, STATE_NEW_LINE);
            if (i < (buflen - 1) && (buf[i] == ':' && buf[i + 1] == ' ')) {
                STATE_SET(ansi_state, STATE_QUOTE_LINE);
                if (STATE_ISSET(ansi_state, STATE_FONT_SET))
                    BUFFERED_OUTPUT(output, "</font>", 7);
                /*
                 * set quoted line styles 
                 */
                STYLE_SET(font_style, FONT_STYLE_QUOTE);
                STYLE_SET_FG(font_style, FONT_COLOR_QUOTE);
                STYLE_CLR_BG(font_style);
                print_font_style(font_style, output);
                BUFFERED_OUTPUT(output, &buf[i], 1);
                STATE_SET(ansi_state, STATE_FONT_SET);
                STATE_CLR(ansi_state, STATE_ESC_SET);
                /*
                 * clear ansi_val[] array 
                 */
                bzero(ansi_val, sizeof(ansi_val));
                ival = 0;
                continue;
            } else
                STATE_CLR(ansi_state, STATE_QUOTE_LINE);
        }
        if (buf[i] == 0x1b) {
            STATE_SET(ansi_state, STATE_ESC_SET);
        }
		else if (STATE_ISSET(ansi_state, STATE_ESC_SET))
		{
            if (isalpha(buf[i]))
				STATE_CLR(ansi_state, STATE_ESC_SET);
        }
		else if (buf[i] == '\n')
		{
            if (STATE_ISSET(ansi_state, STATE_ESC_SET)) {
                /*
                 *[\n or *[13;24\n */
                size_t len;

                ansi_end = &buf[i - 1];
                len = ansi_end - ansi_begin + 1;
                /*print_raw_ansi(ansi_begin, len, output);*/
                STATE_CLR(ansi_state, STATE_ESC_SET);
            }
            if (STATE_ISSET(ansi_state, STATE_QUOTE_LINE)) {
                /*
                 * end of a quoted line 
                 */
                BUFFERED_OUTPUT(output, "</font>", 7);
                STYLE_CLR(font_style, FONT_STYLE_QUOTE);
                STATE_CLR(ansi_state, STATE_FONT_SET);
            }
            BUFFERED_OUTPUT(output, "<br />\n", 7);
            STATE_CLR(ansi_state, STATE_QUOTE_LINE);
            STATE_SET(ansi_state, STATE_NEW_LINE);
        }
		else
			print_raw_ansi(&buf[i], 1, output);
    }
    if (STATE_ISSET(ansi_state, STATE_FONT_SET)) {
        BUFFERED_OUTPUT(output, "</font>", 7);
        STATE_CLR(ansi_state, STATE_FONT_SET);
    }
	for ( i = 0; i<attachmatched ; i++ ){
		if (!attachShowed[i]) { 
			switch(attachType[i]) {
			case ATTACH_IMG:
		 		snprintf(outbuf, 511, "<br /><img src=\"/images/files/img.gif\" border=\"0\" />此主题相关图片如下：%s (%ld 字节)<br /><a href=\"%s&amp;ap=%ld\" target=\"_blank\"><img src=\"%s&amp;ap=%ld\" border=\"0\" alt=\"按此在新窗口浏览图片\" onload=\"javascript:resizeImg(this)\" /></a> ",attachFileName[i], attachLen[i], attachlink, attachPos[i],attachlink, attachPos[i]);
				break;
			case ATTACH_FLASH:
		        snprintf(outbuf, 511, "<br />Flash动画: " "<a href=\"%s&amp;ap=%ld\">%s</a> (%ld 字节)<br />" "<object classid=\"clsid:D27CDB6E-AE6D-11cf-96B8-444553540000\" codebase=\"http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=5,0,0,0\" > <param name=\"MOVIE\" value=\"%s&amp;ap=%ld\" />" "<embed src=\"%s&amp;ap=%ld\"></embed></object><br />", attachlink, attachPos[i], attachFileName[i], attachLen[i], attachlink, attachPos[i], attachlink, attachPos[i]);
				break;
			case ATTACH_OTHERS:
				 snprintf(outbuf, 511, "<br />附件: <a href=\"%s&amp;ap=%ld\">%s</a> (%ld 字节)<br />", attachlink, attachPos[i], attachFileName[i], attachLen[i]);
				 break;
			}	
			outbuf_len = strlen(outbuf);
			BUFFERED_OUTPUT(output, outbuf, outbuf_len);
			attachShowed[i]=1;
		}
		free(attachFileName[i]);
	}

    BUFFERED_FLUSH(output);

}

#define JS_BUFFERED_OUTPUT(buf, buflen, output) \
do { \
    size_t _jbo_i; \
	const char *_jbo_ptr = buf; \
\
    for (_jbo_i = 0; _jbo_i < buflen; _jbo_i++) { \
        switch (_jbo_ptr[_jbo_i]) { \
        case '\'': \
            BUFFERED_OUTPUT(output, "\\\'", 2); \
            break; \
        case '\\': \
            BUFFERED_OUTPUT(output, "\\\\", 2); \
            break; \
		case '\r': \
            BUFFERED_OUTPUT(output, "\\r", 2); \
			break; \
        default: \
            BUFFERED_OUTPUT(output, &(_jbo_ptr[_jbo_i]), 1); \
        } \
    } \
} while (0)

void output_ansi_javascript(char *buf, size_t buflen, 
							buffered_output_t * output, char* attachlink)
{
    unsigned int font_style = 0;
    unsigned int ansi_state;
    unsigned int ansi_val[STRLEN];
    int ival = 0;
    size_t i;
    char *ansi_begin;
    char *ansi_end;
    int attachmatched;
	long attachPos[MAXATTACHMENTCOUNT];
	long attachLen[MAXATTACHMENTCOUNT];
	char* attachFileName[MAXATTACHMENTCOUNT];
	enum ATTACHMENTTYPE attachType[MAXATTACHMENTCOUNT];
	int attachShowed[MAXATTACHMENTCOUNT];
	char outbuf[512];
	int outbuf_len;
	size_t article_len = buflen;

    if (buf == NULL)
        return;

    STATE_ZERO(ansi_state);
    bzero(ansi_val, sizeof(ansi_val));
    bzero(attachShowed, sizeof(attachShowed));
    attachmatched = 0;
	if (attachlink != NULL)
	{
		long attach_len;
		char *attachptr, *attachfilename;
		char *extension;
		for (i = 0; i < buflen ; i++ )
		{
			if (attachmatched >= MAXATTACHMENTCOUNT)
				break;

			if (((attachfilename = checkattach(buf + i, buflen - i, 
									&attach_len, &attachptr)) != NULL))
			{
				extension = attachfilename + strlen(attachfilename);
				i += (attachptr-buf-i) + attach_len - 1;
				if (i > buflen)
					continue;
				attachPos[attachmatched] = attachfilename - buf;
				attachLen[attachmatched] = attach_len;
				attachFileName[attachmatched] = (char*)malloc(256);
				strncpy(attachFileName[attachmatched], attachfilename, 255);
				attachFileName[attachmatched][255] = '\0';
				attachType[attachmatched] = ATTACH_OTHERS;
				extension--;
				while ((*extension != '.') && (*extension != '\0'))
					extension--;
				if (*extension == '.')
				{
					extension++;
					if (!strcasecmp(extension, "jpg")
						|| !strcasecmp(extension, "ico")
						|| !strcasecmp(extension, "gif"))
					{
						attachType[attachmatched] = ATTACH_IMG;
					}
					else if (!strcasecmp(extension, "swf"))
						attachType[attachmatched] = ATTACH_FLASH;
					else if (!strcasecmp(extension, "jpeg")
						|| !strcasecmp(extension, "png")
						|| !strcasecmp(extension, "pcx")
						|| !strcasecmp(extension, "bmp"))
					{
						attachType[attachmatched] = ATTACH_IMG;
					}
				}
				attachmatched++;
			}
		}
	}

	if (attachmatched > 0)
		article_len = attachPos[0] - ATTACHMENT_SIZE;

	BUFFERED_OUTPUT(output, "document.write('", 16);
    for (i = 0; i < article_len; i++)
	{
        if (STATE_ISSET(ansi_state, STATE_NEW_LINE)) {
			BUFFERED_OUTPUT(output, "document.write('", 16);
            STATE_CLR(ansi_state, STATE_NEW_LINE);
            if (i < (buflen - 1) && (buf[i] == ':' && buf[i + 1] == ' ')) {
                STATE_SET(ansi_state, STATE_QUOTE_LINE);
                if (STATE_ISSET(ansi_state, STATE_FONT_SET))
                    BUFFERED_OUTPUT(output, "</font>", 7);
                /*
                 * set quoted line styles 
                 */
                STYLE_SET(font_style, FONT_STYLE_QUOTE);
                STYLE_SET_FG(font_style, FONT_COLOR_QUOTE);
                STYLE_CLR_BG(font_style);
                print_font_style(font_style, output);
                BUFFERED_OUTPUT(output, &buf[i], 1);
                STATE_SET(ansi_state, STATE_FONT_SET);
                STATE_CLR(ansi_state, STATE_ESC_SET);
                /*
                 * clear ansi_val[] array 
                 */
                bzero(ansi_val, sizeof(ansi_val));
                ival = 0;
                continue;
            } else
                STATE_CLR(ansi_state, STATE_QUOTE_LINE);
        }
        if (buf[i] == 0x1b) {
            STATE_SET(ansi_state, STATE_ESC_SET);
        }
		else if (STATE_ISSET(ansi_state, STATE_ESC_SET))
		{
            if (isalpha(buf[i]))
				STATE_CLR(ansi_state, STATE_ESC_SET);
        }
		else if (buf[i] == '\n')
		{
            if (STATE_ISSET(ansi_state, STATE_ESC_SET)) {
                /*
                 *[\n or *[13;24\n */
                size_t len;

                ansi_end = &buf[i - 1];
                len = ansi_end - ansi_begin + 1;
                /*print_raw_ansi(ansi_begin, len, output);*/
                STATE_CLR(ansi_state, STATE_ESC_SET);
            }
            if (STATE_ISSET(ansi_state, STATE_QUOTE_LINE)) {
                /*
                 * end of a quoted line 
                 */
                BUFFERED_OUTPUT(output, "</font>", 7);
                STYLE_CLR(font_style, FONT_STYLE_QUOTE);
                STATE_CLR(ansi_state, STATE_FONT_SET);
            }
            BUFFERED_OUTPUT(output, "<br />');\n", 10);
            STATE_CLR(ansi_state, STATE_QUOTE_LINE);
            STATE_SET(ansi_state, STATE_NEW_LINE);
        }
		else
			js_output(&buf[i], 1, output);
    }
    if (STATE_ISSET(ansi_state, STATE_FONT_SET)) {
        BUFFERED_OUTPUT(output, "</font>", 7);
        STATE_CLR(ansi_state, STATE_FONT_SET);
    }
	if (!STATE_ISSET(ansi_state, STATE_NEW_LINE)) {
		BUFFERED_OUTPUT(output, "<br />');\n", 10);
	}
	for ( i = 0; i<attachmatched ; i++ ){
		if (!attachShowed[i]) { 
			switch(attachType[i]) {
			case ATTACH_IMG:
		 		snprintf(outbuf, 511, "<br /><img src=\"/images/files/img.gif\" border=\"0\" />此主题相关图片如下：%s (%ld 字节)<br /><a href=\"%s&amp;ap=%ld\" target=\"_blank\"><img src=\"%s&amp;ap=%ld\" border=\"0\" alt=\"按此在新窗口浏览图片\" onload=\"javascript:resizeImg(this)\" /></a> ",attachFileName[i], attachLen[i], attachlink, attachPos[i],attachlink, attachPos[i]);
				break;
			case ATTACH_FLASH:
		        snprintf(outbuf, 511, "<br />Flash动画: " "<a href=\"%s&amp;ap=%ld\">%s</a> (%ld 字节)<br />" "<object classid=\"clsid:D27CDB6E-AE6D-11cf-96B8-444553540000\" codebase=\"http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=5,0,0,0\" > <param name=\"MOVIE\" value=\"%s&amp;ap=%ld\" />" "<embed src=\"%s&amp;ap=%ld\"></embed></object><br />", attachlink, attachPos[i], attachFileName[i], attachLen[i], attachlink, attachPos[i], attachlink, attachPos[i]);
				break;
			case ATTACH_OTHERS:
				 snprintf(outbuf, 511, "<br />附件: <a href=\"%s&amp;ap=%ld\">%s</a> (%ld 字节)<br />", attachlink, attachPos[i], attachFileName[i], attachLen[i]);
				 break;
			}	
			BUFFERED_OUTPUT(output, "document.write('", 16);
			outbuf_len = strlen(outbuf);
			JS_BUFFERED_OUTPUT(outbuf, outbuf_len, output);
			BUFFERED_OUTPUT(output, "');\n", 4);
			attachShowed[i]=1;
		}
		free(attachFileName[i]);
	}

    BUFFERED_FLUSH(output);

}


/**
 * Warning: Use of this function is deprecated. It's kept only for compatible
 * purpose. Use output_ansi_text() instead.
 *
 * This function is a core function in wForum, do not modify unless you know what you are doing. - atppp
 *
 * 原来的 UBB 处理代码过于 buggy，现在全盘去掉换成只支持解析 [upload=%d][/upload] 一种，如果以后 ubb 替换要放到这里，
 * 可以参考这个时间点之前的 CVS 代码。- atppp 200408
 */
void output_ansi_html(char *buf, size_t buflen, buffered_output_t * output,char* attachlink, int is_tex, char* preview_attach_dir)
{
    unsigned int font_style = 0;
    unsigned int ansi_state;
    unsigned int ansi_val[STRLEN];
    int ival = 0;
    size_t i;
    char *ansi_begin;
    char *ansi_end;
    int attachmatched;
    long attachPos[MAXATTACHMENTCOUNT];
    long attachLen[MAXATTACHMENTCOUNT];
    char* attachFileName[MAXATTACHMENTCOUNT];
    enum ATTACHMENTTYPE attachType[MAXATTACHMENTCOUNT];
    int attachShowed[MAXATTACHMENTCOUNT];
    char outbuf[512];
    int outbuf_len;
    size_t article_len = buflen;

    /* hardcode everything for supporting [upload=%d][/upload], hehe. - atppp */
    char *UBBUpload1 = "upload=";
    int UBBUpload1_len = 7;
    char *UBBUpload2 = "][/upload]";
    int UBBUpload2_len = 10;

    if (buf == NULL)
        return;

    STATE_ZERO(ansi_state);
    bzero(ansi_val, sizeof(ansi_val));
    bzero(attachShowed, sizeof(attachShowed));
    attachmatched = 0;
    if (attachlink != NULL)
    {
        long attach_len;
        char *attachptr, *attachfilename;
        if (preview_attach_dir) {
            char filepath[MAXPATH];
            char inputbuf[256];
            FILE *fp2;
            snprintf(filepath, MAXPATH, "%s/.index", preview_attach_dir);
            if ((fp2 = fopen(filepath, "r")) != NULL) {
                while (!feof(fp2)) {
                    char *name;
                    char *ptr;
                    struct stat st;

                    if (attachmatched >= MAXATTACHMENTCOUNT)
                        break;                    

                    fgets(inputbuf, 256, fp2);
                    name = strchr(inputbuf, ' ');
                    if (name == NULL)
                        continue;
                    *name = 0;
                    name++;
                    ptr = strchr(name, '\n');
                    if (ptr)
                        *ptr = 0;

                    attachPos[attachmatched] = attachmatched+1;
                    if (stat(inputbuf, &st) < 0) attachLen[attachmatched] = 0;
                    else attachLen[attachmatched] = st.st_size;
                    attachFileName[attachmatched] = (char*)malloc(256);
                    strncpy(attachFileName[attachmatched], name, 255);
                    attachFileName[attachmatched][255] = '\0';
                    attachType[attachmatched] = get_attachment_type(name);

                    attachmatched++;
                }
    			fclose(fp2);
            }
        } else {
            for (i = 0; i < buflen ; i++ )
            {
                if (attachmatched >= MAXATTACHMENTCOUNT)
                    break;

                if (((attachfilename = checkattach(buf + i, buflen - i, 
                                        &attach_len, &attachptr)) != NULL))
                {
                    i += (attachptr-buf-i) + attach_len - 1;
                    if (i > buflen)
                        continue;
                    attachPos[attachmatched] = attachfilename - buf;
                    attachLen[attachmatched] = attach_len;
                    attachFileName[attachmatched] = (char*)malloc(256);
                    strncpy(attachFileName[attachmatched], attachfilename, 255);
                    attachFileName[attachmatched][255] = '\0';
                    attachType[attachmatched] = get_attachment_type(attachfilename);
                    attachmatched++;
                }
            }
            if (attachmatched > 0)
                article_len = attachPos[0] - ATTACHMENT_SIZE;
        }
    }


    for (i = 0; i < article_len; i++) {
        if (STATE_ISSET(ansi_state, STATE_NEW_LINE)) {
            STATE_CLR(ansi_state, STATE_NEW_LINE);
            if (i < (buflen - 1) && !STATE_ISSET(ansi_state,STATE_TEX_SET) && (buf[i] == ':' && buf[i + 1] == ' ')) {
                STATE_SET(ansi_state, STATE_QUOTE_LINE);
                if (STATE_ISSET(ansi_state, STATE_FONT_SET))
                    BUFFERED_OUTPUT(output, "</font>", 7);
                /*
                 * set quoted line styles 
                 */
                STYLE_SET(font_style, FONT_STYLE_QUOTE);
                STYLE_SET_FG(font_style, FONT_COLOR_QUOTE);
                STYLE_CLR_BG(font_style);
                print_font_style(font_style, output);
                BUFFERED_OUTPUT(output, &buf[i], 1);
                STATE_SET(ansi_state, STATE_FONT_SET);
                STATE_CLR(ansi_state, STATE_ESC_SET);
                /*
                 * clear ansi_val[] array 
                 */
                bzero(ansi_val, sizeof(ansi_val));
                ival = 0;
                continue;
            } else
                STATE_CLR(ansi_state, STATE_QUOTE_LINE);
        }
        if (is_tex) {
            if ((buf[i] == '$') && ((i==0) || (buf[i-1] != '\\'))) {
                if (STATE_ISSET(ansi_state, STATE_TEX_SET)) {
                    STATE_CLR(ansi_state, STATE_TEX_SET);
                } else {
                    if (STATE_ISSET(ansi_state, STATE_QUOTE_LINE)) {
                        BUFFERED_OUTPUT(output, "</font>", 7);
                        STYLE_CLR(font_style, FONT_STYLE_QUOTE);
                        STATE_CLR(ansi_state, STATE_QUOTE_LINE);
                        STATE_CLR(ansi_state, STATE_FONT_SET);
                    } else if (STATE_ISSET(ansi_state, STATE_FONT_SET)) {
                        BUFFERED_OUTPUT(output, "</font>", 7);
                        STATE_CLR(ansi_state, STATE_FONT_SET);
                    }
                    STATE_SET(ansi_state, STATE_TEX_SET);
                }
            }
        }
        /*
        * is_tex 情况下，\[upload 优先匹配 \[ 而不是 [upload
        * is_tex 情况下应该还有一个问题是 *[\[ 等，不过暂时不管了 - atppp
        */
        if (i < (buflen - 1) && is_tex && !STATE_ISSET(ansi_state,STATE_ESC_SET) && (buf[i] == '\\' && buf[i + 1] == '[')) {
            if (STATE_ISSET(ansi_state, STATE_QUOTE_LINE)) {
                BUFFERED_OUTPUT(output, "</font>", 7);
                STYLE_CLR(font_style, FONT_STYLE_QUOTE);
                STATE_CLR(ansi_state, STATE_QUOTE_LINE);
                STATE_CLR(ansi_state, STATE_FONT_SET);
            } else if (STATE_ISSET(ansi_state, STATE_FONT_SET)) {
                BUFFERED_OUTPUT(output, "</font>", 7);
                STATE_CLR(ansi_state, STATE_FONT_SET);
            }
            STATE_SET(ansi_state, STATE_TEX_SET);
        }
        if (buf[i] == '[' && !STATE_ISSET(ansi_state, STATE_ESC_SET)) {
            if (is_tex && (i > 0) && (buf[i-1] == '\\')) {
            } else if (attachmatched > 0) {
                /*
                 * determine if this is correct attachment [upload], if correct, output html, and skip i to the end of [/upload]
                 * otherwise, do nothing and keep i unchanged
                 */
                int num = 0;
                int k;
                char *buf_p;
                char *end_buf_p;
                char *UBB_p;
                buf_p = &buf[i];
                end_buf_p = &buf[buflen];
                buf_p++;
                if (buf_p + UBBUpload1_len - 1 >= end_buf_p) {
                    goto atppp_never_use_goto;
                }
                for (UBB_p = UBBUpload1, k = 0; k < UBBUpload1_len; k++, UBB_p++, buf_p++) {
                    if (*UBB_p != *buf_p) {
                        goto atppp_never_use_goto;
                    }
                }
                while(buf_p < end_buf_p) {
                    if ((*buf_p >= '0') && (*buf_p <= '9')) {
                        num = num * 10 + (*buf_p - '0');
                    } else {
                        break;
                    }
                    buf_p++;
                }
                if (buf_p + UBBUpload2_len - 1 >= end_buf_p) {
                    goto atppp_never_use_goto;
                }
                for (UBB_p = UBBUpload2, k = 0; k < UBBUpload2_len; k++, UBB_p++, buf_p++) {
                    if (*UBB_p != *buf_p) {
                        goto atppp_never_use_goto;
                    }
                }
                if ((num > 0) && (num <= attachmatched)) {
                    switch(attachType[num-1]) {
                    case ATTACH_IMG:
                        snprintf(outbuf, 511, "<a href=\"%s&amp;ap=%ld\" target=\"_blank\"><img src=\"%s&amp;ap=%ld\" border=\"0\" alt=\"按此在新窗口浏览图片\" onload=\"javascript:resizeImg(this)\" /></a> ", attachlink, attachPos[num-1],attachlink, attachPos[num-1]);
                        break;
                    case ATTACH_FLASH:
                        snprintf(outbuf, 511, "<object classid=\"clsid:D27CDB6E-AE6D-11cf-96B8-444553540000\" codebase=\"http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=5,0,0,0\" > <param name=\"MOVIE\" value=\"%s&amp;ap=%ld\" />" "<embed src=\"%s&amp;ap=%ld\"></embed></object>", attachlink, attachPos[num-1], attachlink, attachPos[num-1]);
                        break;
                    case ATTACH_OTHERS:
                         snprintf(outbuf, 511, "<br />附件: <a href=\"%s&amp;ap=%ld\">%s</a> (%ld 字节)<br />", attachlink, attachPos[num-1], attachFileName[num-1], attachLen[num-1]);
                         break;
                    }   
                    outbuf_len = strlen(outbuf);
                    BUFFERED_OUTPUT(output, outbuf, outbuf_len);
                    attachShowed[num-1]=1;
                    i = buf_p - buf - 1;
                    continue;
                }
atppp_never_use_goto:
                NULL;
            }
        } else if (is_tex && (buf[i] == ']') && (i > 0) && (buf[i-1] == '\\')) {
            STATE_CLR(ansi_state, STATE_TEX_SET);
        }
        if (i < (buflen - 1) && !STATE_ISSET(ansi_state,STATE_TEX_SET) && (buf[i] == 0x1b && buf[i + 1] == '[')) {
            if (STATE_ISSET(ansi_state, STATE_ESC_SET)) {
                /*
                 *[*[ or *[13;24*[ */
                size_t len;

                ansi_end = &buf[i - 1];
                len = ansi_end - ansi_begin + 1;
                print_raw_ansi(ansi_begin, len, output);
            }
            STATE_SET(ansi_state, STATE_ESC_SET);
            ansi_begin = &buf[i];
            i++;                /* skip the next '[' character */
        } else if (buf[i] == '\n') {
            if (STATE_ISSET(ansi_state, STATE_ESC_SET)) {
                /*
                 *[\n or *[13;24\n */
                size_t len;

                ansi_end = &buf[i - 1];
                len = ansi_end - ansi_begin + 1;
                print_raw_ansi(ansi_begin, len, output);
                STATE_CLR(ansi_state, STATE_ESC_SET);
            }
            if (STATE_ISSET(ansi_state, STATE_QUOTE_LINE)) {
                /*
                 * end of a quoted line 
                 */
                BUFFERED_OUTPUT(output, "</font>", 7);
                STYLE_CLR(font_style, FONT_STYLE_QUOTE);
                STATE_CLR(ansi_state, STATE_FONT_SET);
            }
            if (!STATE_ISSET(ansi_state,STATE_TEX_SET)) {
                BUFFERED_OUTPUT(output, " <br /> ", 8);
            }
            STATE_CLR(ansi_state, STATE_QUOTE_LINE);
            STATE_SET(ansi_state, STATE_NEW_LINE);
        } else {
            if (STATE_ISSET(ansi_state, STATE_ESC_SET)) {
                if (buf[i] == 'm') {
                    /*
                     *[0;1;4;31m */
                    if (STATE_ISSET(ansi_state, STATE_FONT_SET)) {
                        BUFFERED_OUTPUT(output, "</font>", 7);
                        STATE_CLR(ansi_state, STATE_FONT_SET);
                    }
                    if (i < buflen - 1) {
                        generate_font_style(&font_style, ansi_val, ival + 1);
                        if (STATE_ISSET(ansi_state, STATE_QUOTE_LINE))
                            STYLE_SET(font_style, FONT_STYLE_QUOTE);
                        print_font_style(font_style, output);
                        STATE_SET(ansi_state, STATE_FONT_SET);
                        STATE_CLR(ansi_state, STATE_ESC_SET);
                        /*
                         * STYLE_ZERO(font_style);
                         */
                        /*
                         * clear ansi_val[] array 
                         */
                        bzero(ansi_val, sizeof(ansi_val));
                        ival = 0;
                    }
                } else if (isalpha(buf[i])) {
                    /*
                     *[23;32H */
                    /*
                     * ignore it 
                     */
                    STATE_CLR(ansi_state, STATE_ESC_SET);
                    STYLE_ZERO(font_style);
                    /*
                     * clear ansi_val[] array 
                     */
                    bzero(ansi_val, sizeof(ansi_val));
                    ival = 0;
                    continue;
                } else if (buf[i] == ';') {
                    if (ival < sizeof(ansi_val) - 1) {
                        ival++; /* go to next ansi_val[] element */
                        ansi_val[ival] = 0;
                    }
                } else if (buf[i] >= '0' && buf[i] <= '9') {
                    ansi_val[ival] *= 10;
                    ansi_val[ival] += (buf[i] - '0');
                } else {
                    /*
                     *[1;32/XXXX or *[* or *[[ */
                    /*
                     * not a valid ANSI string, just output it 
                     */
                    size_t len;

                    ansi_end = &buf[i];
                    len = ansi_end - ansi_begin + 1;
                    print_raw_ansi(ansi_begin, len, output);
                    STATE_CLR(ansi_state, STATE_ESC_SET);
                    /*
                     * clear ansi_val[] array 
                     */
                    bzero(ansi_val, sizeof(ansi_val));
                    ival = 0;
                }

            } else
                print_raw_ansi(&buf[i], 1, output);
        }
    }
    if (STATE_ISSET(ansi_state, STATE_FONT_SET)) {
        BUFFERED_OUTPUT(output, "</font>", 7);
        STATE_CLR(ansi_state, STATE_FONT_SET);
    }
    for ( i = 0; i<attachmatched ; i++ ){
        if (!attachShowed[i]) { 
            switch(attachType[i]) {
            case ATTACH_IMG:
                snprintf(outbuf, 511, "<br /><img src=\"/images/files/img.gif\" border=\"0\" />此主题相关图片如下：%s (%ld 字节)<br /><a href=\"%s&amp;ap=%ld\" target=\"_blank\"><img src=\"%s&amp;ap=%ld\" border=\"0\" alt=\"按此在新窗口浏览图片\" onload=\"javascript:resizeImg(this)\" /></a> ",attachFileName[i], attachLen[i], attachlink, attachPos[i],attachlink, attachPos[i]);
                break;
            case ATTACH_FLASH:
                snprintf(outbuf, 511, "<br />Flash动画: " "<a href=\"%s&amp;ap=%ld\">%s</a> (%ld 字节)<br />" "<object classid=\"clsid:D27CDB6E-AE6D-11cf-96B8-444553540000\" codebase=\"http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=5,0,0,0\" > <param name=\"MOVIE\" value=\"%s&amp;ap=%ld\" />" "<embed src=\"%s&amp;ap=%ld\"></embed></object><br />", attachlink, attachPos[i], attachFileName[i], attachLen[i], attachlink, attachPos[i], attachlink, attachPos[i]);
                break;
            case ATTACH_OTHERS:
                 snprintf(outbuf, 511, "<br />附件: <a href=\"%s&amp;ap=%ld\">%s</a> (%ld 字节)<br />", attachlink, attachPos[i], attachFileName[i], attachLen[i]);
                 break;
            }
            outbuf_len = strlen(outbuf);
            BUFFERED_OUTPUT(output, outbuf, outbuf_len);
            attachShowed[i]=1;
        }
        free(attachFileName[i]);
    }

    BUFFERED_FLUSH(output);

}

/* ent 是 1-based 的*/
int del_post(int ent, struct fileheader *fileinfo, char *direct, char *board)
{
    char usrid[STRLEN];
    int owned;
    struct userec *user;
    char bm_str[BM_LEN - 1];
    struct boardheader *bp;
    struct write_dir_arg delarg;
    int ret;

    user = getCurrentUser();
    bp = getbcache(board);
    memcpy(bm_str, bp->BM, BM_LEN - 1);
    if (!strcmp(board, "syssecurity")
        || !strcmp(board, "junk")
        || !strcmp(board, "deleted"))   /* Leeward : 98.01.22 */
        return 4;

    if (fileinfo->owner[0] == '-') {
        return 2;
    }
    owned = isowner(user, fileinfo);
    /* change by KCN  ! strcmp( fileinfo->owner, getCurrentUser()->userid ); */
    strcpy(usrid, fileinfo->owner);
    if (!(owned) && !HAS_PERM(getCurrentUser(), PERM_SYSOP))
        if (!chk_currBM(bm_str, getCurrentUser())) {
            return 1;
        }
    init_write_dir_arg(&delarg);
    setbdir(DIR_MODE_NORMAL, direct, bp->filename);
    delarg.filename=direct;
    delarg.ent=ent;
    ret=do_del_post(getCurrentUser(), &delarg, fileinfo, board, DIR_MODE_NORMAL, true, getSession());
    free_write_dir_arg(&delarg);
    if (ret != 0)
        return 3;
    return 0;

}



#ifdef HAVE_WFORUM /* 这个 #ifdef 里面用于产生 .WEBTHREAD 索引 - atppp */

// balance is LH (left heavy), EH (even), RH (right heavy)
enum balance { LH, EH, RH };
typedef struct _wwwthread_treenode {
    struct wwwthreadheader content; // what's written into .WEBTHREAD finally

    /* AVL Tree supportive variables. This is used for quickly searching groupid. */
    enum balance bf;                // bf is the balance factor
    struct _wwwthread_treenode *Lchild;
    struct _wwwthread_treenode *Rchild;

    /* linked list supportive variables. This linked list is sorted by lastreply.posttime, so can be written into .WEBTHREAD linearly */
    struct _wwwthread_treenode *previous;
} wwwthread_treenode;


/* following four routines are standard AVL operation support. */
static wwwthread_treenode *AVL_RotateLeft(wwwthread_treenode * p)
{
    wwwthread_treenode *temp;
    temp = p->Rchild;
    p->Rchild = temp->Lchild;
    temp->Lchild = p;
    p = temp;
    return p;
}

static wwwthread_treenode *AVL_RotateRight(wwwthread_treenode * p)
{
    wwwthread_treenode *temp;
    temp = p->Lchild;
    p->Lchild = temp->Rchild;
    temp->Rchild = p;
    p = temp;
    return p;
}

static wwwthread_treenode *AVL_RightBalance(wwwthread_treenode * r)
{
    wwwthread_treenode *x;
    wwwthread_treenode *w;

    x = r->Rchild;
    switch (x->bf) {
    case RH:
        r->bf = EH;
        x->bf = EH;
        r = AVL_RotateLeft(r);
        break;
    case LH:
        w = x->Lchild;
        switch (w->bf) {
        case EH:
            r->bf = EH;
            x->bf = EH;
            break;
        case LH:
            r->bf = EH;
            x->bf = RH;
            break;
        case RH:
            r->bf = LH;
            x->bf = EH;
            break;
        }
        w->bf = EH;
        x = AVL_RotateRight(x);
        r->Rchild = x;
        r = AVL_RotateLeft(r);
    }
    return r;
}

static wwwthread_treenode *AVL_LeftBalance(wwwthread_treenode * r)
{
    wwwthread_treenode *x;
    wwwthread_treenode *w;

    x = r->Lchild;
    switch (x->bf) {
    case LH:
        r->bf = EH;
        x->bf = EH;
        r = AVL_RotateRight(r);
        break;
    case RH:
        w = x->Rchild;
        switch (w->bf) {
        case EH:
            r->bf = EH;
            x->bf = EH;
            break;
        case RH:
            r->bf = EH;
            x->bf = LH;
            break;
        case LH:
            r->bf = RH;
            x->bf = EH;
            break;
        }
        w->bf = EH;
        x = AVL_RotateLeft(x);
        r->Lchild = x;
        r = AVL_RotateRight(r);
    }
    return r;
}

/*
 * insert a node into the AVL Tree.
 *
 * @param proot     *proot is the place to store the current root node, can be changed if the tree is rotated after insertion
 * @param fh        *fh is the fileheader information to be stored in content field.
 * @param flags     newly created node->content.flags
 * @param previous  *previous is the place to store the previous node in the linked list. If the groupid is already in the AVL
 *                  tree, then no node is created and *previous leaves unchanged; if a node is newly created, *previous stores
 *                  the newly created node address
 *
 * @author  atppp
 */
static bool AVL_Insert(wwwthread_treenode ** proot, struct fileheader *fh, int flags, wwwthread_treenode** previous)
{
    bool tallersubtree;
    bool taller;
    wwwthread_treenode *root;
    int cmp;
    
    root = *proot;
    if (root == NULL) {
        root = (wwwthread_treenode *) malloc(sizeof(wwwthread_treenode));
        if (root == NULL) {
            *previous = NULL;
            return false;
        }
        root->content.lastreply = *fh;

        /*
         * here is the trick: if this post is not the original post, set origin.groupid to something else.
         * So later when written into the .WEBTHREAD file, we know whether the original post does exist or not.
         */
        if (fh->id == fh->groupid)
            root->content.origin = *fh;
        else
            root->content.origin.groupid = fh->groupid + 1;
        
        root->content.flags = flags;
        root->content.articlecount = 1;
        root->content.unused = 0;
        
        root->previous = *previous; //linked list support
        *previous = root;           //return the newly created node address in *previous, so the caller can set tail variable.

        /* AVL Tree support */
        root->Lchild = NULL;
        root->Rchild = NULL;
        root->bf = EH;
        taller = true;
    } else {
        cmp = fh->groupid - root->content.lastreply.groupid;
        if (cmp < 0) {
            tallersubtree = AVL_Insert(&(root->Lchild), fh, flags, previous);
            if (tallersubtree)
                switch (root->bf) {
                case LH:
                    root = AVL_LeftBalance(root);
                    taller = false;
                    break;
                case EH:
                    root->bf = LH;
                    taller = true;
                    break;
                case RH:
                    root->bf = EH;
                    taller = false;
                    break;
                }

            else
                taller = false;
        } else if (cmp > 0) {
            tallersubtree = AVL_Insert(&(root->Rchild), fh, flags, previous);
            if (tallersubtree)
                switch (root->bf) {
                case LH:
                    root->bf = EH;
                    taller = false;
                    break;
                case EH:
                    root->bf = RH;
                    taller = true;
                    break;
                case RH:
                    root = AVL_RightBalance(root);
                    taller = false;
                }

            else
                taller = false;
        } else {  // the groupid (node) already exists
            /* We have a bug here: sysmail board won't generate correct .WEBTHREAD file. But who cares!!! */
			if (root->content.lastreply.groupid == root->content.lastreply.id) { // this node was created by Zhiding fileheader
				root->content.lastreply = *fh;
			} else {
				root->content.articlecount++;
                if ((fh->groupid == fh->id) && (root->content.flags != FILE_ON_TOP)) { //found the original post, put into origin field
                                               /* 后面这个判断只是为了保证新旧代码产生 .WEBTHREAD 完全一致。这个细节以后还要调整 */
                    root->content.origin =*fh;
                }
            }
            taller = false;
        }
    }
    *proot = root;
    return taller;
}

static void clearWWWThreadList(wwwthread_treenode *p){
	wwwthread_treenode *q;
	while (p!=NULL) {
		q=p->previous;
		free(p);
		p=q;
	}
}

int www_generateOriginIndex(const char* board)
/* added by roy 2003.7.17 generate .WEBTHREAD index file. Modified by atppp 2004.06.03 */
{
    struct fileheader *ptr1;
    struct flock ldata, ldata2;
    int fd, fd2, size, total, i;
    char olddirect[PATHLEN];
	char currdirect[PATHLEN];
    char *ptr;
    struct stat buf;

    /* AVL Tree support */
    wwwthread_treenode *root = NULL;

    /* linked list support */
    wwwthread_treenode *tail = NULL;
    wwwthread_treenode *temp = NULL;
    
    int bid;
    struct BoardStatus* bs;

    setbdir(DIR_MODE_NORMAL, olddirect, board);
    setbdir(DIR_MODE_WEB_THREAD, currdirect, board);
    if ((fd = open(currdirect, O_WRONLY | O_CREAT | O_TRUNC, 0664)) == -1) {
        bbslog("user", "%s", "recopen err");
        return -1;      /* 创建文件发生错误*/
    }
    ldata.l_type = F_WRLCK;
    ldata.l_whence = 0;
    ldata.l_len = 0;
    ldata.l_start = 0;
    if (fcntl(fd, F_SETLKW, &ldata) == -1) {
        bbslog("user", "%s", "reclock err");
        close(fd);
        return -2;      /* lock error*/
    }
    /* 开始互斥过程*/

    if ((fd2 = open(olddirect, O_RDONLY, 0664)) == -1) {
        bbslog("user", "%s", "recopen err");
        ldata.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW, &ldata);
        close(fd);
        return -3;
    }
    ldata2.l_type = F_RDLCK;
    ldata2.l_whence = 0;
    ldata2.l_len = 0;
    ldata2.l_start = 0;
    if (fcntl(fd2, F_SETLKW, &ldata2) == -1) {
        bbslog("user", "%s", "reclock err");
        ldata.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW, &ldata);
        close(fd);
        close(fd2);
        return -4;      /* lock error*/
    }
    if (fstat(fd2, &buf)==-1) {
        ldata2.l_type = F_UNLCK;
        fcntl(fd2, F_SETLKW, &ldata2);
        close(fd2);
        ldata.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW, &ldata);
        close(fd);
        return -5;
	}
	
    total = buf.st_size /sizeof(fileheader);

    if ((i = safe_mmapfile_handle(fd2,  PROT_READ, MAP_SHARED, (void **) &ptr, &buf.st_size)) != 1) {
        if (i == 2)
            end_mmapfile((void *) ptr, buf.st_size, -1);
        ldata2.l_type = F_UNLCK;
        fcntl(fd2, F_SETLKW, &ldata2);
        close(fd2);
        ldata.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW, &ldata);
        close(fd);
        return -5;
    }

	size=sizeof(struct wwwthreadheader);

    bid = getbnum(board);
    bs = getbstatus(bid);
    for (i=bs->toptitle-1;i>=0;i--) {
        if (bs->topfh[i].groupid!=bs->topfh[i].id) continue;
        AVL_Insert(&root, &(bs->topfh[i]), FILE_ON_TOP, &temp);
        if (temp == NULL) { //malloc failure, impossible?
            clearWWWThreadList(tail);
            return -5;
        }
        tail = temp;
    }


    ptr1 = (struct fileheader *) ptr;
	for (i=total-1;i>=0;i--) {
        AVL_Insert(&root, &(ptr1[i]), 0, &temp);
        if (temp == NULL) { //malloc failure, impossible?
				clearWWWThreadList(tail);
				return -5;
        }
        tail = temp;
	}
	while (tail!=NULL) {
		temp=tail->previous;
        if (tail->content.origin.groupid != tail->content.lastreply.groupid) {
            //original post does not exist, do something
            tail->content.origin = tail->content.lastreply;
            tail->content.origin.id = tail->content.origin.groupid;
            tail->content.origin.groupid++; //indication that this thread has no original post.
        }
        write(fd,&(tail->content),size);
		free(tail);
		tail=temp;
	}
    end_mmapfile((void *) ptr, buf.st_size, -1);
    ldata2.l_type = F_UNLCK;
    fcntl(fd2, F_SETLKW, &ldata2);
    close(fd2);

    ldata.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &ldata);        /* 退出互斥区域*/
    close(fd);
    return 0;
}

#endif //ifdef HAVE_WFORUM

#ifdef SMS_SUPPORT

int web_send_sms(char *dest,char *msgstr){
	struct userdata ud;
	char uident[STRLEN];
	char destid[STRLEN];
	bool cansend = true;
	struct userec *ur;
	int ret;
	char buf[MAX_MSG_SIZE];

	read_userdata(getCurrentUser()->userid, &ud);
	if(!ud.mobileregistered)
		return -1;

	if(!msgstr || !msgstr[0])
		return -3;

	sms_init_memory(getSession());
	getSession()->smsuin = u_info;

	if(isdigit(dest[0])){
		int i;
		cansend = cansend && (strlen(dest) == 11);
		for(i=0;i<strlen(dest);i++)
			cansend = cansend && (isdigit(dest[i]));
		if(cansend)
			strcpy(uident,dest);
	}else{
		struct userdata destud;
		return -2;
		getuser(dest, &ur);
		if(ur)
			strcpy(destid, ur->userid);
		if(read_userdata(destid, &destud))
			cansend = false;
		else
			cansend = destud.mobileregistered && (strlen(destud.mobilenumber)==11);
		if(cansend)
			strcpy(uident, destud.mobilenumber);
	}

	if(!cansend){
		shmdt(getSession()->head);
		return -2;
	}

	strncpy(buf, msgstr, MAX_MSG_SIZE);
	buf[MAX_MSG_SIZE-1]=0;

	if( strlen(buf) + strlen(ud.smsprefix) + strlen(ud.smsend) < MAX_MSG_SIZE ){
		int i,i1,j;

		i=strlen(buf);
		i1=strlen(ud.smsprefix);
		for(j= i+i1; j>=i1; j--){
			buf[j] = buf[j-i1];
		}
		for(j=0;j<i1;j++)
			buf[j] = ud.smsprefix[j];
		strcat(buf, ud.smsend);

	}

	ret = DoSendSMS(ud.mobilenumber, uident, buf,getSession());

	if( ret == CMD_ERR_SMS_VALIDATE_FAILED){
		if( read_user_memo(getCurrentUser()->userid, &getSession()->currentmemo) <= 0) return -1;
		ud.mobilenumber[0]=0;
		ud.mobileregistered=0;
		memcpy(&(getSession()->currentmemo->ud), &ud, sizeof(ud));
		end_mmapfile(getSession()->currentmemo, sizeof(struct usermemo), -1);
		write_userdata(getCurrentUser()->userid, &ud);
	}

	if( ret ){
		shmdt(getSession()->head);
		return 1;
	}else{
		struct msghead h;
		struct user_info *uin;
		h.frompid = u_info->pid;
		h.topid = -1;
		if( !isdigit(dest[0]) ){
			uin = t_search(destid, false);
			if(uin) h.topid = uin->pid;
			strcpy(h.id, destid);
		}else
			strcpy(h.id, uident);
		h.mode = 6;
		h.sent = 1;
		h.time = time(0);
		save_msgtext(getCurrentUser()->userid, &h, buf,getSession());
#if HAVE_MYSQL_SMTH == 1
        save_smsmsg(getCurrentUser()->userid, &h, buf, 1, getSession());
#endif
		if( !isdigit(dest[0]) ){
			h.sent = 0;
			strcpy(h.id, getCurrentUser()->userid);
			save_msgtext(destid, &h, buf,getSession());
#if HAVE_MYSQL_SMTH == 1
        	save_smsmsg(uident, &h, buf, 1, getSession());
#endif
			if(uin) kill(uin->pid, SIGUSR2);
		}
	}

	shmdt(getSession()->head);
	return 0;

}

int web_register_sms_sendcheck(char *mnumber)
{
    char ans[4];
    char valid[20];
    char buf2[80];
	struct userdata ud;
	int i;

	if( read_user_memo(getCurrentUser()->userid, &getSession()->currentmemo) <= 0) return -1;
	memcpy(&ud, &(getSession()->currentmemo->ud), sizeof(ud));

    sms_init_memory(getSession());
    getSession()->smsuin = u_info;

    if(ud.mobileregistered) {
		shmdt(getSession()->head);
        return -1;
    }

	if( mnumber == NULL ){
		shmdt(getSession()->head);
		return -2;
	}

	if( strlen(mnumber) != 11 ){
		shmdt(getSession()->head);
		return -3;
	}

	for(i=0;i <11; i++){
		if( ! isdigit( mnumber[i] ) ){
			shmdt(getSession()->head);
			return -4;
		}
	}

    if(DoReg(mnumber)) {
		shmdt(getSession()->head);
        return -5;
    }

	strcpy(ud.mobilenumber, mnumber);
	memcpy(&(getSession()->currentmemo->ud), &ud, sizeof(ud));
	end_mmapfile(getSession()->currentmemo, sizeof(struct usermemo), -1);
	write_userdata(getCurrentUser()->userid, &ud);
    
	shmdt(getSession()->head);
	return 0;
}

int web_register_sms_docheck(char *valid)
{
    char ans[4];
    char buf2[80];
	struct userdata ud;

	if( read_user_memo(getCurrentUser()->userid, &getSession()->currentmemo) <= 0) return -1;
	memcpy(&ud, &(getSession()->currentmemo->ud), sizeof(ud));

    sms_init_memory(getSession());
    getSession()->smsuin = u_info;

    if(ud.mobileregistered) {
		shmdt(getSession()->head);
        return -1;
    }

    if(! ud.mobilenumber[0] || strlen(ud.mobilenumber)!=11 ) {
		shmdt(getSession()->head);
		return -2;
    }

    if(valid == NULL || !valid[0]){
		shmdt(getSession()->head);
		return -3;
	}

    if(DoCheck(ud.mobilenumber, valid)) {
		shmdt(getSession()->head);
        return -4;
    }

    ud.mobileregistered = 1;
	memcpy(&(getSession()->currentmemo->ud), &ud, sizeof(ud));
	end_mmapfile(getSession()->currentmemo, sizeof(struct usermemo), -1);
    write_userdata(getCurrentUser()->userid, &ud);
    
	shmdt(getSession()->head);
	return 0;
}

int web_unregister_sms()
{
    char ans[4];
    char valid[20];
    char buf2[80];
    int rr;

	if( read_user_memo(getCurrentUser()->userid, &getSession()->currentmemo) <= 0) return -1;
    sms_init_memory(getSession());
    getSession()->smsuin = u_info;

    if(!getSession()->currentmemo->ud.mobileregistered) {
        shmdt(getSession()->head);
        getSession()->smsbuf=NULL;
        return -1;
    }

        rr = DoUnReg(getSession()->currentmemo->ud.mobilenumber,getSession());
        if(rr&&rr!=CMD_ERR_NO_SUCHMOBILE) {
            shmdt(getSession()->head);
	    	getSession()->currentmemo->ud.mobileregistered = 0;
	    	write_userdata(getCurrentUser()->userid, &(getSession()->currentmemo->ud));
			end_mmapfile(getSession()->currentmemo, sizeof(struct usermemo), -1);
            getSession()->smsbuf=NULL;
            return -1;
        }

        getSession()->currentmemo->ud.mobilenumber[0]=0;
        getSession()->currentmemo->ud.mobileregistered = 0;
        write_userdata(getCurrentUser()->userid, &(getSession()->currentmemo->ud));
		end_mmapfile(getSession()->currentmemo, sizeof(struct usermemo), -1);

    shmdt(getSession()->head);
    getSession()->smsbuf=NULL;

	return 0;
}

#endif
