/*

canohost.c

Author: Tatu Ylonen <ylo@cs.hut.fi>

Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
                   All rights reserved

Created: Sun Jul  2 17:52:22 1995 ylo

Functions for returning the canonical host name of the remote site.

*/

#include "includes.h"
#include "packet.h"
#include "xmalloc.h"
#include "ssh.h"

/* Return the canonical name of the host at the other end of the socket. 
   The caller should free the returned string with xfree. */

char *get_remote_hostname(int socket)
{
    struct sockaddr_in from;
    int i;
    socklen_t fromlen;
    struct hostent *hp;
    char name[255];

    /* Get IP address of client. */
    fromlen = sizeof(from);
    memset(&from, 0, sizeof(from));
    if (getpeername(socket, (struct sockaddr *) &from, &fromlen) < 0) {
        error("getpeername failed: %.100s", strerror(errno));
        strcpy(name, "UNKNOWN");
        goto check_ip_options;
    }

    /* Map the IP address to a host name. */
    // hp = gethostbyaddr((char *) &from.sin_addr, sizeof(struct in_addr), from.sin_family);
    hp = NULL;
    if (hp) {
        /* Got host name. */
        strncpy(name, hp->h_name, sizeof(name));
        name[sizeof(name) - 1] = '\0';

        /* Convert it to all lowercase (which is expected by the rest of this
           software). */
        for (i = 0; name[i]; i++)
            if (isupper(name[i]))
                name[i] = tolower(name[i]);

        /* Map it back to an IP address and check that the given address actually
           is an address of this host.  This is necessary because anyone with
           access to a name server can define arbitrary names for an IP address.
           Mapping from name to IP address can be trusted better (but can still
           be fooled if the intruder has access to the name server of the
           domain). */
        hp = gethostbyname(name);
        if (!hp) {
            log_msg("reverse mapping checking gethostbyname for %.700s failed - POSSIBLE BREAKIN ATTEMPT!", name);
            strcpy(name, inet_ntoa(from.sin_addr));
            goto check_ip_options;
        }
        /* Look for the address from the list of addresses. */
        for (i = 0; hp->h_addr_list[i]; i++)
            if (memcmp(hp->h_addr_list[i], &from.sin_addr, sizeof(from.sin_addr))
                == 0)
                break;
        /* If we reached the end of the list, the address was not there. */
        if (!hp->h_addr_list[i]) {
            /* Address not found for the host name. */
            log_msg("Address %.100s maps to %.600s, but this does not map back to the address - POSSIBLE BREAKIN ATTEMPT!", inet_ntoa(from.sin_addr), name);
            strcpy(name, inet_ntoa(from.sin_addr));
            goto check_ip_options;
        }
        /* Address was found for the host name.  We accept the host name. */
    } else {
        /* Host name not found.  Use ascii representation of the address. */
        strcpy(name, inet_ntoa(from.sin_addr));
        log_msg("Could not reverse map address %.100s.", name);
    }

  check_ip_options:

#ifdef IP_OPTIONS
    /* If IP options are supported, make sure there are none (log and clear
       them if any are found).  Basically we are worried about source routing;
       it can be used to pretend you are somebody (ip-address) you are not.
       That itself may be "almost acceptable" under certain circumstances,
       but rhosts autentication is useless if source routing is accepted.
       Notice also that if we just dropped source routing here, the other
       side could use IP spoofing to do rest of the interaction and could still
       bypass security.  So we exit here if we detect any IP options. */
    {
        unsigned char options[200], *ucp;
        char text[1024], *cp;
        socklen_t option_size;
        int ipproto;
        struct protoent *ip;

        if ((ip = getprotobyname("ip")) != NULL)
            ipproto = ip->p_proto;
        else
            ipproto = IPPROTO_IP;
        option_size = sizeof(options);
        if (getsockopt(socket, ipproto, IP_OPTIONS, (char *) options, &option_size) >= 0 && option_size != 0) {
            cp = text;
            if (option_size > 256)
                option_size = 256;
            /* Note: "text" buffer must be at least 3x as big as options. */
            for (ucp = options; option_size > 0; ucp++, option_size--, cp += 3)
                sprintf(cp, " %2.2x", *ucp);
            log_msg("Connection from %.100s with IP options:%.800s", inet_ntoa(from.sin_addr), text);
            packet_disconnect("Connection from %.100s with IP options:%.800s", inet_ntoa(from.sin_addr), text);
        }
    }
#endif

    return xstrdup(name);
}

static char *canonical_host_name = NULL;
static char *canonical_host_ip = NULL;

/* Return the canonical name of the host in the other side of the current
   connection.  The host name is cached, so it is efficient to call this 
   several times. */

const char *get_canonical_hostname(void)
{
    socklen_t fromlen, tolen;
    struct sockaddr_in from, to;

    /* Check if we have previously retrieved this same name. */
    if (canonical_host_name != NULL)
        return canonical_host_name;

    /* If using different descriptors for the two directions, check if
       both have the same remote address.  If so, get the address; otherwise
       return UNKNOWN. */
    if (packet_get_connection_in() != packet_get_connection_out()) {
        fromlen = sizeof(from);
        memset(&from, 0, sizeof(from));
        if (getpeername(packet_get_connection_in(), (struct sockaddr *) &from, &fromlen) < 0)
            goto no_ip_addr;

        tolen = sizeof(to);
        memset(&to, 0, sizeof(to));
        if (getpeername(packet_get_connection_out(), (struct sockaddr *) &to, &tolen) < 0)
            goto no_ip_addr;

        if (from.sin_family == AF_INET && to.sin_family == AF_INET && memcmp(&from, &to, sizeof(from)) == 0)
            goto return_ip_addr;

      no_ip_addr:
        canonical_host_name = xstrdup("UNKNOWN");
        return canonical_host_name;
    }

  return_ip_addr:

    /* Get the real hostname. */
    canonical_host_name = get_remote_hostname(packet_get_connection_in());
    return canonical_host_name;
}

/* Returns the IP-address of the remote host as a string.  The returned
   string need not be freed. */

const char *get_remote_ipaddr(void)
{
    struct sockaddr_in from, to;
    socklen_t fromlen,tolen;
    int socket;

    /* Check if we have previously retrieved this same name. */
    if (canonical_host_ip != NULL)
        return canonical_host_ip;

    /* If using different descriptors for the two directions, check if
       both have the same remote address.  If so, get the address; otherwise
       return UNKNOWN. */
    if (packet_get_connection_in() != packet_get_connection_out()) {
        fromlen = sizeof(from);
        memset(&from, 0, sizeof(from));
        if (getpeername(packet_get_connection_in(), (struct sockaddr *) &from, &fromlen) < 0)
            goto no_ip_addr;

        tolen = sizeof(to);
        memset(&to, 0, sizeof(to));
        if (getpeername(packet_get_connection_out(), (struct sockaddr *) &to, &tolen) < 0)
            goto no_ip_addr;

        if (from.sin_family == AF_INET && to.sin_family == AF_INET && memcmp(&from, &to, sizeof(from)) == 0)
            goto return_ip_addr;

      no_ip_addr:
        canonical_host_ip = xstrdup("UNKNOWN");
        return canonical_host_ip;
    }

  return_ip_addr:

    /* Get client socket. */
    socket = packet_get_connection_in();

    /* Get IP address of client. */
    fromlen = sizeof(from);
    memset(&from, 0, sizeof(from));
    if (getpeername(socket, (struct sockaddr *) &from, &fromlen) < 0) {
        error("getpeername failed: %.100s", strerror(errno));
        return NULL;
    }

    /* Get the IP address in ascii. */
    canonical_host_ip = xstrdup(inet_ntoa(from.sin_addr));

    /* Return ip address string. */
    return canonical_host_ip;
}

/* Returns the port of the peer of the socket. */

int get_peer_port(int sock)
{
    struct sockaddr_in from;
    socklen_t fromlen;

    /* Get IP address of client. */
    fromlen = sizeof(from);
    memset(&from, 0, sizeof(from));
    if (getpeername(sock, (struct sockaddr *) &from, &fromlen) < 0) {
        error("getpeername failed: %.100s", strerror(errno));
        return 0;
    }

    /* Return port number. */
    return ntohs(from.sin_port);
}

/* Returns the port number of the remote host.  */

int get_remote_port(void)
{
    int socket;
    socklen_t fromlen,tolen;
    struct sockaddr_in from, to;

    /* If two different descriptors, check if they are internet-domain, and
       have the same address. */
    if (packet_get_connection_in() != packet_get_connection_out()) {
        fromlen = sizeof(from);
        memset(&from, 0, sizeof(from));
        if (getpeername(packet_get_connection_in(), (struct sockaddr *) &from, &fromlen) < 0)
            goto no_ip_addr;

        tolen = sizeof(to);
        memset(&to, 0, sizeof(to));
        if (getpeername(packet_get_connection_out(), (struct sockaddr *) &to, &tolen) < 0)
            goto no_ip_addr;

        if (from.sin_family == AF_INET && to.sin_family == AF_INET && memcmp(&from, &to, sizeof(from)) == 0)
            goto return_port;

      no_ip_addr:
        return 65535;
    }

  return_port:

    /* Get client socket. */
    socket = packet_get_connection_in();

    /* Get and return the peer port number. */
    return get_peer_port(socket);
}
