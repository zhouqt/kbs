#include "bbs.h"

struct boardheader *bcache;
struct BCACHE *brdshm;
struct BDIRCACHE	*bdirshm;
struct UTMPFILE *utmpshm;
struct UTMPHEAD *utmphead;

int WORDBOUND, WHOLELINE, NOUPPER, INVERSE, FILENAMEONLY, SILENT, FNAME;
int ONLYCOUNT;

struct UCACHE *uidshm = NULL;

/*log.c*/
int disablelog = 0;
int logmsqid = -1;

struct public_data *publicshm;

#ifndef THREADSAFE
session_t g_session;

#ifdef CYGWIN
session_t* getSession() { return &g_session; }
#endif

#endif
void init_sessiondata(session_t * session)
{
    bzero(session,sizeof(*session));
#ifdef HAVE_BRC_CONTROL
    session->brc_cache_entry=NULL;
    session->brc_currcache=-1;
#endif

    session->zapbuf_changed = 0;

    session->mybrd_list_t = -1;
    session->favnow = 0;
    session->favbrd_list = NULL;

    session->currentuser=NULL;
    session->currentuid=0;
    session->currentmemo=NULL;
    session->topfriend=NULL;

    session->sigjmp_stack=NULL;

#ifdef SMS_SUPPORT    
    session->lastsmsstatus=0;
    session->smsbuf=NULL;
    session->smsresult=0;

#endif

    session->utmpent=-1;
    session->currentuinfo=NULL;
}


int init_all(ARG_VOID)
{
    int ret=0;
    chdir(BBSHOME);
    get_publicshm();
    resolve_boards();
    ret=resolve_ucache();
    resolve_utmp();
#ifndef THREADSAFE
    init_sessiondata(getSession());
#endif
	resolve_guest_table();
    return ret;
}
