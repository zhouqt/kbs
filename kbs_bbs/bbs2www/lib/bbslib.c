/*
 * $Id$
 */
#include "types.h"
#include "bbslib.h"
#include <assert.h>

time_t update_time = 0;
int showexplain = 0, freshmode = 0;
int mailmode, numf;
int friendmode = 0;
int usercounter, real_user_names = 0;
int range, page, readplan, num;

struct user_info *user_record[USHM_SIZE];
struct userec *user_data;

time_t set_idle_time(struct user_info * uentp, time_t t);
int loginok = 0;
friends_t bbb[MAXREJECTS];
int badnum = 0;

struct user_info *u_info;

/*struct UTMPFILE *shm_utmp;*/
/*struct UCACHE *shm_ucache;*/
char fromhost[IPLEN + 1];
char parm_name[256][80], *parm_val[256];
int parm_num = 0;

struct favbrd_struct {
    int flag;
    char *title;
    int father;
};

extern struct favbrd_struct favbrd_list[FAVBOARDNUM];
extern int favbrd_list_t;
extern int favnow;

friends_t fff[200];

int friendnum = 0;


int file_has_word(char *file, char *word)
{
    FILE *fp;
    char buf[256], buf2[256];

    fp = fopen(file, "r");
    if (fp == 0)
        return 0;
    while (1) {
        bzero(buf, 256);
        if (fgets(buf, 255, fp) == 0)
            break;
        sscanf(buf, "%s", buf2);
        if (!strcasecmp(buf2, word)) {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

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

char *noansi(char *s)
{
    static char buf[1024];
    int i = 0, mode = 0;

    while (s[0] && i < 1023) {
        if (mode == 0) {
            if (s[0] == 27) {
                mode = 1;
            } else {
                buf[i] = s[0];
                i++;
            }
        } else {
            if (!strchr(";[0123456789", s[0]))
                mode = 0;
        }
        s++;
    }
    buf[i] = 0;
    return buf;
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

char *strright(char *s, int len)
{
    int l = strlen(s);

    if (len <= 0)
        return "";
    if (len >= l)
        return s;
    return s + (l - len);
}

#ifndef FREEBSD
char *strcasestr(char *s1, char *s2)
{
    int l;

    l = strlen(s2);
    while (s1[0]) {
        if (!strncasecmp(s1, s2, l))
            return s1;
        s1++;
    }
    return 0;
}
#endif                          /* not FREEBSD */

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
    printf("´íÎó! %s! <br><br>\n", buf);
    printf("<a href=\"javascript:history.go(-1)\">¿ìËÙ·µ»Ø</a>");
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
    case 1:                    /* default color, big font */
        printf("<link rel=\"stylesheet\" type=\"text/css\" href=\"%s\">\n", CSS_FILE_BIGFONT);
        printf("<link rel=\"stylesheet\" type=\"text/css\" href=\"/ansi-web-middle.css\">\n");
        break;
    case 0:
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

/* smh_init() ÐèÒªÐÞ¸Ä */
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
    int uid, key;

    strsncpy(id, getparm("UTMPUSERID"), 13);
    strsncpy(num, getparm("UTMPNUM"), 12);
    /*
     * printf("utmpuserid = %s\n", id); 
     */
    /*
     * printf("utmpnum = %s\n", num); 
     */
    key = atoi(getparm("UTMPKEY"));
    utmpent = atoi(num);
    if (id[0] == '\0')
        return -1;

    if (www_user_init(utmpent, id, key, x, y, 0) == 0 && strcasecmp("guest", currentuser->userid))
        return 1;
    return 0;
}

int del_mail(int ent, struct fileheader *fh, char *direct)
{
    char buf[PATHLEN];
    char *t;
    char genbuf[PATHLEN];
    struct stat st;

    if (strstr(direct, ".DELETED")) {
        strcpy(buf, direct);
        t = strrchr(buf, '/') + 1;
        strcpy(t, fh->filename);
		if (lstat(buf, &st) == 0 && S_ISREG(st.st_mode) && st.st_nlink == 1)
            currentuser->usedspace -= st.st_size;
    }

    strcpy(buf, direct);
    if ((t = strrchr(buf, '/')) != NULL)
        *t = '\0';
    if (!delete_record(direct, sizeof(*fh), ent, (RECORD_FUNC_ARG) cmpname, fh->filename)) {
        sprintf(genbuf, "%s/%s", buf, fh->filename);
        if (strstr(direct, ".DELETED"))
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

int post_mail(char *userid, char *title, char *file, char *id, char *nickname, char *ip, int sig)
{
    FILE *fp, *fp2;
    char buf3[256], dir[256];
    struct fileheader header;
    struct stat st;
    struct userec *touser;      /*peregrine for updating used space */
    int unum;
    int t, i;

    bzero(&header, sizeof(header));
    strcpy(header.owner, id);
    for (i = 0; i < 100; i++) {
        t = time(0) + i;
        sprintf(buf3, "mail/%c/%s/M.%d.A", toupper(userid[0]), userid, i + time(0));
        if (!file_exist(buf3))
            break;
    }
    if (i >= 99)
        return -1;

    if (false == canIsend2(currentuser, userid)) {
        return -2;
    }

    unum = getuser(userid, &touser);
    if (!HAS_PERM(currentuser, PERM_SYSOP) && chkusermail(touser)) {    /*Haohamru.99.4.05 */
        return -3;
    }
    sprintf(header.filename, "M.%d.A", t);
    strsncpy(header.title, title, 60);
    fp = fopen(buf3, "w");
    if (fp == NULL)
        return -4;
    fp2 = fopen(file, "r");
    fprintf(fp, "¼ÄÐÅÈË: %s (%s)\n", id, nickname);
    fprintf(fp, "±ê  Ìâ: %s\n", title);
    fprintf(fp, "·¢ÐÅÕ¾: %s (%s)\n", BBSNAME, wwwCTime(time(0)));
    fprintf(fp, "À´  Ô´: %s\n\n", ip);
    if (fp2) {
        while (fgets(buf3, 256, fp2) != NULL) {
            fprintf2(fp, buf3);
        }
        fclose(fp2);
    }
    fprintf(fp, "\n--\n");
    sig_append(fp, id, sig);
    fprintf(fp, "\n[1;%dm¡ù À´Ô´:£®%s %s£®[FROM: %.20s][m\n", 31 + rand() % 7, BBSNAME, NAME_BBS_ENGLISH, ip);
    fclose(fp);
    sprintf(buf3, "mail/%c/%s/%s", toupper(userid[0]), userid, header.filename); /*ft.buf3 have changed.added by binxun.*/
    if (stat(buf3, &st) != -1)
        touser->usedspace += st.st_size;
    sprintf(dir, "mail/%c/%s/.DIR", toupper(userid[0]), userid);
    fp = fopen(dir, "a");
    if (fp == NULL)
        return -5;
    fwrite(&header, sizeof(header), 1, fp);
    fclose(fp);
	setmailcheck(userid);
    
   /* Ìí¼ÓLog Bigman: 2003.4.7 */
    newbbslog(BBSLOG_USER, "mailed(www) %s %s", userid,title);

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

    color = (user->numlogins % 7) + 31; /* ÑÕÉ«Ëæ»ú±ä»¯ */
    sethomefile(fname, currentuser->userid, "signatures");
    if ((fp2 = fopen(fname, "r")) == NULL ||    /* ÅÐ¶ÏÊÇ·ñÒÑ¾­ ´æÔÚ Ç©Ãûµµ */
        user->signature == 0 || anony == 1) {
        fputs("\n--\n", fp);
    } else {                    /*Bigman 2000.8.10ÐÞ¸Ä,¼õÉÙ´úÂë */
        fprintf(fp, "\n");
    }
    /*
     * ÓÉBigmanÔö¼Ó:2000.8.10 Announce°æÄäÃû·¢ÎÄÎÊÌâ 
     */
    if (!strcmp(board, "Announce"))
        fprintf(fp, "[m[%2dm¡ù À´Ô´:¡¤%s http://%s¡¤[FROM: %s][m\n", color, BBS_FULL_NAME, BBS_FULL_NAME);
    else
        fprintf(fp, "\n[m[%2dm¡ù À´Ô´:¡¤%s http://%s¡¤[FROM: %s][m\n", color, BBS_FULL_NAME, NAME_BBS_ENGLISH, (anony) ? NAME_ANONYMOUS_FROM : user->lasthost);

    if (fp2)
        fclose(fp2);
    return;
}

void write_header2(FILE * fp, char *board, char *title, char *userid, char *username, int anony)
{
    if (!strcmp(board, "Announce"))
        fprintf(fp, "·¢ÐÅÈË: %s (%s), ÐÅÇø: %s\n", "SYSOP", NAME_SYSOP, board);
    else
        fprintf(fp, "·¢ÐÅÈË: %s (%s), ÐÅÇø: %s\n", anony ? board : userid, anony ? NAME_ANONYMOUS : username, board);
    fprintf(fp, "±ê  Ìâ: %s\n·¢ÐÅÕ¾: %s (%24.24s)\n\n", title, "BBS " NAME_BBS_CHINESE "Õ¾", Ctime(time(0)));
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
        ptr = strrchr(buf3, '\r');
        if (ptr != NULL) {
            if (*(ptr + 1) == '\n') {
                *ptr = '\n';
                *(ptr + 1) = '\0';
            }
        }
        fprintf2(fp, buf3);
    }
}

/* return value:
   >0		success
   -1		write .DIR failed*/
int post_article(char *board, char *title, char *file, struct userec *user, char *ip, int sig, int local_save, int anony, struct fileheader *oldx, char *attach_dir)
{
    struct fileheader post_file;
    char filepath[MAXPATH], fname[STRLEN];
    char buf[256];
    int fd, anonyboard;
	int retvalue;
    FILE *fp, *fp2;

    memset(&post_file, 0, sizeof(post_file));
    anonyboard = anonymousboard(board); /* ÊÇ·ñÎªÄäÃû°æ */

    /*
     * ×Ô¶¯Éú³É POST ÎÄ¼þÃû 
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
    write_header2(fp, board, title, user->userid, user->username, anony);
#endif
    write_file2(fp, fp2);
    fclose(fp2);
    if (!anony)
        addsignature(fp, user, sig);
#ifndef RAW_ARTICLE
    add_loginfo2(fp, board, user, anony);       /*Ìí¼Ó×îºóÒ»ÐÐ */
#endif

    strncpy(post_file.title, title, STRLEN);
    if (local_save == 1) {      /* local save */
        post_file.innflag[1] = 'L';
        post_file.innflag[0] = 'L';
    } else {
        post_file.innflag[1] = 'S';
        post_file.innflag[0] = 'S';
        outgo_post2(&post_file, board, user->userid, user->username, title);
    }

    setbfile(buf, board, DOT_DIR);

    /*
     * ÔÚboards°æ°æÖ÷·¢ÎÄ×Ô¶¯Ìí¼ÓÎÄÕÂ±ê¼Ç Bigman:2000.8.12 
     */
    if (!strcmp(board, "Board") && !HAS_PERM(currentuser, PERM_OBOARDS)
        && HAS_PERM(currentuser, PERM_BOARDS)) {
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
	retvalue = after_post(currentuser, &post_file, board, oldx);

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
                long size, begin, save_size;
                char *ptr;

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
                    if (safe_mmapfile_handle(fd, O_RDONLY, PROT_READ, MAP_SHARED, (void **) &ptr, (size_t *) & size) == 0) {
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
     * Ìø¹ýÇ°ÃæµÄ (sig - 1)*6 ÐÐ  
     */
    do {
        if (skip_lines == (sig - 1) * 6)
            break;
        skip_lines++;
    } while ((fgets(buf, sizeof(buf), fp2)) != NULL);
    /*
     * ¼ì²éÊÇ·ñ¿ÉÒÔ¶ÁÈëµÚ sig ¸öÇ©Ãûµµ 
     */
    if (skip_lines == (sig - 1) * 6) {
        /*
         * ¶ÁÈëÇ©Ãûµµ²¢Ð´Èë fp ¶ÔÓ¦µÄÎÄ¼þÖÐ 
         */
        for (i = skip_lines; i < skip_lines + 6; i++) {
            if (fgets(buf, sizeof(buf), fp2) == NULL)
                break;
            unix_string(buf);
            if (buf[0] != '\n')
                fprintf(fp, "%s", buf);
        }
        /*
         * ¶ÁÈëÇ©Ãûµµ³É¹¦£¬ÉèÖÃÄ¬ÈÏÇ©ÃûµµÎªµ±Ç°Ê¹ÓÃµÄÇ©Ãûµµ 
         */
        if (i > skip_lines)
            x->signature = sig;
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

int count_mails(char *id, int *total, int *unread)
{
    struct fileheader x1;
    char buf[256];
    int n;
    FILE *fp;

    *total = 0;
    *unread = 0;
    {
        struct userec *x;

        if (getuser(id, &x) == 0)
            return 0;
    }
    sprintf(buf, "%s/mail/%c/%s/.DIR", BBSHOME, toupper(id[0]), id);
    fp = fopen(buf, "r");
    if (fp == 0)
        return -1;
    while (fread(&x1, sizeof(x1), 1, fp) > 0) {
        (*total)++;
        if (!(x1.accessed[0] & FILE_READ))
            (*unread)++;
    }
    fclose(fp);
    return 0;
}

int setmsgfile(char *buf, char *id)
{
    if (buf == NULL || id == NULL)
        return -1;
    sethomefile(buf, id, SYS_MSGFILE);
    return 0;
}

int setmsgfilelog(char *buf, char *id)
{
    if (buf == NULL || id == NULL)
        return -1;
    sethomefile(buf, id, SYS_MSGFILELOG);
    return 0;
}

extern char MsgDesUid[14];
int send_msg(char *srcid, int srcutmp, char *destid, int destutmp, char *msg)
{
    int i;
    uinfo_t *uin;

    /*
     * ÂËµôÌØÊâ×Ö·û£¬Ó¦¸ÃÐ´³ÉÒ»¸öº¯Êý 
     */
    for (i = 0; i < (int) strlen(msg); i++)
        if ((0 < msg[i] && msg[i] <= 27) || msg[i] == -1)
            msg[i] = 32;
    if (destutmp == 0)
        uin = t_search(destid, destutmp);
    else
        uin = get_utmpent(destutmp);
    if (uin == NULL)
        return -1;
    if (strcasecmp(uin->userid, destid))
        return -1;
    strcpy(MsgDesUid, uin->userid);
    return sendmsgfunc(uin, msg, 2);
}

int count_life_value(struct userec *urec)
{
    int i;

    i = (time(0) - urec->lastlogin) / 60;
    if (urec->userlevel & PERM_SPECIAL8)
        return (360 * 24 * 60 - i) / 1440;
    if ((urec->userlevel & PERM_XEMPT) || !strcasecmp(urec->userid, "guest"))
        return 999;
    if (urec->numlogins <= 3)
        return (45 * 1440 - i) / 1440;
    if (!(urec->userlevel & PERM_LOGINOK))
        return (90 * 1440 - i) / 1440;
    return (180 * 1440 - i) / 1440;
}

int modify_mode(struct user_info *x, int newmode)
{
    if (x == 0)
        return;
    x->mode = newmode;
}

int save_user_data(struct userec *x)
{
    int n;

    n = getusernum(x->userid);
    update_user(x, n, 1);
    return 1;
}

int is_bansite(char *ip)
{
    FILE *fp;
    char buf3[256];

    fp = fopen(".bansite", "r");
    if (fp == 0)
        return 0;
    while (fscanf(fp, "%s", buf3) > 0)
        if (!strcasecmp(buf3, ip))
            return 1;
    fclose(fp);
    return 0;
}

int user_perm(struct userec *x, int level)
{
    return (x->userlevel & level);
}

int getusernum(char *id)
{
    return searchuser(id);
}

int loadfriend(char *id)
{
    FILE *fp;
    char file[256];

    if (!loginok)
        return;
    sprintf(file, "home/%c/%s/friends", toupper(id[0]), id);
    fp = fopen(file, "r");
    if (fp) {
        friendnum = fread(fff, sizeof(fff[0]), 200, fp);
        fclose(fp);
    }
}

int isfriend(char *id)
{
    return myfriend(searchuser(id), NULL);
}

int loadbad(char *id)
{
    FILE *fp;
    char file[256];

    if (!loginok)
        return;
    sprintf(file, "home/%c/%s/rejects", toupper(id[0]), id);
    fp = fopen(file, "r");
    if (fp) {
        badnum = fread(fff, sizeof(fff[0]), MAXREJECTS, fp);
        fclose(fp);
    }
}

int isbad(char *id)
{
    static inited = 0;
    int n;

    if (!inited) {
        loadbad(currentuser->userid);
        inited = 1;
    }
    for (n = 0; n < badnum; n++)
        if (!strcasecmp(id, bbb[n].id))
            return 1;
    return 0;
}

void http_redirect(char *url)
{
    printf("Status: 302 Found\n");
    printf("Location: %s\n", url);
    printf("Content-type: text/html; charset=%s\n\n", CHARSET);
}

int init_all()
{
    srand(time(0) * 2 + getpid());
    chdir(BBSHOME);
    http_init();
    /*
     * seteuid(BBSUID);
     * if(geteuid()!=BBSUID) http_fatal("uid error."); 
     */
    shm_init();
    loginok = user_init(&currentuser, &u_info);
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

int init_no_http()
{
    srand(time(0) + getpid());
    chdir(BBSHOME);
    shm_init();
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

char *sec(char c)
{
    int i;

    for (i = 0; i < SECNUM; i++) {
        if (strchr(seccode[i], c))
            return secname[i][0];
    }
    return "(unknown.)";
}

char *flag_str(int access)
{
    static char buf[80];
    char *flag2 = "";

    strcpy(buf, "  ");
    if (access & FILE_DIGEST)
        flag2 = "G";
    if (access & FILE_MARKED)
        flag2 = "M";
    if ((access & FILE_MARKED) && (access & FILE_DIGEST))
        flag2 = "B";
    sprintf(buf, "%s", flag2);
    return buf;
}

char *flag_str2(int access, int has_read)
{
    static char buf[80];

    strcpy(buf, "   ");
    if (loginok)
        strcpy(buf, "N  ");
    if (access & FILE_DIGEST)
        buf[0] = 'G';
    if (access & FILE_MARKED)
        buf[0] = 'M';
    if ((access & FILE_MARKED) && (access & FILE_DIGEST))
        buf[0] = 'B';
    if (has_read)
        buf[0] = tolower(buf[0]);
    if (buf[0] == 'n')
        buf[0] = ' ';
    return buf;
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
            sprintf(buf2, "<a href=\"bbsqry?userid=%s\">%s</a> ", ptr, ptr);
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

int set_my_cookie()
{
    FILE *fp;
    char path[256], buf[256], buf1[256], buf2[256];
    int my_t_lines = 20, my_link_mode = 0, my_def_mode = 0;

    sprintf(path, "home/%c/%s/.mywww", toupper(currentuser->userid[0]), currentuser->userid);
    fp = fopen(path, "r");
    if (fp) {
        while (1) {
            if (fgets(buf, 80, fp) == 0)
                break;
            if (sscanf(buf, "%80s %80s", buf1, buf2) != 2)
                continue;
            if (!strcmp(buf1, "t_lines"))
                my_t_lines = atoi(buf2);
            if (!strcmp(buf1, "link_mode"))
                my_link_mode = atoi(buf2);
            if (!strcmp(buf1, "def_mode"))
                my_def_mode = atoi(buf2);
        }
        fclose(fp);
    }
    sprintf(buf, "%d", my_t_lines);
    setcookie("my_t_lines", buf);
    sprintf(buf, "%d", my_link_mode);
    setcookie("my_link_mode", buf);
    sprintf(buf, "%d", my_def_mode);
    setcookie("my_def_mode", buf);
}

int has_fill_form()
{
    FILE *fp;
    int r;
    char userid[256], tmp[256], buf[256], *ptr;

    fp = fopen("new_register", "r");
    if (fp == 0)
        return 0;
    while (1) {
        if (fgets(buf, 100, fp) == 0)
            break;
        r = sscanf(buf, "%s %s", tmp, userid);
        if (r == 2) {
            if (!strcasecmp(tmp, "userid:") && !strcasecmp(userid, currentuser->userid)) {
                fclose(fp);
                return 1;
            }
        }
    }
    fclose(fp);
    return 0;
}


/* added by flyriver, 2001.12.17
 * using getcurrusr() instead of using currentuser directly
 */
struct userec *getcurrusr()
{
    return currentuser;
}

struct userec *setcurrusr(struct userec *user)
{
    if (user == NULL)
        return NULL;
    currentuser = user;
    return currentuser;
}

char *getcurruserid()
{
    return currentuser->userid;
}

unsigned int getcurrulevel()
{
    return currentuser->userlevel;
}

int define(unsigned int x)
{
    return x ? currentuser->userdefine & x : 1;
}

time_t get_idle_time(struct user_info * uentp)
{
    return uentp->freshtime;
}


bcache_t *getbcacheaddr()
{
    return bcache;
}

uinfo_t *getcurruinfo()
{
    return u_info;
}

uinfo_t *setcurruinfo(uinfo_t * ui)
{
    u_info = ui;

    return u_info;
}

/* should do some locking before calling this function */
int eat_file_content(int fd, off_t start, off_t len)
{
    struct stat fs;
    char *addr;
    int i, r;

    if (start < 0 || len <= 0)
        return 1;
    if (fstat(fd, &fs) < 0)
        return -1;
    if (start >= fs.st_size)
        return 1;
    addr = mmap(0, fs.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED)
        return -1;
    if (start + len > fs.st_size)
        len = fs.st_size - start;
    r = fs.st_size - start - len;
    for (i = start; i < r; i++)
        addr[start + i] = addr[start + len + i];
    munmap(addr, fs.st_size);
    ftruncate(fd, fs.st_size - len);

    return 0;
}

int count_online()
{
    return get_utmp_number();
}

int count_www()
{
    int i, total = 0;

    for (i = 0; i < MAXACTIVE; i++) {
        if (utmpshm->uinfo[i].mode == WEBEXPLORE)
            total++;
    }
    return total;
}


int get_ulist_length()
{
    return sizeof(user_record) / sizeof(user_record[0]);
}

struct user_info **get_ulist_addr()
{
    return user_record;
}

uinfo_t *get_user_info(int utmpnum)
{

    if (utmpnum < 1 || utmpnum > USHM_SIZE)
        return NULL;
    return &(utmpshm->uinfo[utmpnum - 1]);
}

int get_friends_num()
{
    return get_utmpent(utmpent)->friendsnum;
    ;
}

struct boardheader *getbcache_addr()
{
    return bcache;
}

/* from talk.c */
int cmpfuid(a, b)
struct friends *a, *b;
{
    return strcasecmp(a->id, b->id);
}

/* from bbs.c */
static void record_exit_time(char *userid)
{                               /* ¼ÇÂ¼ÀëÏßÊ±¼ä  Luzi 1998/10/23 */
    char path[80];
    FILE *fp;
    time_t now;

    sethomefile(path, userid, "exit");
    fp = fopen(path, "wb");
    if (fp != NULL) {
        now = time(NULL);
        fwrite(&now, sizeof(time_t), 1, fp);
        fclose(fp);
    }
}

/* from list.c */

int set_friendmode(int mode)
{
    friendmode = mode;

    return friendmode;
}

int get_friendmode()
{
    return friendmode;
}

int full_utmp(struct user_info *uentp, int *count)
{
    if (!uentp->active || !uentp->pid) {
        return 0;
    }
    if (!HAS_PERM(currentuser, PERM_SEECLOAK) && uentp->invisible && strcmp(uentp->userid, getcurruserid())) {  /*Haohmaru.99.4.24.ÈÃÒþÉíÕßÄÜ¿´¼û×Ô¼º */
        return 0;
    }
    if (friendmode && !myfriend(uentp->uid, NULL)) {
        return 0;
    }
    user_record[*count] = uentp;
    (*count)++;
    return COUNT;
}

int fill_userlist()
{
    static int i, i2;
    struct user_info *u;

    i2 = 0;
    if (!friendmode) {
        apply_ulist_addr((APPLY_UTMP_FUNC) full_utmp, (char *) &i2);
    } else {
        u = get_utmpent(utmpent);
        for (i = 0; i < u->friendsnum; i++) {
            if (u->friends_uid[i])
                apply_utmpuid((APPLY_UTMP_FUNC) full_utmp, u->friends_uid[i], (char *) &i2);
        }
    }
    range = i2;
    return i2 == 0 ? -1 : 1;
}

int countusers(struct userec *uentp, char *arg)
{
    char permstr[10];

    if (uentp->numlogins != 0 && uleveltochar(permstr, uentp) != 0)
        return COUNT;
    return 0;
}

int allusers()
{
    int count;

    if ((count = apply_users(countusers, 0)) <= 0) {
        return 0;
    }
    return count;
}

char *get_favboard(int k)
{
    int i, j = 0;

    for (i = 0; i < favbrd_list_t; i++)
        if (favbrd_list[i].father == favnow) {
            if (j == k) {
                if (favbrd_list[i].flag == -1)
                    return favbrd_list[i].title;
                else {
                    struct boardheader const*bptr;

                    bptr = getboard(favbrd_list[i].flag+1);
                    return (char*)bptr->filename;
                }
            }
            j++;
        }
    return NULL;
}

int get_favboard_id(int k)
{
    int i, j = 0;

    for (i = 0; i < favbrd_list_t; i++)
        if (favbrd_list[i].father == favnow) {
            if (j == k)
                return i;
            j++;
        }
    return NULL;
}

int get_favboard_type(int k)
{
    int i, j = 0;

    for (i = 0; i < favbrd_list_t; i++)
        if (favbrd_list[i].father == favnow) {
            if (j == k) {
                if (favbrd_list[i].flag == -1)
                    return 0;
                else {
                    return 1;
                }
            }
            j++;
        }
    return 0;
}

int add_favboard(char *brdname)
{
    int i;

    if (brdname != NULL && *brdname)
        i = getbnum(brdname);
    else
        return -3;              /* err brdname */
    i--;
    addFavBoard(i);
}

/* from bbsfadd.c */
/*int addtooverride2(char *uident, char *exp)
{
    friends_t tmp;
    int  n;
    char buf[STRLEN];

    memset(&tmp,0,sizeof(tmp));
    setuserfile( buf,currentuser->userid, "friends" );
    if((!HAS_PERM(currentuser,PERM_ACCOUNTS) && !HAS_PERM(currentuser,PERM_SYSOP)) &&
            (get_num_records(buf,sizeof(struct friends))>=MAXFRIENDS) )
    {
        return -1;
    }
    if( myfriend( searchuser(uident) , NULL) )
        return -2;
	if (exp == NULL || exp[0] == '\0')
		tmp.exp[0] = '\0';
	else
	{
		strncpy(tmp.exp, exp, sizeof(tmp.exp)-1);
		tmp.exp[sizeof(tmp.exp)-1] = '\0';
	}
    n=append_record(buf,&tmp,sizeof(struct friends));
    if(n!=-1)
        getfriendstr();
    else
        return -3;
    return n;
}*/

/*
int post_file(char *filename, postinfo_t *pi)
{
	FILE *fp;
	FILE *fp2;
    struct fileheader pf;
	char filepath[STRLEN];
	char buf[STRLEN];

	memset(&pf, 0, sizeof(pf));
	if (get_unifile(buf, pi->board, 0) == -1)
		return -1;
	setbfile(filepath, pi->board, buf);
	strcpy(pf.filename, buf);
	strncpy(pf.owner, pi->userid, STRLEN);
	strncpy(pf.title, pi->title, STRLEN);
	pf.accessed[0] = pi->access & 0xFF;
	pf.accessed[1] = (pi->access >> 8) & 0xFF;
	fp = fopen(filepath, "w");
	fp2 = fopen(filename, "r");	
	write_header2(fp, pi->board, pi->title, pi->userid, pi->username,
		   	pi->anony);
	write_file2(fp, fp2);
	fclose(fp2);
	fclose(fp);
    if ( pi->local == 1 )
	{
		pf.filename[ STRLEN - 1 ] = 'L';
		pf.filename[ STRLEN - 2 ] = 'L';
	}
    else
	{
		pf.filename[ STRLEN - 1 ] = 'S';
		pf.filename[ STRLEN - 2 ] = 'S';
		outgo_post2(&pf, pi->board, pi->userid, pi->username, pi->title);
	}
	setbfile(filepath, pi->board, DOT_DIR);
	if (append_record(filepath, &pf, sizeof(pf)) == -1)
   	{
		sprintf(buf, "post file '%s' on '%s': append_record failed!",
				pf.title, pi->board);
		report(buf);
		return -1;
	}
	sprintf(buf,"posted file '%s' on '%s'", pf.title, pi->board);
	report(buf);

	return 0;
}
*/
/*
 * get_unifile() ÓÃÓÚ»ñµÃÒ»¸ö¶ÀÒ»ÎÞ¶þµÄÎÄ¼þ.
 *     filename  ÎÄ¼þÃû»º³åÇø
 *     key       ¹Ø¼ü×Ö, ¿ÉÒÔÊÇ°æÃû»òÕßÊÇÓÃ»§Ãû
 *     mode      == 0, ±íÊ¾ filename ÊÇ·¢±íÎÄÕÂÓÃµÄÎÄ¼þÃû
 *                     ´ËÊ± key ÊÇ°æÃû
 *               == 1, ±íÊ¾ filename ÊÇÍ¶µÝÓÊ¼þÓÃµÄÎÄ¼þÃû
 *                     ´ËÊ± key ÊÇÓÃ»§Ãû
 */
int get_unifile(char *filename, char *key, int mode)
{
    int fd;
    char *ip;
    char filepath[STRLEN];
    int now;                    /* added for mail to SYSOP: Bigman 2000.8.11 */

    now = time(NULL);
    sprintf(filename, "M.%d.A", now);
    if (mode == 0) {
        if (getbcache(key) == NULL)
            return -1;
        setbfile(filepath, key, filename);
    } else {
        if (searchuser(key) == 0)
            return -1;
        setmailfile(filepath, key, filename);
    }
    ip = strrchr(filename, 'A');
    while ((fd = open(filepath, O_CREAT | O_EXCL | O_WRONLY, 0644)) == -1) {
        if (*ip == 'Z')
            ip++, *ip = 'A', *(ip + 1) = '\0';
        else
            (*ip)++;
        if (mode == 0)
            setbfile(filepath, key, filename);
        else
            setmailfile(filepath, key, filename);
    }
    close(fd);

    return 0;
}

/* »ñµÃÒ»¸öÓÃ»§µÄÔÚÏßÓÃ»§ÊýÄ¿ */
int count_user_online(char *uid)
{
    return apply_utmp(NULL, 0, uid, 0);
}

/* »ñµÃµ±Ç°ÓÃ»§µÄ utmp ºÅ */
int get_curr_utmpent()
{
    return get_utmpent_num(u_info);
}


/* ÒÔÏÂµÄ´úÂëÊÇcgiºÍphp¶¼Ê¹ÓÃµÄ*/
static struct user_info www_guest_uinfo;

static int www_new_guest_entry()
{
    struct public_data *pub;
    int fd, i, j;
    time_t now;
    struct userec *user;

    fd = www_guest_lock();
    if (fd == -1)
        return -1;
    setpublicshmreadonly(0);
    pub = get_publicshm();
    if (pub->www_guest_count >= MAX_WWW_GUEST) {
        setpublicshmreadonly(1);
        return -1;
    }
    user = currentuser;
    getuser("guest", &currentuser);

    if (currentuser == NULL)
        return NULL;
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
             * Çå³ýuse_map 
             */
	    guestinfo.currentboard=wwwguest_shm->guest_entry[i].currentboard;
	    do_after_logout(currentuser, &guestinfo, i, 1);

            wwwguest_shm->use_map[i / 32] &= ~(1 << (i % 32));
            if (pub->www_guest_count > 0) {
                pub->www_guest_count--;
                /*
                 * Çå³ýÊý¾Ý 
                 */
                bzero(&wwwguest_shm->guest_entry[i], sizeof(struct WWW_GUEST_S));
            }
        }
    }
    for (i = 0; i < MAX_WWW_MAP_ITEM; i++)
        if (wwwguest_shm->use_map[i] != 0xFFFFFFFF) {
            int map = wwwguest_shm->use_map[i];

            for (j = 0; j < 32; j++)
                if ((map & 1) == 0) {
                    wwwguest_shm->use_map[i] |= 1 << j;
                    wwwguest_shm->guest_entry[i * 32 + j].freshtime = time(0);
                    /*
                     * ±ÜÃâ±»kickÏÂÈ¥ 
                     */
                    break;
                } else
                    map = map >> 1;
            break;
        }
    if (i != MAX_WWW_MAP_ITEM) {
        pub->www_guest_count++;
        if (get_utmp_number() + getwwwguestcount() > get_publicshm()->max_user) {
            save_maxuser();
        }
    }
    currentuser = user;
    setpublicshmreadonly(1);
    www_guest_unlock(fd);
    if (i == MAX_WWW_MAP_ITEM)
        return -1;
    return i * 32 + j;
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
    do_after_logout(currentuser, &guestinfo, idx, 1);
    setpublicshmreadonly(0);
    pub = get_publicshm();
    fd = www_guest_lock();
    if (wwwguest_shm->use_map[idx / 32] & (1 << (idx % 32))) {
        wwwguest_shm->use_map[idx / 32] &= ~(1 << (idx % 32));
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
     * www_guest_infoÄ¿Ç°ÏÈÊ¹ÓÃÒ»¸öÈ«¾Ö±äÁ¿À´×ö£¬ÕâÑù
     * »áµ¼ÖÂÏß³Ì²»°²È«:P   µ«ÊÇ¶ÔÓÚ½ø³ÌÄ£ÐÍµÄcgi ºÍphp
     * ×ã¹»ÁË
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
     * destuid ½«±»ÓÃÀ´´æ·Åwww guest±íµÄÈë¿Ú 
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
         * ·ÇguestÔÚÏßÓÃ»§´¦Àí 
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
	    init_brc_cache((*x)->userid,true);
#endif
#endif
    } else {
        /*
         * guestÓÃ»§´¦Àí 
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

int www_user_login(struct userec *user, int useridx, int kick_multi, char *fromhost, char *fullfrom, struct user_info **ppuinfo, int *putmpent)
{
    int ret;
    char buf[255];
    struct userdata ud;
    char genbuf[STRLEN];
    struct userec* uc;

    if (user != NULL && strcasecmp(user->userid, "guest")) {
        struct user_info ui;
        int utmpent;
        time_t t;
        int multi_ret = 1;
        int tmp;

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
				if(auto_register(user->userid,ud.realemail,STRLEN)==0)user->userlevel |= PERM_DEFAULT;
		 }
		 read_userdata(user->userid,&ud);
	    }
#endif		  
	    
        }

        memset(&ui, 0, sizeof(struct user_info));
        ui.active = true;
        /*
         * Bigman 2000.8.29 ÖÇÄÒÍÅÄÜ¹»ÒþÉí
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
            if (addto_msglist(utmpent, user->userid) < 0) {
                bbslog("3system", "can't add msg:%d %s!!!\n", utmpent, user->userid);
                *ppuinfo = u;
                *putmpent = utmpent;
                ret = 2;
            } else {
                *ppuinfo = u;
                *putmpent = utmpent;
                ret = 0;
            }
            getfriendstr(currentuser, u);
            do_after_login(currentuser,utmpent,0);
        }
    } else {
        /*
         * TODO:alloc guest table 
         */
        int idx = www_new_guest_entry();

        if (idx < 0)
            ret = 5;
        else {
            int tmp = rand() % 100000000;

            wwwguest_shm->guest_entry[idx].key = tmp;
            wwwguest_shm->guest_entry[idx].logintime = time(0);
            www_guest_uinfo.logintime = wwwguest_shm->guest_entry[idx].logintime;

            wwwguest_shm->guest_entry[idx].freshtime = time(0);
            www_guest_uinfo.freshtime = wwwguest_shm->guest_entry[idx].freshtime;

            www_guest_uinfo.destuid = idx;
            www_guest_uinfo.utmpkey = tmp;
            *ppuinfo = &www_guest_uinfo;
            *putmpent = idx;
            getuser("guest", &currentuser);
            ret = 0;
            do_after_login(currentuser,idx,1);
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
    struct userec *x = NULL;

    stay = abs(time(0) - puinfo->logintime);
    /*
     * ÉÏÕ¾Ê±¼ä³¬¹ý 2 Ð¡Ê±°´ 2 Ð¡Ê±¼Æ 
     */
    if (stay > 7200)
        stay = 7200;
    user->stay += stay;
    record_exit_time(user->userid);
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
    if (HAS_PERM(currentuser, PERM_CHAT))
        return 1;
    else
        return 0;
}

int can_send_mail()
{
    if (HAS_PERM(currentuser, PERM_DENYMAIL))
        return 0;
    else if (HAS_PERM(currentuser, PERM_LOGINOK)) {
        if (chkusermail(currentuser))
            return 0;
        return 1;
    } else
        return 0;
}

int can_reply_post(char *board, char *filename)
{
    char dirpath[STRLEN];
    struct fileheader fh;
    int pos;

    if ((!strcmp(board, "News")) || (!strcmp(board, "Original")))
        return 0;
    setbdir(0, dirpath, board);
    pos = search_record(dirpath, &fh, sizeof(fh), (RECORD_FUNC_ARG) cmpname, filename);
    if (pos <= 0)
        return 0;
    if (fh.accessed[1] & FILE_READ)
        return 0;
    else
        return 1;
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

/*
 * buflen is a value-result variable. When it is passed to the function,
 * its value is the buffer length (including the trailing '\0' character).
 * When the function returned, its value is the number of characters 
 * actually copied to buf (excluding the trailing '\0' character).
*/
char *string_copy(char *buf, const char *str, size_t * buflen)
{
    size_t i;
    size_t len;

    if (*buflen == 0)
        return buf;
    len = *buflen - 1;
    for (i = 0; i < len; i++) {
        if (str[i] == '\0') {
            buf[i] = str[i];
            break;
        }
        buf[i] = str[i];
    }
    *buflen = i;
    buf[i] = '\0';

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

int is_BM(struct boardheader *board, struct userec *user)
{
    char BM[STRLEN];

    strncpy(BM, board->BM, sizeof(BM) - 1);
    BM[sizeof(BM) - 1] = '\0';
    return chk_currBM(BM, user);
}

int is_owner(struct fileheader *fh, struct userec *user)
{
    if (!strcmp(fh->owner, user->userid))
        return 1;
    else
        return 0;
}

int can_delete_post(struct boardheader *board, struct fileheader *fh, struct userec *user)
{
    if (is_BM(board, user) || is_owner(fh, user))
        return 1;
    else
        return 0;
}

int can_edit_post(struct boardheader *board, struct fileheader *fh, struct userec *user)
{
    if (is_BM(board, user) || is_owner(fh, user))
        return 1;
    else
        return 0;
}

int get_seccode_index(char prefix)
{
    int i;

    for (i = 0; i < SECNUM; i++) {
        if (strchr(seccode[i], prefix) != NULL)
            return i;
    }
    return -1;
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
    output->output(font_str, strlen(font_str), output);
}

static void html_output(char *buf, size_t buflen, buffered_output_t * output)
{
    size_t i;

    for (i = 0; i < buflen; i++) {
        switch (buf[i]) {
        case '&':
            output->output("&amp;", 5, output);
            break;
        case '<':
            output->output("&lt;", 4, output);
            break;
        case '>':
            output->output("&gt;", 4, output);
            break;
        case ' ':
            output->output("&nbsp;", 6, output);
            break;
        default:
            output->output(&buf[i], 1, output);
        }
    }
}

static void print_raw_ansi(char *buf, size_t buflen, buffered_output_t * output)
{
    size_t i;

    for (i = 0; i < buflen; i++) {
        if (buf[i] == 0x1b)
            html_output("*", 1, output);
        else if (buf[i]=='\n') {
			output->output(" <br /> ", 8, output);
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

void output_ansi_html(char *buf, size_t buflen, buffered_output_t * output,char* attachlink)
{
    unsigned int font_style = 0;
    unsigned int ansi_state;
    unsigned int ansi_val[STRLEN];
    int ival = 0;
    size_t i;
    char *ptr = buf;
    char *ansi_begin;
    char *ansi_end;
	char *ubbstart_begin,*ubbmiddle_begin, *ubbfinish_begin;
    int attachmatched;
    char link[256];
	long attachPos[MAXATTACHMENTCOUNT];
	long attachLen[MAXATTACHMENTCOUNT];
	char* attachFileName[MAXATTACHMENTCOUNT];
	enum ATTACHMENTTYPE attachType[MAXATTACHMENTCOUNT];
	int attachShowed[MAXATTACHMENTCOUNT];
	char UBBCode[256];	
	int UBBCodeLen;
	enum UBBTYPE UBBCodeType;
	int isUBBMiddleOutput; 
	int UBBArg1, UBBArg2, UBBArg3;
	char UBBStrArg[256];


    if (ptr == NULL)
        return;
    STATE_ZERO(ansi_state);
    bzero(ansi_val, sizeof(ansi_val));
    bzero(attachShowed, sizeof(attachShowed));
    attachmatched = 0;
	for (i = 0; i < buflen ; i++ ){
        long attach_len;
        char *attachptr, *attachfilename;
		if (attachmatched>=MAXATTACHMENTCOUNT)	{
			break;
		}
        if (attachlink&&((attachfilename = checkattach(buf + i, buflen - i, &attach_len, &attachptr)) != NULL)) {
            char *extension;
            int type;
            extension = attachfilename + strlen(attachfilename);
			i+=(attachptr-buf-i)+attach_len-1;
			if (i>buflen) continue;
			attachPos[attachmatched]=attachfilename-buf;
			attachLen[attachmatched]=attach_len;
			attachFileName[attachmatched]=(char*)malloc(256);
			strncpy(attachFileName[attachmatched],attachfilename,255);
			attachFileName[attachmatched][255]=0;
			attachType[attachmatched]=ATTACH_OTHERS;
			extension--;
            while ((*extension != '.') && (*extension != NULL))
                extension--;
            if (*extension == '.') {
                extension++;
                if (!strcasecmp(extension, "bmp") || !strcasecmp(extension, "jpg")
                    || !strcasecmp(extension, "png") || !strcasecmp(extension, "jpeg")
                    || !strcasecmp(extension, "pcx") || !strcasecmp(extension, "gif"))
                    attachType[attachmatched]=ATTACH_IMG;
                else if (!strcasecmp(extension, "swf"))
                    attachType[attachmatched] = ATTACH_FLASH;
            }
			attachmatched++;
		}
	}

    for (i = 0; i < buflen; i++) {
        long attach_len;
        char *attachptr, *attachfilename;

		/* skip attachments */
        if (attachlink&&((attachfilename = checkattach(buf + i, buflen - i, &attach_len, &attachptr)) != NULL)) {
		    i=(attachptr-buf)+attach_len-1;
			continue;
        }
		
        if (STATE_ISSET(ansi_state, STATE_NEW_LINE)) {
            STATE_CLR(ansi_state, STATE_NEW_LINE);
            if (i < (buflen - 1) && (buf[i] == ':' && buf[i + 1] == ' ')) {
                STATE_SET(ansi_state, STATE_QUOTE_LINE);
                if (STATE_ISSET(ansi_state, STATE_FONT_SET))
                    output->output("</font>", 7, output);
                /*
                 * set quoted line styles 
                 */
                STYLE_SET(font_style, FONT_STYLE_QUOTE);
                STYLE_SET_FG(font_style, FONT_COLOR_QUOTE);
                STYLE_CLR_BG(font_style);
                print_font_style(font_style, output);
                output->output(&buf[i], 1, output);
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
		if ( buf[i] == '[' )	{ //UBB¿ØÖÆ´úÂë¿ªÊ¼?
			if (STATE_ISSET(ansi_state, STATE_UBB_START)){
				size_t len;
				STATE_CLR(ansi_state, STATE_UBB_START);
				len=(&(buf[i]))-ubbstart_begin;
				print_raw_ansi(ubbstart_begin,len,output);
			}
			if (STATE_ISSET(ansi_state, STATE_UBB_END))
			{
				STATE_CLR(ansi_state, STATE_UBB_END);
				STATE_SET(ansi_state, STATE_UBB_MIDDLE);
			}

			if ( (i < (buflen-1) ) && (buf[i + 1] == '/') )	{ //UBB´úÂë½áÊø?
				if (!STATE_ISSET(ansi_state, STATE_UBB_MIDDLE)){
					print_raw_ansi(&buf[i], 1, output);
					continue;
				}
				STATE_CLR(ansi_state,STATE_UBB_MIDDLE);
				STATE_SET(ansi_state,STATE_UBB_END);
				ubbfinish_begin=&buf[i];
				i++;
			} else {
				if (STATE_ISSET(ansi_state,STATE_UBB_MIDDLE | STATE_UBB_START | STATE_UBB_END)){
					size_t len;
					len=(&(buf[i]))-ubbstart_begin;
					print_raw_ansi(ubbstart_begin,len,output);
					STATE_CLR(ansi_state, STATE_UBB_MIDDLE | STATE_UBB_START | STATE_UBB_END);
				}
				ubbstart_begin=&buf[i];
				STATE_SET(ansi_state,STATE_UBB_START);
			}
			UBBCodeLen=0;
			continue;

		} else if ( buf[i] == ']' )	{ //UBB¿ØÖÆ´úÂë½áÊø?
			if (STATE_ISSET(ansi_state, STATE_UBB_START))	{
				int num;
				num=0;
				UBBCode[UBBCodeLen]=0;
				isUBBMiddleOutput=1;
				sscanf(UBBCode, "upload=%d",&num);
				if (num>0) {
					UBBArg1=num;
					UBBCodeType=UBB_TYPE_ATTACH;
					isUBBMiddleOutput=0;
				} else {
					size_t len;
					STATE_CLR(ansi_state, STATE_UBB_START);
					len=(&(buf[i+1]))-ubbstart_begin;
					print_raw_ansi(ubbstart_begin,len,output);
					continue;
				}
				STATE_CLR(ansi_state, STATE_UBB_START);
				STATE_SET(ansi_state, STATE_UBB_MIDDLE);
				ubbmiddle_begin=&buf[i+1];
				continue;
			} else if (STATE_ISSET(ansi_state, STATE_UBB_END))	{
				size_t len;
				UBBCode[UBBCodeLen]=0;		
				STATE_CLR(ansi_state, STATE_UBB_END);
				switch (UBBCodeType){
				case UBB_TYPE_ATTACH:
					if (!strcasecmp(UBBCode,"upload")){
						if ( (UBBArg1>0) && (UBBArg1<=attachmatched)) {
							char outbuf[512];
							switch(attachType[UBBArg1-1]) {
							case ATTACH_IMG:
								snprintf(outbuf, 511, "<br><IMG SRC=\"images/files/img.gif\" border=0>´ËÖ÷ÌâÏà¹ØÍ¼Æ¬ÈçÏÂ£º<br><A HREF=\"%s&ap=%d\" TARGET=_blank><IMG SRC=\"%s&ap=%d\" border=0 alt=°´´ËÔÚÐÂ´°¿Úä¯ÀÀÍ¼Æ¬ onload=\"javascript:if(this.width>screen.width-333)this.width=screen.width-333\"></A> ", attachlink, attachPos[UBBArg1-1], attachlink, attachPos[UBBArg1-1]);
								break;
							case ATTACH_FLASH:
				                snprintf(outbuf, 511, "<br>Flash¶¯»­: " "<a href='%s&ap=%d'>%s</a> (%d ×Ö½Ú)<br>" "<OBJECT classid=clsid:D27CDB6E-AE6D-11cf-96B8-444553540000 codebase=http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=5,0,0,0 > <PARAM NAME='MOVIE' VALUE='%s&ap=%d'>" "<EMBED SRC='%s&ap=%d'></EMBED></OBJECT><br />", attachlink, attachPos[UBBArg1-1], attachFileName[UBBArg1-1], attachLen[UBBArg1-1], attachlink, attachPos[UBBArg1-1], attachlink, attachPos[UBBArg1-1]);
								break;
							case ATTACH_OTHERS:
								 snprintf(outbuf, 511, "<br>¸½¼þ: <a href='%s&ap=%d'>%s</a> (%d ×Ö½Ú)<br />", attachlink, attachPos[UBBArg1-1], attachFileName[UBBArg1-1], attachLen[UBBArg1-1]);
								 break;
							}	
							outbuf[511]=0;
							output->output(outbuf, 511, output);
							attachShowed[UBBArg1-1]=1;
							continue;							
						}	
					} 
					break;
				}
				STATE_SET(ansi_state, STATE_UBB_MIDDLE);
			} 
		}
		if (i < (buflen - 1) && (buf[i] == 0x1b && buf[i + 1] == '[')) {
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
                output->output("</font>", 7, output);
                STYLE_CLR(font_style, FONT_STYLE_QUOTE);
                STATE_CLR(ansi_state, STATE_FONT_SET);
            }
		    if (!STATE_ISSET(ansi_state,STATE_UBB_MIDDLE) || isUBBMiddleOutput) {
				output->output(" <br /> ", 8, output);
			}
            STATE_CLR(ansi_state, STATE_QUOTE_LINE);
            STATE_SET(ansi_state, STATE_NEW_LINE);
        } else {
			if (STATE_ISSET(ansi_state, STATE_UBB_START|STATE_UBB_END))	{
				if (UBBCodeLen>100)	{
					if (STATE_ISSET(ansi_state, STATE_UBB_START)){
						size_t len;
						len=(&(buf[i+1]))-ubbstart_begin;
						print_raw_ansi(ubbstart_begin,len,output);
					}
					if (STATE_ISSET(ansi_state, STATE_UBB_END)){
						size_t len;
						len=(&(buf[i+1]))-ubbfinish_begin;
						print_raw_ansi(ubbfinish_begin,len,output);
					}
					STATE_CLR(ansi_state, STATE_UBB_START | STATE_UBB_END);
					continue;
				}
				UBBCode[UBBCodeLen]=buf[i];
				UBBCodeLen++;
			} else if (STATE_ISSET(ansi_state, STATE_UBB_MIDDLE)){
				if (isUBBMiddleOutput)	{
	                		print_raw_ansi(&buf[i], 1, output);
				}
			} else if (STATE_ISSET(ansi_state, STATE_ESC_SET)) {
                if (buf[i] == 'm') {
                    /*
                     *[0;1;4;31m */
                    if (STATE_ISSET(ansi_state, STATE_FONT_SET)) {
                        output->output("</font>", 7, output);
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
        output->output("</font>", 7, output);
        STATE_CLR(ansi_state, STATE_FONT_SET);
    }
	for ( i = 0; i<attachmatched ; i++ ){
		if (!attachShowed[i]) { 
			char outbuf[512];
			switch(attachType[i]) {
			case ATTACH_IMG:
		 		snprintf(outbuf, 511, "<br><IMG SRC=\"images/files/img.gif\" border=0>´ËÖ÷ÌâÏà¹ØÍ¼Æ¬ÈçÏÂ£º<br><A HREF=\"%s&ap=%d\" TARGET=_blank><IMG SRC=\"%s&ap=%d\" border=0 alt=°´´ËÔÚÐÂ´°¿Úä¯ÀÀÍ¼Æ¬ onload=\"javascript:if(this.width>screen.width-333)this.width=screen.width-333\"></A> ",attachlink, attachPos[i],attachlink, attachPos[i]);
				break;
			case ATTACH_FLASH:
		        snprintf(outbuf, 511, "<br>Flash¶¯»­: " "<a href='%s&ap=%d'>%s</a> (%d ×Ö½Ú)<br>" "<OBJECT classid=clsid:D27CDB6E-AE6D-11cf-96B8-444553540000 codebase=http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=5,0,0,0 > <PARAM NAME='MOVIE' VALUE='%s&ap=%d'>" "<EMBED SRC='%s&ap=%d'></EMBED></OBJECT><br />", attachlink, attachPos[i], attachFileName[i], attachLen[i], attachlink, attachPos[i], attachlink, attachPos[i]);
				break;
			case ATTACH_OTHERS:
				 snprintf(outbuf, 511, "<br>¸½¼þ: <a href='%s&ap=%d'>%s</a> (%d ×Ö½Ú)<br />", attachlink, attachPos[i], attachFileName[i], attachLen[i]);
				 break;
			}	
			outbuf[511]=0;
			output->output(outbuf, 511, output);
			attachShowed[i]=1;
		}
		free(attachFileName[i]);
	}

    output->flush(output);

}

/* ent ÊÇ 1-based µÄ*/
int del_post(int ent, struct fileheader *fileinfo, char *direct, char *board)
{
    FILE *fn;
    char buf[512];
    char usrid[STRLEN];
    char *t;
    int owned, fail;
    struct userec *user;
    char bm_str[BM_LEN - 1];
    struct boardheader *bp;

    user = currentuser;
    bp = getbcache(board);
    memcpy(bm_str, bp->BM, BM_LEN - 1);
    if (!strcmp(board, "syssecurity")
        || !strcmp(board, "junk")
        || !strcmp(board, "deleted"))   /* Leeward : 98.01.22 */
        return DONOTHING;

    if (fileinfo->owner[0] == '-') {
        return FULLUPDATE;
    }
    owned = isowner(user, fileinfo);
    /* change by KCN  ! strcmp( fileinfo->owner, currentuser->userid ); */
    strcpy(usrid, fileinfo->owner);
    if (!(owned) && !HAS_PERM(currentuser, PERM_SYSOP))
        if (!chk_currBM(bm_str, currentuser)) {
            return DONOTHING;
        }
    if (do_del_post(currentuser, ent, fileinfo, direct, board, 0, 1) != 0)
        return FULLUPDATE;
    return DIRCHANGED;

}

unsigned int binarySearchInFileHeader(struct fileheader *start,int total, unsigned int key){
	unsigned int low, high ,mid, found;
	int comp;
	low = 0;
	high = total - 1;
	found=-1;
	while (low <= high) {
		mid = (high + low) / 2;
		comp = (key) - (start[mid].id);
		if (comp == 0) {
			found=mid;
			break;
		} else if (comp < 0)
			high = mid - 1;
		else
			low = mid + 1;
		if (high<low)
			break;
	}
	return found;
}

//ÒÔºó¸Ä³É¸üÓÐÐ§ÂÊµÄËã·¨°É¡­¡­
typedef struct _wwwthreadheader_list{
	struct wwwthreadheader content;
	struct _wwwthreadheader_list *previous;
} wwwthreadheader_list, *pwwwthreadheader_list;


static pwwwthreadheader_list foundInWWWThreadList(unsigned int groupid, pwwwthreadheader_list p){
	while (p!=NULL) {
		if (p->content.origin.groupid==groupid)	{
			return p;
		}
		p=p->previous;
	}
	return NULL;
}

static pwwwthreadheader_list CreateNewWWWThreadListNode(pwwwthreadheader_list p){
	pwwwthreadheader_list q;
	q=(pwwwthreadheader_list)malloc(sizeof(wwwthreadheader_list));
	if (q!=NULL){
		q->previous=p;
	}
	return q;
}

static void clearWWWThreadList(pwwwthreadheader_list p){
	pwwwthreadheader_list q;
	while (p!=NULL) {
		q=p->previous;
		free(p);
		p=q;
	}
}

int www_generateOriginIndex(char* board)
/* added by roy 2003.7.17 generate .ORIGIN index file*/
{
    struct fileheader *ptr1,*ptrtemp;
    struct flock ldata, ldata2 , ldata3;
    int fd, fd2, fd3,  size , total3, total, i, count = 0;
    char olddirect[PATHLEN];
	char currdirect[PATHLEN];
	char dingdir[PATHLEN];
    char *ptr,*ptr3;
    struct stat buf,buf3;
	pwwwthreadheader_list tail,temp;
    size_t bm_search[256];
	int found;

    setbdir(DIR_MODE_NORMAL, olddirect, board);
    setbdir(DIR_MODE_WEB_THREAD, currdirect, board);
	setbdir(DIR_MODE_ZHIDING, dingdir, board);
    if ((fd = open(currdirect, O_WRONLY | O_CREAT, 0664)) == -1) {
        bbslog("user", "%s", "recopen err");
        return -1;      /* ´´½¨ÎÄ¼þ·¢Éú´íÎó*/
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
    /* ¿ªÊ¼»¥³â¹ý³Ì*/

    if ((fd2 = open(olddirect, O_RDONLY, 0664)) == -1) {
        bbslog("user", "%s", "recopen err");
        ldata.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW, &ldata);
        close(fd);
        return -3;
    }
    fstat(fd2, &buf);
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
    total = buf.st_size /sizeof(fileheader);

    if ((i = safe_mmapfile_handle(fd2, O_RDONLY, PROT_READ, MAP_SHARED, (void **) &ptr, (size_t*)&buf.st_size)) != 1) {
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

	tail=temp=NULL;


    if ((fd3 = open(dingdir, O_RDONLY, 0664)) != -1) {
		fstat(fd3, &buf3);
		ldata3.l_type = F_RDLCK;
		ldata3.l_whence = 0;
		ldata3.l_len = 0;
		ldata3.l_start = 0;
		if (fcntl(fd3, F_SETLKW, &ldata3) != -1) {
			total3 = buf3.st_size / sizeof(fileheader);
			if (total3>MAX_DING)
				total3=MAX_DING;

			if ((i = safe_mmapfile_handle(fd3, O_RDONLY, PROT_READ, MAP_SHARED, (void **) &ptr3, (size_t*)&buf3.st_size)) == 1) {
				ptr1 = (struct fileheader *) ptr3;
				ptrtemp = (struct fileheader *) ptr;

				for (i=total3-1;i>=0;i--) {
					if (ptr1[i].groupid!=ptr1[i].id) continue;
					if (foundInWWWThreadList(ptr1[i].groupid,tail)!=NULL) continue;
					temp=CreateNewWWWThreadListNode(tail);
					if (temp==NULL) {
						clearWWWThreadList(tail);
						return -5;
					}
					temp->content.origin=ptr1[i];
					temp->content.lastreply=ptr1[i];
					temp->content.articlecount=1;
					temp->content.flags=FILE_ON_TOP;
					temp->content.unused=0;
					tail=temp;
					count++;
				}
			    end_mmapfile((void *) ptr3, buf3.st_size, -1);
			} else if (i == 2)
				end_mmapfile((void *) ptr3, buf3.st_size, -1);
			ldata3.l_type = F_UNLCK;
			fcntl(fd3, F_SETLKW, &ldata3);	
		} 
		close(fd3);
    }


    ptr1 = (struct fileheader *) ptr;

	for (i=total-1;i>=0;i--) {
		temp=foundInWWWThreadList(ptr1[i].groupid,tail);
		if (temp==NULL)	{
			if ((found=binarySearchInFileHeader(ptr1,total,ptr1[i].groupid))==-1) continue;
			temp=CreateNewWWWThreadListNode(tail);
			if (temp==NULL) {
				clearWWWThreadList(tail);
				return -5;
			}
			temp->content.origin=ptr1[found];
			temp->content.lastreply=ptr1[i];
			temp->content.articlecount=1;
			temp->content.flags=0;
			temp->content.unused=0;
			count++;
			tail=temp;
			if(count>=50000) 
				break;
		} else {
			if (temp->content.lastreply.groupid==temp->content.lastreply.id) {
				temp->content.lastreply=ptr1[i];
			} else {
				temp->content.articlecount++;
			}
		}
	}

	while (tail!=NULL) {
		temp=tail->previous;
		write(fd,&(tail->content),size);
		free(tail);
		tail=temp;
	}
    end_mmapfile((void *) ptr, buf.st_size, -1);
    ldata2.l_type = F_UNLCK;
    fcntl(fd2, F_SETLKW, &ldata2);
    close(fd2);
    ftruncate(fd, count * size);

    ldata.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &ldata);        /* ÍË³ö»¥³âÇøÓò*/
    close(fd);
    return 0;
}

#ifdef SMS_SUPPORT

extern struct user_info * smsuin;

int web_send_sms(char *dest,char *msgstr){
	struct userdata ud;
	char uident[STRLEN];
	char destid[STRLEN];
	bool cansend = true;
	struct userec *ur;
	int ret;

	read_userdata(currentuser->userid, &ud);
	if(!ud.mobileregistered)
		return -1;

	if(!msgstr || !msgstr[0])
		return -3;

	sms_init_memory();
	smsuin = u_info;

	if(isdigit(dest[0])){
		int i;
		cansend = cansend && (strlen(dest) == 11);
		for(i=0;i<strlen(dest);i++)
			cansend = cansend && (isdigit(dest[i]));
		if(cansend)
			strcpy(uident,dest);
	}else{
		struct userdata destud;
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

	if(!cansend)
		return -2;

	ret = DoSendSMS(ud.mobilenumber, uident, msgstr);

	if( ret == CMD_ERR_SMS_VALIDATE_FAILED){
		if( read_user_memo(currentuser->userid, &currentmemo) < 0) return -1;
		ud.mobilenumber[0]=0;
		ud.mobileregistered=0;
		memcpy(&(currentmemo->ud), &ud, sizeof(ud));
		end_mmapfile(currentmemo, sizeof(struct usermemo), -1);
		write_userdata(currentuser->userid, &ud);
	}

	if( ret ){
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
		save_msgtext(currentuser->userid, &h, msgstr);
		if( !isdigit(dest[0]) ){
			h.sent = 0;
			strcpy(h.id, currentuser->userid);
			save_msgtext(destid, &h, msgstr);
			if(uin) kill(uin->pid, SIGUSR2);
		}
	}

	return 0;

}

int web_register_sms_sendcheck(char *mnumber)
{
    char ans[4];
    char valid[20];
    char buf2[80];
	struct userdata ud;
	int i;

	if( read_user_memo(currentuser->userid, &currentmemo) < 0) return -1;
	memcpy(&ud, &(currentmemo->ud), sizeof(ud));

    sms_init_memory();
    smsuin = u_info;

    if(ud.mobileregistered) {
		shmdt(head);
        return -1;
    }

	if( mnumber == NULL ){
		shmdt(head);
		return -2;
	}

	if( strlen(mnumber) != 11 ){
		shmdt(head);
		return -3;
	}

	for(i=0;i <11; i++){
		if( ! isdigit( mnumber[i] ) ){
			shmdt(head);
			return -4;
		}
	}

    if(DoReg(mnumber)) {
		shmdt(head);
        return -5;
    }

	strcpy(ud.mobilenumber, mnumber);
	memcpy(&(currentmemo->ud), &ud, sizeof(ud));
	end_mmapfile(currentmemo, sizeof(struct usermemo), -1);
	write_userdata(currentuser->userid, &ud);
    
	shmdt(head);
	return 0;
}

int web_register_sms_docheck(char *valid)
{
    char ans[4];
    char buf2[80];
	struct userdata ud;

	if( read_user_memo(currentuser->userid, &currentmemo) < 0) return -1;
	memcpy(&ud, &(currentmemo->ud), sizeof(ud));

    sms_init_memory();
    smsuin = u_info;

    if(ud.mobileregistered) {
		shmdt(head);
        return -1;
    }

    if(! ud.mobilenumber[0] || strlen(ud.mobilenumber)!=11 ) {
		shmdt(head);
		return -2;
    }

    if(valid == NULL || !valid[0]){
		shmdt(head);
		return -3;
	}

    if(DoCheck(ud.mobilenumber, valid)) {
		shmdt(head);
        return -4;
    }

    ud.mobileregistered = 1;
	memcpy(&(currentmemo->ud), &ud, sizeof(ud));
	end_mmapfile(currentmemo, sizeof(struct usermemo), -1);
    write_userdata(currentuser->userid, &ud);
    
	shmdt(head);
	return 0;
}

#endif
