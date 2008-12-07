/*
    Pirate Bulletin Board System
    Copyright (C) 1990, Edward Luke, lush@Athena.EE.MsState.EDU
    Eagles Bulletin Board System
    Copyright (C) 1992, Raymond Rocker, rocker@rock.b11.ingr.com
   Guy Vega, gtvega@seabass.st.usm.edu
   Tynes, dbtynes@seabass.st.usm.edu

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "bbs.h"
#include <sys/resource.h>
#include "md5.h"

#ifdef SECONDSITE
#include "libauth.h"
#endif

/*
#include "w_md5.h"


void w_igenpass(const char *passwd,const char *userid,unsigned char md5passwd[])
{
    static const char passmagic[]="wwj&kcn4SMTHBBS MD5 p9w2d gen2rat8, //grin~~, 2001/5/7";
    w_MD5_CTX md5;
    w_MD5Init(&md5);

    w_MD5Update(&md5,(unsigned char *)passmagic,strlen(passmagic));
    w_MD5Update(&md5,(unsigned char *)passwd,strlen(passwd));
    w_MD5Update(&md5,(unsigned char *)passmagic,strlen(passmagic));
    w_MD5Update(&md5,(unsigned char *)userid,strlen(userid));

    w_MD5Final(&md5,md5passwd);
}
*/

void igenpass(const char *passwd, const char *userid, unsigned char md5passwd[])
{
    static const char passmagic[] = "wwj&kcn4SMTHBBS MD5 p9w2d gen2rat8, //grin~~, 2001/5/7";
    MD5_CTX md5;

    MD5Init(&md5);

    /* update size > 128 */
    MD5Update(&md5, (unsigned char *) passmagic, strlen(passmagic));
    MD5Update(&md5, (unsigned char *) passwd, strlen(passwd));
    MD5Update(&md5, (unsigned char *) passmagic, strlen(passmagic));
    MD5Update(&md5, (unsigned char *) userid, strlen(userid));

    MD5Final(md5passwd, &md5);
}

int setpasswd(const char *passwd, struct userec *user)
{
    igenpass(passwd, user->userid, user->md5passwd);
#ifdef CONV_PASS
    user->passwd[0] = 0;
#endif
    return 1;
}

int checkpasswd2(const char *passwd, const struct userec *user)
{
#ifdef CONV_PASS
    if (user->passwd[0]) {
        return checkpasswd(user->passwd, passwd);
    } else
#endif
    {
        unsigned char md5passwd[MD5_DIGEST_LENGTH];

        igenpass(passwd, user->userid, md5passwd);
        /*
                if (memcmp(md5passwd,user->md5passwd,MD5_DIGEST_LENGTH)) {
                    unsigned char w_md5passwd[MD5_DIGEST_LENGTH];
                    w_igenpass(passwd,user->userid,w_md5passwd);
                    if (memcmp(w_md5passwd,user->md5passwd,MD5_DIGEST_LENGTH)) return 0;
                    memcpy(user->md5passwd,md5passwd,MD5_DIGEST_LENGTH);
                    bbslog("5system","Convert %s password.",user->userid);
                }
         return 1;
        */
        return !(memcmp(md5passwd, user->md5passwd, MD5_DIGEST_LENGTH));
    }
}

#ifdef FREE

char *crypt();
#define OLDPWLEN 35

#else

#define OLDPWLEN 14

#endif

int checkpasswd(const char *passwd, const char *test)
{
    char *crypt1(char *buf, char* outputbuff, char *salt);

    char pwbuf[OLDPWLEN];
    char buff[20];
    char *pw;

    /*    return 1; */
    strncpy(pwbuf, test, OLDPWLEN);
#ifdef FREE
    pw = crypt(pwbuf, passwd);
#else
    pw = crypt1(pwbuf, buff, (char *) passwd);
#endif
    return (!strcmp(pw, passwd));
}

#ifdef SECONDSITE
static int kickuser(struct user_info *uentp, char *arg, int count)
{
    if (uentp->mode != WEBEXPLORE)
        kill(uentp->pid, SIGKILL);
    clear_utmp((uentp - utmpshm->uinfo) + 1, uentp->uid, uentp->pid);
    return 0;
}

static void rm_userid_stuff(char *userid)
{
    char tmpbuf[256];
    char genbuf1[256];
    newbbslog(BBSLOG_USIES, "kill user %s", userid);
    apply_utmp((APPLY_UTMP_FUNC) kickuser, 0, userid, 0);
    setmailpath(tmpbuf, userid);
    sprintf(genbuf1, "/bin/rm -rf %s", tmpbuf);
    system(genbuf1);
    sethomepath(tmpbuf, userid);
    sprintf(genbuf1, "/bin/rm -rf %s", tmpbuf);
    system(genbuf1);
    sprintf(genbuf1, "/bin/rm -fr tmp/email/%s", userid);
    system(genbuf1);
    return ;
}

static void create_userid_stuff(struct userec *u)
{
    char buf[256];
    char cmd[500];

    sethomepath(buf, u->userid);
    sprintf(cmd, "/bin/mv -f %s " BBSHOME "/homeback/%s", buf, u->userid);
    system(cmd);
    mkdir(buf, 0777);

    setmailpath(buf, u->userid);
    sprintf(cmd, "/bin/mv -f %s " BBSHOME "/homeback/%s", buf, u->userid);
    system(cmd);
    mkdir(buf, 0777);

    u->userlevel = PERM_DEFAULT;
    u->userdefine[0] = -1;
    u->userdefine[1] = -1;
    SET_UNDEFINE(u, DEF_NOTMSGFRIEND);

    u->exittime = time(NULL) - 100;
    u->flags |= PAGER_FLAG;
    u->title = 0;
    u->firstlogin = u->lastlogin = time(NULL);

}

int remote_auth(const char *passwd, const char *userid, char *permstr)
{
    struct bbsauth auth;
    int uid;
    struct userec *u;

    memset(&auth, 0, sizeof(auth));
    uid = bbs_auth(&auth, userid, passwd, getSession()->fromhost);

    if (permstr) {
        if (uid<=0) permstr[0]='\0';
        else {
            strncpy(permstr, auth.perm, 32);
            permstr[32]='\0';
        }
    }

    if (uid > 0) {
        u = getuserbynum(uid);
        if (u==NULL) uid=0;
    }
    if (uid > 0) {
        if (!strcasecmp(u->userid, auth.userid)) {
            /*TODO: check local->firstlogin > remote->firstlogin ? */
            setpasswd(passwd, u);
        } else {
            int id;
            struct userec *u1;

            if (u->userid[0]) {
                rm_userid_stuff(u->userid);
                setuserid2(uid, "");
            }

            if ((id=getuser(auth.userid, &u1))!=0) {
                rm_userid_stuff(u1->userid);
                setuserid2(id, "");
                memset(u1, 0, sizeof(struct userec));
            }

            memset(u, 0, sizeof(struct userec));
            setuserid2(uid, auth.userid);
            setpasswd(passwd, u);
            create_userid_stuff(u);
        }
    }

    return uid;
}
#endif /* SECONDSITE */

