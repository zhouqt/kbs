#include "bbs.h"

#include <libesmtp.h>

struct mail_option{
    FILE* fin;
    int isbig5;
    int noansi;
};

char *
bbs_readmailfile (char **buf, int *len, void *arg)
{
#define MAILBUFLEN	8192
    struct mail_option* pmo=(struct mail_option*)arg;
    
    if (*buf == NULL)
    *buf = malloc (MAILBUFLEN);
    
    char* retbuf;
    
    if (len == NULL)
    {
      rewind (pmo->fin);
      return NULL;
    }

    *len = fread(*buf, 1, BUFLEN, pmo->fin);

    if (pmo->isbig5) {
        retbuf = gb2big(*buf,len,1);
    } else {
        retbuf=*buf;
    }

    if (pmo->noansi) {
        char *p1,*p2;
        p1=retbuf;
        p2=retbuf;
        esc=0;
        for (;*p1;p1++) {
            if (esc) {
                if (*p1=='\033') {
                    esc=0;
                    *p2=*p1;
                    p2++;
                }else
                    if (isalpha(*p1))
                        esc=0;
            } else {
                if (*p1=='\033') {
                    esc=1;
                } else {
                    *p2=*p1;
                    p2++;
                }
            }
        }
    
        *p2=0;
    };
    return retbuf;
#undef MAILBUFLEN
}

char *email_domain()
{
    char        *domain;

    domain = sysconf_str( "BBSDOMAIN" );
    if( domain == NULL )  domain = "unknown.BBSDOMAIN";
    return domain;
}

/* Callback to prnt the recipient status */
void
print_recipient_status (smtp_recipient_t recipient,
			const char *mailbox, void *arg)
{
  const smtp_status_t *status;

  status = smtp_recipient_status (recipient);
  prints("mail to %s: %d %s", mailbox, status->code, status->text);
}

int
bbs_sendmail(fname, title, receiver, isuu, isbig5, noansi) /* Modified by ming, 96.10.9  KCN,99.12.16*/
char *fname, *title, *receiver;
int isuu, isbig5, noansi;
{
    struct mail_option mo;
    FILE *fin;
    char uname[STRLEN];

    smtp_session_t session;
    smtp_message_t message;
    smtp_recipient_t recipient;
    const smtp_status_t *status;
    
    if ( isuu  )
    {
        sprintf( uname, "tmp/uu%05d", getpid() );
        sprintf( genbuf, "uuencode %s thbbs.%05d > %s",
                 fname, getpid(), uname );
        system( genbuf );
    }


    session = smtp_create_session ();
    message = smtp_add_message (session);
    
    smtp_set_server (session, "166.111.8.18:25");

    sprintf( newbuf, "%s.bbs@%s", currentuser.userid, email_domain() );
    smtp_set_reverse_path (message, from);
    smtp_set_header (message, "Message-Id", NULL);
    
    smtp_set_header (message, "Subject", title);
    smtp_set_header_option (message, "Subject", Hdr_OVERRIDE, 1);

/*    
    smtp_set_header (message,"Content-Transfer-Encoding", "8bit");
    if (isbig5)
        smtp_set_header (message,"Content-Type","text/plain;\n\tcharset=\"gb2312\"");
    else
        smtp_set_header (message,"Content-Type","text/plain;\n\tcharset=\"gb2312\"");
*/
    if ((fin = fopen (isuu?uname:fname, "r")) == NULL)
    {
      prints("can't open %s: %s\n", file, strerror (errno));
      return -1;
    }

    mo.isbig5=isbig5;
    mo.noansi=noansi;
    mo.fin=fin;
    smtp_set_messagecb (message, bbs_readmailfile, (void*)&mo);

    recipient = smtp_add_recipient (message, argv[optind++]);
    if (notify != Notify_NOTSET)
        smtp_dsn_set_notify (recipient, notify);
    /* Initiate a connection to the SMTP server and transfer the
        message. */
    smtp_start_session (session);
    status = smtp_message_transfer_status (message);
    prints("%d %s", status->code, status->text);
    smtp_enumerate_recipients (message, print_recipient_status, NULL);
    
    /* Free resources consumed by the program.
    */
    smtp_destroy_session (session);
    fclose (fp);
/*
    char* buf,*p;
    char newbuf[256];
    int esc;

    fprintf( fout, "Reply-To: %s.bbs@%s\n", currentuser.userid, email_domain());
    fprintf( fout, "From: %s.bbs@%s\n", currentuser.userid, email_domain() );
    fprintf( fout, "To: %s\n", receiver);
    fprintf( fout, "Subject: %s\n", title);
    fprintf( fout, "X-Forwarded-By: %s (%s)\n",
             currentuser.userid,
#ifdef REALNAME
             currentuser.REALNAME);
#else
             currentuser.username);
#endif

    fprintf(fout, "X-Disclaimer: %s 对本信内容恕不负责。\n", BoardName);
    fprintf(fout, "Precedence: junk\n");
    fprintf(fout, "\n");

    fclose(fout);

    if ( isbig5 == 0 )
    {
        strcpy(tname, hname);
        strcpy(cname, fname);
    }
    else
    {
        sprintf(tname, "/tmp/hcvt%05d", getpid() );
        system_convert(hname, tname, "g2b");

        sprintf(cname, "/tmp/ccvt%05d", getpid() );
        system_convert(fname, cname, "g2b");
    }

    if ( isuu == 0 )
        strcpy( uname, cname );
    else {
        sprintf( uname, "tmp/uu%05d", getpid() );
        sprintf( genbuf, "uuencode %s thbbs.%05d > %s",
                 cname, getpid(), uname );
        system( genbuf );
    }

    sprintf( genbuf, "/usr/lib/sendmail -f %s.bbs@%s %s ",
             currentuser.userid, email_domain(), receiver );
    fout = popen( genbuf, "w" );
    fin  = fopen( tname, "r" );
    fin2 = fopen( uname, "r" );
    if (fout == NULL || fin == NULL || fin2 == NULL) return -1;

    while (fgets( genbuf, 255, fin ) != NULL ) {
        if (genbuf[0] == '.' && genbuf[ 1 ] == '\n')
            fputs( ". \n", fout );
        else
            fputs( genbuf, fout );
    }

    while (fgets( genbuf, 255, fin2 ) != NULL ) {
        if (genbuf[0] == '.' && genbuf[ 1 ] == '\n')
            fputs( ". \n", fout );
        else
            if (noansi)
            {
                buf=newbuf;
                esc=0;
                for (p=genbuf;*p;p++) {
                    if (esc) {
                        if (*p=='\033') {
                            esc=0;
                            *buf=*p;
                            buf++;
                        }else
                            if (isalpha(*p))
                                esc=0;
                    } else {
                        if (*p=='\033') {
                            esc=1;
                        } else {
                            *buf=*p;
                            buf++;
                        }
                    }
                }

                *buf=0;
                fputs( newbuf, fout );
            } else
                fputs( genbuf, fout );
    }
    fprintf(fout, ".\n");

    fclose( fin );
    pclose( fout );

    if ( isuu == 1 )
        system_delete(uname);
    if ( isbig5 == 1 ) {
        system_delete(cname);
        system_delete(tname);
    }
    system_delete(hname);
*/
    return 0;
}

