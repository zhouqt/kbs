#include "bbs.h"
extern char *gb2big(char *, int *, int);
extern char *sysconf_str();

#include <libesmtp.h>
static int getmailnum(char* recmaildir)      /*Haohmaru.99.4.5.查对方信件数 */
{   
    struct fileheader fh;
    struct stat st;
    int fd;
    register int numfiles;
    
    if ((fd = open(recmaildir, O_RDONLY)) < 0)
        return 0;
    fstat(fd, &st);
    numfiles = st.st_size;
    numfiles = numfiles / sizeof(fh);
    close(fd);
    return numfiles;
}

int chkusermail(struct userec *user)
{
    char recmaildir[STRLEN];
    int sum, sumlimit, numlimit;

/*Arbitrator's mailbox has no limit, stephen 2001.11.1 */
#ifdef NINE_BUILD
    sumlimit = 10000;
    numlimit = 10000;
    setmailfile(recmaildir, user->userid, DOT_DIR);
    if (getmailnum(recmaildir) > numlimit || (sum = get_sum_records(recmaildir, sizeof(fileheader))) > sumlimit)
            return 1;
#else
    if ((!(user->userlevel & PERM_SYSOP)) && strcmp(user->userid, "Arbitrator")) {
        if (user->userlevel & PERM_CHATCLOAK) {
            sumlimit = 4000;
            numlimit = 4000;
        } else
            /*
             * if (lookupuser->userlevel & PERM_BOARDS)
             * set BM, chatop, and jury have bigger mailbox, stephen 2001.10.31 
             */
        if (user->userlevel & PERM_MANAGER) {
            sumlimit = 600;
            numlimit = 600;
        } else if (user->userlevel & PERM_LOGINOK) {
            sumlimit = 240;
            numlimit = 300;
        } else {
            sumlimit = 15;
            numlimit = 15;
        }
        setmailfile(recmaildir, user->userid, DOT_DIR);
        if (getmailnum(recmaildir) > numlimit || (sum = get_sum_records(recmaildir, sizeof(fileheader))) > sumlimit)
            return 1;
    }
#endif
    return 0;
}
int chkreceiver(struct userec *fromuser, struct userec *touser)

/*Haohmaru.99.4.4.检查收信者信箱是否满,改动下面的数字时请同时改动do_send do_gsend doforward doforward函数*/
{
    /*
     * Bigman 2000.9.8 : 修正没有用户的话,返回0 
     */
    /*
     * 修正PERM_SYSOP给自杀用户发信后的错误 
     */
    if (fromuser)
        if ((HAS_PERM(fromuser, PERM_SYSOP)) || (!strcmp(fromuser->userid, "Arbitrator")))
            /*
             * Leeward 99.07.28 , Bigman 2002.6.5: Arbitrator can send any mail to user 
             */
            return 0;
    if (touser->userlevel & PERM_SUICIDE)
        return 1;
    if (!(touser->userlevel & PERM_READMAIL))
        return 1;
    if (fromuser)
        if (chkusermail(fromuser))
            return 2;
    if (chkusermail(touser))
        return 3;
    return 0;
}
int check_query_mail(char qry_mail_dir[STRLEN])
{
    struct fileheader fh;
    struct stat st;
    int fd;
    register int offset;
    register long numfiles;
    unsigned char ch;

    offset = (int) ((char *) &(fh.accessed[0]) - (char *) &(fh));
    if ((fd = open(qry_mail_dir, O_RDONLY)) < 0)
        return 0;
    fstat(fd, &st);
    numfiles = st.st_size;
    numfiles = numfiles / sizeof(fh);
    if (numfiles <= 0) {
        close(fd);
        return 0;
    }
    lseek(fd, (st.st_size - (sizeof(fh) - offset)), SEEK_SET);
    /*
     * for(i = 0 ; i < numfiles ; i++) {
     * read(fd,&ch,1) ;
     * if(!(ch & FILE_READ)) {
     * close(fd) ;
     * return true ;
     * }
     * lseek(fd,-sizeof(fh)-1,SEEK_CUR);
     * } 
     */
    /*
     * 离线查询新信只要查询最後一封是否为新信，其他并不重要 
     */
    /*
     * Modify by SmallPig 
     */
    read(fd, &ch, 1);
    if (!(ch & FILE_READ)) {
        close(fd);
        return true;
    }
    close(fd);
    return false;
}
int mail_file(char *fromid, char *tmpfile, char *userid, char *title, int unlink)
{
    struct fileheader newmessage;
    struct stat st;
    char fname[STRLEN], filepath[STRLEN];
    char buf[255];
    int now;                    /* added for mail to SYSOP: Bigman 2000.8.11 */

    memset(&newmessage, 0, sizeof(newmessage));
    strcpy(buf, fromid);        /* Leeward 98.04.14 */
    strncpy(newmessage.owner, buf, OWNER_LEN);
    newmessage.owner[OWNER_LEN-1]=0;
    strncpy(newmessage.title, title, STRLEN);
    setmailpath(filepath, userid);
    if (stat(filepath, &st) == -1) {
        if (mkdir(filepath, 0755) == -1)
            return -1;
    } else {
        if (!(st.st_mode & S_IFDIR))
            return -1;
    }
    now = time(NULL);
    /*
     * setmailpath(filepath, userid, fname); 
     */
    setmailpath(filepath, userid);
    get_postfilename(fname, filepath);
    strcpy(newmessage.filename, fname);
    setmailfile(filepath, userid, fname);
    /*
     * sprintf(genbuf, "cp %s %s",tmpfile, filepath) ;
     */
    if (unlink)
        f_mv(tmpfile, filepath);
    else
        f_cp(tmpfile, filepath, 0);
    setmailfile(buf, userid, DOT_DIR);
    if (append_record(buf, &newmessage, sizeof(newmessage)) == -1)
        return -1;
    newbbslog(LOG_USER, "mailed %s ", userid);
    if (!strcasecmp(userid, "SYSOP"))
        updatelastpost("sysmail");
    return 0;
}

char *email_domain()
{
    char *domain;

    domain = sysconf_str("BBSDOMAIN");
    if (domain == NULL)
        domain = "unknown.BBSDOMAIN";
    /*
     * domain = MAIL_BBSDOMAIN; 
     */
    return domain;
}
struct mail_option {
    FILE *fin;
    int isbig5;
    int noansi;
    int bfirst;
};
void monitor_cb(const char *buf, int buflen, int writing, void *arg)
{
    FILE *fp = arg;

    if (writing == SMTP_CB_HEADERS) {
        fputs("H: ", fp);
        fwrite(buf, 1, buflen, fp);
        return;
    }
    fputs(writing ? "C >>>>\n" : "S <<<<\n", fp);
    fwrite(buf, 1, buflen, fp);
    if (buf[buflen - 1] != '\n')
        putc('\n', fp);
}
char *bbs_readmailfile(char **buf, int *len, void *arg)
{
#define MAILBUFLEN	8192
    struct mail_option *pmo = (struct mail_option *) arg;
    char *retbuf;
    char *p, *pout;
    int i;
    char getbuf[MAILBUFLEN / 2];

    if (*buf == NULL)
        *buf = malloc(MAILBUFLEN);
    if (len == NULL) {
        rewind(pmo->fin);
        pmo->bfirst = 1;
        return NULL;
    }
    *len = fread(getbuf, 1, MAILBUFLEN / 2, pmo->fin);
    if (pmo->isbig5) {
        retbuf = gb2big(getbuf, len, 1);
    } else {
        retbuf = getbuf;
    }
    p = retbuf;
    pout = *buf;
    if (pmo->bfirst) {

/*	sprintf(pout,"Reply-To: %s.bbs@%s\r\n\r\n", currentuser->userid, email_domain());
*/
        if (pmo->isbig5)
            sprintf(pout, "%s", "MIME-Version: 1.0\r\nContent-Type: text/plain; charset=big5\r\nContent-Transfer-Encoding: 8bit\r\n\r\n");
        else
            sprintf(pout, "%s", "MIME-Version: 1.0\r\nContent-Type: text/plain; charset=gb2312\r\nContent-Transfer-Encoding: 8bit\r\n\r\n");
        pout = *buf + strlen(*buf);
        pmo->bfirst = 0;
    }
    for (i = 0; i < *len; i++) {
        if ((*p == '\n') && ((i == 0) || (*(p - 1) != '\r'))) {
            *pout = '\r';
            pout++;
        }
        *pout = *p;
        pout++;
        p++;
    }
    *len = pout - (*buf);
    retbuf = *buf;
    if (pmo->noansi) {
        char *p1, *p2;
        int esc;

        p1 = retbuf;
        p2 = retbuf;
        esc = 0;
        for (i = 0; i < *len; i++, p1++) {
            if (esc) {
                if (*p1 == '\033') {
                    esc = 0;
                    *p2 = *p1;
                    p2++;
                } else if (isalpha(*p1))
                    esc = 0;
            } else {
                if (*p1 == '\033') {
                    esc = 1;
                } else {
                    *p2 = *p1;
                    p2++;
                }
            }
        }
        *p2 = 0;
        *len = p2 - retbuf;
    };
    return retbuf;
#undef MAILBUFLEN
}

/* Callback to prnt the recipient status */
void print_recipient_status(smtp_recipient_t recipient, const char *mailbox, void *arg)
{
    const smtp_status_t *status;

    status = smtp_recipient_status(recipient);
#ifdef BBSMAIN
    prints("mail to %s: %d %s\n", mailbox, status->code, status->text);
#endif
}
int bbs_sendmail(char *fname, char *title, char *receiver, int isuu, int isbig5, int noansi)
{                               /* Modified by ming, 96.10.9  KCN,99.12.16 */
    struct mail_option mo;
    FILE *fin;
    char uname[STRLEN];
    int len;
    smtp_session_t session;
    smtp_message_t message;
    smtp_recipient_t recipient;
    const smtp_status_t *status;
    enum notify_flags notify = Notify_NOTSET;
    char *server;
    char newbuf[257];

    if (isuu) {
        char buf[256];

        sprintf(uname, "tmp/uu%05d", getpid());
        sprintf(buf, "uuencode %s thbbs.%05d > %s", fname, getpid(), uname);
        system(buf);
    }
    if ((fin = fopen(isuu ? uname : fname, "r")) == NULL) {
#ifdef BBSMAIN
        prints("can't open %s: %s\n", isuu ? uname : fname, strerror(errno));
#endif
        return -1;
    }
    session = smtp_create_session();
    message = smtp_add_message(session);

/*
    if ((fout = fopen ("tmp/maillog", "w+")) == NULL)
    {
      prints("can't open %s: %s\n", "tmp/maillog", strerror (errno));
      return -1;
    }
    smtp_set_monitorcb (session, monitor_cb, fout, 1);
*/
    server = sysconf_str("MAILSERVER");
    /*
     * server = MAIL_MAILSERVER; 
     */
    if ((server == NULL) || !strcmp(server, "(null ptr)"))
        server = "127.0.0.1:25";
    smtp_set_server(session, server);
    sprintf(newbuf, "%s@%s", currentuser->userid, email_domain());
    smtp_set_reverse_path(message, newbuf);
    smtp_set_header(message, "Message-Id", NULL);
    if (isbig5) {
        strcpy(newbuf, title);
        len = strlen(title);
        smtp_set_header(message, "Subject", gb2big(title, &len, 1));
    } else {
        smtp_set_header(message, "Subject", title);
    }
    smtp_set_header_option(message, "Subject", Hdr_OVERRIDE, 3);
    /*
     * smtp_8bitmime_set_body(message, E8bitmime_8BITMIME); 
     */
    mo.isbig5 = isbig5;
    mo.noansi = noansi;
    mo.fin = fin;
    mo.bfirst = 1;
    smtp_set_messagecb(message, (smtp_messagecb_t) bbs_readmailfile, (void *) &mo);
    recipient = smtp_add_recipient(message, receiver);
    if (notify != Notify_NOTSET)
        smtp_dsn_set_notify(recipient, notify);
    /*
     * Initiate a connection to the SMTP server and transfer the
     * message. 
     */
    smtp_start_session(session);
    status = smtp_message_transfer_status(message);
#ifdef BBSMAIN
    prints("return code:%d(%s)\n", status->code, status->text);
#endif                          /* 
                                 */
    smtp_enumerate_recipients(message, print_recipient_status, NULL);
    /*
     * Free resources consumed by the program.
     */
    smtp_destroy_session(session);
    fclose(fin);
    if (isuu)
        unlink(uname);
    return (status->code != 250);
}
