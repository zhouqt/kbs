/*

auth-passwd.c

Author: Tatu Ylonen <ylo@cs.hut.fi>

Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
                   All rights reserved

Created: Sat Mar 18 05:11:38 1995 ylo

Password authentication.  This file contains the functions to check whether
the password is valid for the user.

*/

#include "includes.h"
#include "packet.h"
#include "ssh.h"
#include "servconf.h"
#include "xmalloc.h"
#include "bbs.h"

char useridbuf[255];

/* Tries to authenticate the user using password.  Returns true if
   authentication succeeds. */
int auth_password(const char *server_user, const char *password)
{
    int sinlen;
#ifdef SECONDSITE
    char permstr[33];
#endif
#ifdef HAVE_IPV6_SMTH
    struct sockaddr_in6 sin;
    char host[IPLEN];
#else
    struct sockaddr_in sin;
    char *host;
#endif
    load_sysconf();
    resolve_ucache();
    resolve_utmp();
    strcpy(useridbuf, server_user);

#ifdef SECONDSITE
    if (*server_user == '\0')
        return 0;
    if(password[0] == '\0')
        return 0;
#else /* SECONDSITE */
    if (*server_user == '\0' || !dosearchuser(useridbuf))
        return 0;
    if (password[0] == '\0')
        return (!strcasecmp(useridbuf,"guest"));
#endif /* SECONDSITE */

#ifdef HAVE_IPV6_SMTH
    sinlen = sizeof(struct sockaddr_in6);
    getpeername(packet_get_connection_in(), (struct sockaddr *) &sin, (void *) &sinlen);
	host[0]='\0';
#ifdef LEGACY_IPV4_DISPLAY
	if (ISV4ADDR(sin.sin6_addr)) 
	    inet_ntop(AF_INET, &sin.sin6_addr.s6_addr[12], host, IPLEN); 
	else
#endif
	inet_ntop(AF_INET6, &sin.sin6_addr, host, IPLEN);
#else
    sinlen = sizeof(struct sockaddr_in);
    getpeername(packet_get_connection_in(), (struct sockaddr *) &sin, (void *) &sinlen);
    host = (char *) inet_ntoa(sin.sin_addr);
#endif /* IPV6 */

#ifdef SECONDSITE
    {
        FILE *fp;

        strncpy(getSession()->fromhost, host, IPLEN);
        getSession()->fromhost[IPLEN-1] = 0;
        
        if(fp=fopen("/home/bbs/LOCALCHECK", "r")) {
#endif /* SECONDSITE */
    if(check_ip_acl(getCurrentUser()->userid, host)) {
    	return 0;
    }
    if (!checkpasswd2(password, getCurrentUser())) {
        logattempt(getCurrentUser()->userid, (char *)get_canonical_hostname(), "ssh");
        return 0;
    }
#ifdef SECONDSITE
            fclose(fp);
        } else {
        
            if(remote_auth(password, useridbuf, permstr) <= 0) {
                return 0;
            }
            if(permstr[0] == '\0' || permstr[0] == '+' || XPERMSTR[4] != permstr[4]) {
                return 0;
            }
            if(!dosearchuser(useridbuf))
                return 0;
        }
    }
#endif /* SECONDSITE */

#ifdef NEWSMTH
    strcpy(getSession()->passwd, password);
#endif
    strcpy(useridbuf, server_user);
    return 1;
}
