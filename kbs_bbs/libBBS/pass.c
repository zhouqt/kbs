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
#include <sys/param.h>
#include <sys/resource.h>
#include "md5.h"
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

void igenpass(const char *passwd,const char *userid,unsigned char md5passwd[])
{
    static const char passmagic[]="wwj&kcn4SMTHBBS MD5 p9w2d gen2rat8, //grin~~, 2001/5/7";
    MD5_CTX md5;
    MD5Init(&md5);
    
    /* update size > 128 */
    MD5Update(&md5,(unsigned char *)passmagic,strlen(passmagic));
    MD5Update(&md5,(unsigned char *)passwd,strlen(passwd));
    MD5Update(&md5,(unsigned char *)passmagic,strlen(passmagic));
    MD5Update(&md5,(unsigned char *)userid,strlen(userid));
    
    MD5Final(md5passwd,&md5);
}

int setpasswd(const char *passwd,struct userec *user)
{
    igenpass(passwd,user->userid,user->md5passwd);
    user->passwd[0]=0;
    return 1;
}

int checkpasswd2(const char * passwd,const struct userec *user)
{
    if(user->passwd[0]){
        return checkpasswd(user->passwd,passwd);
    } else {
        unsigned char md5passwd[MD5_DIGEST_LENGTH];
        igenpass(passwd,user->userid,md5passwd);
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
        return !(memcmp(md5passwd,user->md5passwd,MD5_DIGEST_LENGTH));
    }
}

int checkpasswd(const char * passwd,const char * test)
{
    char *crypt1(char *buf, char *salt);

    static char pwbuf[14] ;
    char *pw ;

/*    return 1; */
    strncpy(pwbuf,test,14) ;
    pw = crypt1(pwbuf, (char *)passwd) ;
    return (!strcmp(pw, passwd)) ;
}

