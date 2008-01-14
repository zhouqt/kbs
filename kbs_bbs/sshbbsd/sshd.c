/*

sshd.c

Author: Tatu Ylonen <ylo@cs.hut.fi>

Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
                   All rights reserved

Created: Fri Mar 17 17:09:28 1995 ylo

This program is the ssh daemon.  It listens for connections from clients, and
performs authentication, executes use commands or shell, and forwards
information to/from the application to the user client over an encrypted
connection.  This can also handle forwarding of X11, TCP/IP, and authentication
agent connections.

*/

#include "includes.h"
#include "bbs.h"
#include "ssh_funcs.h"
#include <gmp.h>
#include "xmalloc.h"
#include "rsa.h"
#include "ssh.h"
#include "packet.h"
#include "buffer.h"
#include "cipher.h"
#include "mpaux.h"
#include "servconf.h"
#include "userfile.h"
#include "emulate.h"

#ifdef HAVE_ULIMIT_H
#include <ulimit.h>
#endif                          /* HAVE_ULIMIT_H */
#ifdef HAVE_ETC_SHADOW
#ifdef HAVE_SHADOW_H
#include <shadow.h>
#endif                          /* HAVE_SHADOW_H */
#ifndef SHADOW
#define SHADOW "/etc/shadow"
#endif
#endif                          /* HAVE_ETC_SHADOW */


#ifdef LIBWRAP
#include <tcpd.h>
#include <syslog.h>
#ifdef NEED_SYS_SYSLOG_H
#include <sys/syslog.h>
#endif                          /* NEED_SYS_SYSLOG_H */
int allow_severity = LOG_INFO;
int deny_severity = LOG_WARNING;
#endif                          /* LIBWRAP */


#ifdef _PATH_BSHELL
#define DEFAULT_SHELL           _PATH_BSHELL
#else
#define DEFAULT_SHELL           "/bin/sh"
#endif

#ifndef DEFAULT_PATH
#ifdef _PATH_USERPATH
#define DEFAULT_PATH            _PATH_USERPATH
#else
#ifdef _PATH_DEFPATH
#define DEFAULT_PATH            _PATH_DEFPATH
#else
#define DEFAULT_PATH    "/bin:/usr/bin:/usr/ucb:/usr/bin/X11:/usr/local/bin"
#endif
#endif
#endif                          /* DEFAULT_PATH */

#ifndef O_NOCTTY
#define O_NOCTTY        0
#endif

/* Server configuration options. */
ServerOptions options;

/* Name of the server configuration file. */
char *config_file_name = SERVER_CONFIG_FILE;

/* Debug mode flag.  This can be set on the command line.  If debug
   mode is enabled, extra debugging output will be sent to the system
   log, the daemon will not go to background, and will exit after processing
   the first connection. */
int debug_flag = 0;

/* Flag indicating that the daemon is being started from inetd. */
int inetd_flag = 0;

/* argv[0] without path. */
char *av0;

/* Saved arguments to main(). */
char **saved_argv;

/* This is set to the socket that the server is listening; this is used in
   the SIGHUP signal handler. */
int listen_sock;

/* This is not really needed, and could be eliminated if server-specific
   and client-specific code were removed from newchannels.c */
uid_t original_real_uid = 0;

/* Flags set in auth-rsa from authorized_keys flags.  These are set in
  auth-rsa.c. */
int no_port_forwarding_flag = 0;
int no_agent_forwarding_flag = 0;
int no_x11_forwarding_flag = 0;
int no_pty_flag = 0;
time_t idle_timeout = 0;
char *forced_command = NULL;    /* RSA authentication "command=" option. */
char *original_command = NULL;  /* original command from protocol. */
struct envstring *custom_environment = NULL;

                          /* RSA authentication "environment=" options. */

/* Session id for the current session. */
unsigned char session_id[16];

/* Any really sensitive data in the application is contained in this structure.
   The idea is that this structure could be locked into memory so that the
   pages do not get written into swap.  However, there are some problems.
   The private key contains MP_INTs, and we do not (in principle) have
   access to the internals of them, and locking just the structure is not
   very useful.  Currently, memory locking is not implemented. */
struct {
    /* Random number generator. */
    RandomState random_state;

    /* Private part of server key. */
    RSAPrivateKey private_key;

    /* Private part of host key. */
    RSAPrivateKey host_key;
} sensitive_data;

/* Flag indicating whether the current session key has been used.  This flag
   is set whenever the key is used, and cleared when the key is regenerated. */
int key_used = 0;

/* This is set to true when SIGHUP is received. */
int received_sighup = 0;

/* Public side of the server key.  This value is regenerated regularly with
   the private key. */
RSAPublicKey public_key;

/* Remote end username (mallocated) or NULL if not available */
char *remote_user_name;

/* Days before the password / account expires, or -1 if information not
   available */
int days_before_account_expires = -1;
int days_before_password_expires = -1;

/* Prototypes for various functions defined later in this file. */
void do_connection(int privileged_port);
void do_authentication(char *user, int privileged_port, int cipher_type);
void do_authenticated(char *pw);
void do_exec_no_pty(const char *command, char *pw, const char *display, const char *auth_proto, const char *auth_data);
void do_child(const char *command, char *pw, const char *term, const char *display, const char *auth_proto, const char *auth_data, const char *ttyname);


/* Signal handler for SIGHUP.  Sshd execs itself when it receives SIGHUP;
   the effect is to reread the configuration file (and to regenerate
   the server key). */

RETSIGTYPE sighup_handler(int sig)
{
    received_sighup = 1;
    signal(SIGHUP, sighup_handler);
}

/* Called from the main program after receiving SIGHUP.  Restarts the 
   server. */

void sighup_restart(void)
{
    log_msg("Received SIGHUP; restarting.");
    close(listen_sock);
    execvp(saved_argv[0], saved_argv);
    log_msg("RESTART FAILED: av[0]='%.100s', error: %.100s.", saved_argv[0], strerror(errno));
    exit(1);
}

/* Generic signal handler for terminating signals in the master daemon. 
   These close the listen socket; not closing it seems to cause "Address
   already in use" problems on some machines, which is inconvenient. */

RETSIGTYPE sigterm_handler(int sig)
{
    log_msg("Received signal %d; terminating.", sig);
    close(listen_sock);
    exit(255);
}

#ifdef SIGDANGER
/* Signal handler for AIX's SIGDANGER low-memory signal
   It logs the signal and ignores the message. */
RETSIGTYPE sigdanger_handler(int sig)
{
    log_msg("Received signal %d (SIGDANGER, means memory is low); ignoring.", sig);
}
#endif                          /* SIGDANGER */

/* SIGCHLD handler.  This is called whenever a child dies.  This will then 
   reap any zombies left by exited c. */

RETSIGTYPE main_sigchld_handler(int sig)
{
    int status;

#ifdef HAVE_WAITPID
    /* Reap all childrens */
    while (waitpid(-1, &status, WNOHANG) > 0);
#else
    wait(&status);
#endif
    signal(SIGCHLD, main_sigchld_handler);
}

/* Signal handler for the alarm after the login grace period has expired. */

RETSIGTYPE grace_alarm_handler(int sig)
{
    /* Close the connection. */
    packet_close();

    /* Log error and exit. */
    fatal_severity(SYSLOG_SEVERITY_INFO, "Timeout before authentication.");
}

/* Signal handler for the key regeneration alarm.  Note that this
   alarm only occurs in the daemon waiting for connections, and it does not
   do anything with the private key or random state before forking.  Thus there
   should be no concurrency control/asynchronous execution problems. */

RETSIGTYPE key_regeneration_alarm(int sig)
{
    /* Check if we should generate a new key. */
    if (key_used) {
        /* This should really be done in the background. */
        log_msg("Generating new %d bit RSA key.", options.server_key_bits);
        random_acquire_light_environmental_noise(&sensitive_data.random_state);
        rsa_generate_key(&sensitive_data.private_key, &public_key, &sensitive_data.random_state, options.server_key_bits);
        random_save(&sensitive_data.random_state, geteuid(), options.random_seed_file);
        key_used = 0;
        log_msg("RSA key generation complete.");
    }

    /* Reschedule the alarm. */
    signal(SIGALRM, key_regeneration_alarm);
    alarm(options.key_regeneration_time);
}

/* Main program for the daemon. */

int main(int ac, char **av)
{
    extern char *optarg;
    extern int optind;
    int opt, sock_in, sock_out, newsock, i, pid = 0, on = 1;
    socklen_t aux;
    int remote_major, remote_minor;
    int perm_denied = 0;
    int ret;
    fd_set fdset;
#ifdef HAVE_IPV6_SMTH
    struct sockaddr_in6 sin;
#else
    struct sockaddr_in sin;
#endif
    char buf[100];              /* Must not be larger than remote_version. */
    char remote_version[100];   /* Must be at least as big as buf. */
    char addr[STRLEN];
    char *comment;
    char *ssh_remote_version_string = NULL;
    FILE *f;

#if defined(SO_LINGER) && defined(ENABLE_SO_LINGER)
    struct linger linger;
#endif                          /* SO_LINGER */
    int done;

    chdir(BBSHOME);
    /* Save argv[0]. */
    saved_argv = av;
    if (strchr(av[0], '/'))
        av0 = strrchr(av[0], '/') + 1;
    else
        av0 = av[0];

    /* Prevent core dumps to avoid revealing sensitive information. */
    signals_prevent_core();

    /* Set SIGPIPE to be ignored. */
    signal(SIGPIPE, SIG_IGN);

    /* Initialize configuration options to their default values. */
    initialize_server_options(&options);
    addr[0]=0;

    /* Parse command-line arguments. */
    while ((opt = getopt(ac, av, "f:a:p:b:k:h:g:diqV:")) != EOF) {
        switch (opt) {
        case 'f':
            config_file_name = optarg;
            break;
        case 'd':
            debug_flag = 1;
            break;
        case 'i':
            inetd_flag = 1;
            break;
        case 'q':
            options.quiet_mode = 1;
            break;
        case 'b':
            options.server_key_bits = atoi(optarg);
            break;
        case 'a':
            if(optarg[0])
                snprintf(addr,STRLEN,"%s",optarg);
            break;
        case 'p':
            if(isdigit(optarg[0]))
                options.port=atoi(optarg);
            break;
        case 'g':
            options.login_grace_time = atoi(optarg);
            break;
        case 'k':
            options.key_regeneration_time = atoi(optarg);
            break;
        case 'h':
            options.host_key_file = optarg;
            break;
        case 'V':
            ssh_remote_version_string = optarg;
            break;
        case '?':
        default:
#ifdef F_SECURE_COMMERCIAL

#endif                          /* F_SECURE_COMMERCIAL */
            fprintf(stderr, "sshd version %s [%s]\n", SSH_VERSION, HOSTTYPE);
            fprintf(stderr, "Usage: %s [options]\n", av0);
            fprintf(stderr, "Options:\n");
            fprintf(stderr, "  -f file    Configuration file (default %s/sshd_config)\n", ETCDIR);
            fprintf(stderr, "  -d         Debugging mode\n");
            fprintf(stderr, "  -i         Started from inetd\n");
            fprintf(stderr, "  -q         Quiet (no logging)\n");
            fprintf(stderr, "  -a addr    Bind to the specified address (default: all)\n");
            fprintf(stderr, "  -p port    Listen on the specified port (default: 22)\n");
            fprintf(stderr, "  -k seconds Regenerate server key every this many seconds (default: 3600)\n");
            fprintf(stderr, "  -g seconds Grace period for authentication (default: 300)\n");
            fprintf(stderr, "  -b bits    Size of server RSA key (default: 768 bits)\n");
            fprintf(stderr, "  -h file    File from which to read host key (default: %s)\n", HOST_KEY_FILE);
            fprintf(stderr, "  -V str     Remote version string already read from the socket\n");
            exit(1);
        }
    }

    /* Read server configuration options from the configuration file. */
    read_server_config(&options, config_file_name);

    /* Fill in default values for those options not explicitly set. */
    fill_default_server_options(&options);

    /* Check certain values for sanity. */
    if (options.server_key_bits < 512 || options.server_key_bits > 32768) {
        fprintf(stderr, "fatal: Bad server key size.\n");
        exit(1);
    }
    if (options.port < 1 || options.port > 65535) {
        fprintf(stderr, "fatal: Bad port number.\n");
        exit(1);
    }
    if (options.umask != -1) {
        umask(options.umask);
    }

    /* Check that there are no remaining arguments. */
    if (optind < ac) {
        fprintf(stderr, "fatal: Extra argument %.100s.\n", av[optind]);
        exit(1);
    }

    /* Initialize the log (it is reinitialized below in case we forked). */
    log_init(av0, debug_flag && !inetd_flag, debug_flag || options.fascist_logging, options.quiet_mode, options.log_facility);

    debug("sshd version %.100s [%.100s]", SSH_VERSION, HOSTTYPE);

    /* Load the host key.  It must have empty passphrase. */
    done = load_private_key(geteuid(), options.host_key_file, "", &sensitive_data.host_key, &comment);

    if (!done) {
        if (debug_flag) {
            fprintf(stderr, "Could not load host key: %.200s\n", options.host_key_file);
            fprintf(stderr, "fatal: Please check that you have sufficient permissions and the file exists.\n");
        } else {
            log_init(av0, !inetd_flag, 1, 0, options.log_facility);
            error("fatal: Could not load host key: %.200s.  Check path and permissions.", options.host_key_file);
        }
        exit(1);
    }
    xfree(comment);

    /* If not in debugging mode, and not started from inetd, disconnect from
       the controlling terminal, and fork.  The original process exits. */
    if (!debug_flag && !inetd_flag)
#ifdef HAVE_DAEMON
        if (daemon(0, 0) < 0)
            error("daemon: %.100s", strerror(errno));
    chdir(BBSHOME);
#else                           /* HAVE_DAEMON */
    {
#ifdef TIOCNOTTY
        int fd;
#endif                          /* TIOCNOTTY */

        /* Fork, and have the parent exit.  The child becomes the server. */
        if (fork())
            exit(0);

        /* Redirect stdin, stdout, and stderr to /dev/null. */
        freopen("/dev/null", "r", stdin);
        freopen("/dev/null", "w", stdout);
        freopen("/dev/null", "w", stderr);

        /* Disconnect from the controlling tty. */
#ifdef TIOCNOTTY
        fd = open("/dev/tty", O_RDWR | O_NOCTTY);
        if (fd >= 0) {
            (void) ioctl(fd, TIOCNOTTY, NULL);
            close(fd);
        }
#endif                          /* TIOCNOTTY */
#ifdef HAVE_SETSID
#ifdef ultrix
        setpgrp(0, 0);
#else                           /* ultrix */
        if (setsid() < 0)
            error("setsid: %.100s", strerror(errno));
#endif
#endif                          /* HAVE_SETSID */
    }
#endif                          /* HAVE_DAEMON */

    /* Reinitialize the log (because of the fork above). */
    log_init(av0, debug_flag && !inetd_flag, debug_flag || options.fascist_logging, options.quiet_mode, options.log_facility);

    /* Check that server and host key lengths differ sufficiently.  This is
       necessary to make double encryption work with rsaref.  Oh, I hate
       software patents. */
    if (options.server_key_bits > sensitive_data.host_key.bits - SSH_KEY_BITS_RESERVED && options.server_key_bits < sensitive_data.host_key.bits + SSH_KEY_BITS_RESERVED) {
        options.server_key_bits = sensitive_data.host_key.bits + SSH_KEY_BITS_RESERVED;
        debug("Forcing server key to %d bits to make it differ from host key.", options.server_key_bits);
    }

    /* Initialize memory allocation so that any freed MP_INT data will be
       zeroed. */
    rsa_set_mp_memory_allocation();

    /* Do not display messages to stdout in RSA code. */
    rsa_set_verbose(debug_flag);

    /* Initialize the random number generator. */
    debug("Initializing random number generator; seed file %.200s", options.random_seed_file);
    random_initialize(&sensitive_data.random_state, geteuid(), options.random_seed_file);

    /* Chdir to the root directory so that the current disk can be unmounted
       if desired. */

    idle_timeout = options.idle_timeout;

    /* Start listening for a socket, unless started from inetd. */
    if (inetd_flag) {
        int s1, s2;

        s1 = dup(0);            /* Make sure descriptors 0, 1, and 2 are in use. */
        s2 = dup(s1);
        sock_in = dup(0);
        sock_out = dup(1);
        /* We intentionally do not close the descriptors 0, 1, and 2 as our
           code for setting the descriptors won\'t work if ttyfd happens to
           be one of those. */
        debug("inetd sockets after dupping: %d, %d", sock_in, sock_out);

        /* Generate an rsa key. */
        log_msg("Generating %d bit RSA key.", options.server_key_bits);
        rsa_generate_key(&sensitive_data.private_key, &public_key, &sensitive_data.random_state, options.server_key_bits);
        random_save(&sensitive_data.random_state, geteuid(), options.random_seed_file);
        log_msg("RSA key generation complete.");
    } else {
        /* Create socket for listening. */
#ifdef HAVE_IPV6_SMTH
        listen_sock = socket(AF_INET6, SOCK_STREAM, 0);
#else       
        listen_sock = socket(AF_INET, SOCK_STREAM, 0);
#endif
        if (listen_sock < 0)
            fatal("socket: %.100s", strerror(errno));

        /* Set socket options.  We try to make the port reusable and have it
           close as fast as possible without waiting in unnecessary wait states
           on close. */
        setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, (void *) &on, sizeof(on));
#if defined(SO_LINGER) && defined(ENABLE_SO_LINGER)
        linger.l_onoff = 1;
        linger.l_linger = 15;
        setsockopt(listen_sock, SOL_SOCKET, SO_LINGER, (void *) &linger, sizeof(linger));
#endif                          /* SO_LINGER */

        /* Initialize the socket address. */
        memset(&sin, 0, sizeof(sin));
#ifdef HAVE_IPV6_SMTH
        sin.sin6_family = AF_INET6;
        if ( inet_pton(AF_INET6, addr, &(sin.sin6_addr)) <= 0 )
            sin.sin6_addr = in6addr_any;
        sin.sin6_port = htons(options.port);
#else
        sin.sin_family = AF_INET;
        if ( inet_pton(AF_INET, addr, &(sin.sin_addr)) <= 0 )
            sin.sin_addr.s_addr = htonl(INADDR_ANY);
        sin.sin_port = htons(options.port);
#endif
        /* Bind the socket to the desired port. */
        if (bind(listen_sock, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
            error("bind: %.100s", strerror(errno));
            shutdown(listen_sock, 2);
            close(listen_sock);
            fatal("Bind to port %d failed: %.200s.", options.port, strerror(errno));
        }
        /* COMMAN : setuid to bbs */

        if(setgid(BBSGID)==-1)
            exit(8);
        if(setuid(BBSUID)==-1)
            exit(8);

#if 0 /* etnlegend, 2006.10.31 ... */
        if (!debug_flag) {
            /* Record our pid in /etc/sshd_pid to make it easier to kill the
               correct sshd.  We don\'t want to do this before the bind above
               because the bind will fail if there already is a daemon, and this
               will overwrite any old pid in the file. */
            f = fopen(options.pid_file, "w");
            if (f) {
                fprintf(f, "%u\n", (unsigned int) getpid());
                fclose(f);
            }
        }
#endif

        /* Start listening on the port. */
        log_msg("Server listening on port %d.", options.port);
        if (listen(listen_sock, 5) < 0)
            fatal("listen: %.100s", strerror(errno));

        /* Generate an rsa key. */
        log_msg("Generating %d bit RSA key.", options.server_key_bits);
        rsa_generate_key(&sensitive_data.private_key, &public_key, &sensitive_data.random_state, options.server_key_bits);
        random_save(&sensitive_data.random_state, geteuid(), options.random_seed_file);
        log_msg("RSA key generation complete.");

        /* Schedule server key regeneration alarm. */
        signal(SIGALRM, key_regeneration_alarm);
        alarm(options.key_regeneration_time);

        /* Arrange to restart on SIGHUP.  The handler needs listen_sock. */
        signal(SIGHUP, sighup_handler);
        signal(SIGTERM, sigterm_handler);
        signal(SIGQUIT, sigterm_handler);

        /* AIX sends SIGDANGER when memory runs low.  The default action is
           to terminate the process.  This sometimes makes it difficult to
           log in and fix the problem. */

#ifdef SIGDANGER
        signal(SIGDANGER, sigdanger_handler);
#endif                          /* SIGDANGER */

        /* Arrange SIGCHLD to be caught. */
        signal(SIGCHLD, main_sigchld_handler);


        if(!debug_flag){
            if(!addr[0])
                sprintf(buf,"var/sshbbsd.%d.pid",options.port);
            else
                sprintf(buf,"var/sshbbsd.%d_%s.pid",options.port,addr);
            if((f=fopen(buf,"w"))){
                fprintf(f,"%d\n",(int)getpid());
                fclose(f);
            }
        }

        /* Stay listening for connections until the system crashes or the
           daemon is killed with a signal. */
        for (;;) {
            if (received_sighup)
                sighup_restart();

            /* Wait in select until there is a connection. */
            FD_ZERO(&fdset);
            FD_SET(listen_sock, &fdset);
            ret = select(listen_sock + 1, &fdset, NULL, NULL, NULL);
            if (ret < 0 || !FD_ISSET(listen_sock, &fdset)) {
                if (errno == EINTR)
                    continue;
                error("select: %.100s", strerror(errno));
                continue;
            }

            aux = sizeof(sin);
            newsock = accept(listen_sock, (struct sockaddr *) &sin, &aux);
            if (newsock < 0) {
                if (errno == EINTR)
                    continue;
                error("accept: %.100s", strerror(errno));
                continue;
            }

            /* Got connection.  Fork a child to handle it, unless we are in
               debugging mode. */
            if (debug_flag) {
                /* In debugging mode.  Close the listening socket, and start
                   processing the connection without forking. */
                debug("Server will not fork when running in debugging mode.");
                close(listen_sock);
                sock_in = newsock;
                sock_out = newsock;
                pid = getpid();
#ifdef LIBWRAP
                {
                    struct request_info req;

                    signal(SIGCHLD, SIG_DFL);

                    request_init(&req, RQ_DAEMON, av0, RQ_FILE, newsock, NULL);
                    fromhost(&req);
                    if (!hosts_access(&req))
                        refuse(&req);
                    syslog(allow_severity, "connect from %s", eval_client(&req));
                }
#endif                          /* LIBWRAP */
                break;
            } else {
#ifdef CHECK_IP_LINK
#ifdef HAVE_IPV6_SMTH
                if (check_IP_lists(sin.sin6_addr)==0)
#else
                if (check_IP_lists(sin.sin_addr.s_addr)==0) 
#endif
#endif
                /* Normal production daemon.  Fork, and have the child process
                   the connection.  The parent continues listening. */
                if ((pid = fork()) == 0) {
                    /* Child.  Close the listening socket, and start using
                       the accepted socket.  Reinitialize logging (since our
                       pid has changed).  We break out of the loop to handle
                       the connection. */
                    close(listen_sock);
                    sock_in = newsock;
                    sock_out = newsock;
#ifdef LIBWRAP
                    {
                        struct request_info req;

                        signal(SIGCHLD, SIG_DFL);

                        request_init(&req, RQ_DAEMON, av0, RQ_FILE, newsock, NULL);
                        fromhost(&req);
                        if (!hosts_access(&req))
                            refuse(&req);
                        syslog(allow_severity, "connect from %s", eval_client(&req));
                    }
#endif                          /* LIBWRAP */

                    log_init(av0, debug_flag && !inetd_flag, options.fascist_logging || debug_flag, options.quiet_mode, options.log_facility);
                    break;
                }
            }

            /* Parent.  Stay in the loop. */
            if (pid < 0)
                error("fork: %.100s", strerror(errno));
            else
                debug("Forked child %d.", pid);

            /* Mark that the key has been used (it was "given" to the child). */
            key_used = 1;

            random_acquire_light_environmental_noise(&sensitive_data.random_state);

            /* Close the new socket (the child is now taking care of it). */
            close(newsock);
        }
    }

    /* This is the child processing a new connection. */

    /* Disable the key regeneration alarm.  We will not regenerate the key
       since we are no longer in a position to give it to anyone.  We will
       not restart on SIGHUP since it no longer makes sense. */
    alarm(0);
    signal(SIGALRM, SIG_DFL);
    signal(SIGHUP, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGCHLD, SIG_DFL);

    /* Set socket options for the connection.  We want the socket to close
       as fast as possible without waiting for anything.  If the connection
       is not a socket, these will do nothing. */
    /* setsockopt(sock_in, SOL_SOCKET, SO_REUSEADDR, (void *)&on, sizeof(on)); */
#if defined(SO_LINGER) && defined(ENABLE_SO_LINGER)
    linger.l_onoff = 1;
    linger.l_linger = 15;
    setsockopt(sock_in, SOL_SOCKET, SO_LINGER, (void *) &linger, sizeof(linger));
#endif                          /* SO_LINGER */

    /* Register our connection.  This turns encryption off because we do not
       have a key. */
    packet_set_connection(sock_in, sock_out, &sensitive_data.random_state);

    /* Log the connection. */
    log_msg("Connection from %.100s port %d", get_remote_ipaddr(), get_remote_port());

    /* Check whether logins are denied from this host. */
    {
        const char *hostname = get_canonical_hostname();
        const char *ipaddr = get_remote_ipaddr();
        int i;

        if (options.num_deny_hosts > 0) {
            for (i = 0; i < options.num_deny_hosts; i++)
                if (match_host(hostname, ipaddr, options.deny_hosts[i]))
                    perm_denied = 1;
        }
        if ((!perm_denied) && options.num_allow_hosts > 0) {
            for (i = 0; i < options.num_allow_hosts; i++)
                if (match_host(hostname, ipaddr, options.allow_hosts[i]))
                    break;
            if (i >= options.num_allow_hosts)
                perm_denied = 1;
        }
        if (perm_denied && options.silent_deny) {
            close(sock_in);
            close(sock_out);
            exit(0);
        }
    }

    /* We don't want to listen forever unless the other side successfully
       authenticates itself.  So we set up an alarm which is cleared after
       successful authentication.  A limit of zero indicates no limit.
       Note that we don't set the alarm in debugging mode; it is just annoying
       to have the server exit just when you are about to discover the bug. */
    signal(SIGALRM, grace_alarm_handler);
    if (!debug_flag)
        alarm(options.login_grace_time);


    if (ssh_remote_version_string == NULL) {
        /* Send our protocol version identification. */
        snprintf(buf, sizeof(buf), "SSH-%d.%d-%.50s", PROTOCOL_MAJOR, PROTOCOL_MINOR, SSH_VERSION);
        strcat(buf, "\n");
        if (write(sock_out, buf, strlen(buf)) != strlen(buf))
            fatal_severity(SYSLOG_SEVERITY_INFO, "Could not write ident string.");
    }

    if (ssh_remote_version_string == NULL) {
        /* Read other side\'s version identification. */
        for (i = 0; i < sizeof(buf) - 1; i++) {
            if (read(sock_in, &buf[i], 1) != 1)
                fatal_severity(SYSLOG_SEVERITY_INFO, "Did not receive ident string.");
            if (buf[i] == '\r') {
                buf[i] = '\n';
                buf[i + 1] = 0;
                break;
            }
            if (buf[i] == '\n') {
                /* buf[i] == '\n' */
                buf[i + 1] = 0;
                break;
            }
        }
        buf[sizeof(buf) - 1] = 0;
    } else {
        strncpy(buf, ssh_remote_version_string, sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = 0;
    }

    /* Check that the versions match.  In future this might accept several
       versions and set appropriate flags to handle them. */
    if (sscanf(buf, "SSH-%d.%d-%[^\n]\n", &remote_major, &remote_minor, remote_version) != 3) {
        const char *s = "Protocol mismatch.\n";

        (void) write(sock_out, s, strlen(s));
        close(sock_in);
        close(sock_out);
        fatal_severity(SYSLOG_SEVERITY_INFO, "Bad protocol version identification: %.100s", buf);
    }
    debug("Client protocol version %d.%d; client software version %.100s", remote_major, remote_minor, remote_version);

    switch (check_emulation(remote_major, remote_minor, NULL, NULL)) {
    case EMULATE_MAJOR_VERSION_MISMATCH:
        {
            const char *s = "Protocol major versions differ.\n";

            (void) write(sock_out, s, strlen(s));
            close(sock_in);
            close(sock_out);
            fatal_severity(SYSLOG_SEVERITY_INFO, "Protocol major versions differ: %d vs. %d", PROTOCOL_MAJOR, remote_major);
        }
        break;
    case EMULATE_VERSION_TOO_OLD:
        packet_disconnect("Your ssh version is too old and is no " "longer supported.  Please install a newer version.");
        break;
    case EMULATE_VERSION_NEWER:
        packet_disconnect("This server does not support your " "new ssh version.");
        break;
    case EMULATE_VERSION_OK:
        break;
    default:
        fatal("Unexpected return value from check_emulation.");
    }

    if (perm_denied) {
        const char *hostname = get_canonical_hostname();

        log_msg("Connection from %.200s not allowed.\n", hostname);
        packet_disconnect("Sorry, you are not allowed to connect.");
     /*NOTREACHED*/}

    packet_set_nonblocking();

    /* Handle the connection.   We pass as argument whether the connection
       came from a privileged port. */
    do_connection(get_remote_port() < 1024);

    /* The connection has been terminated. */
    log_msg("Closing connection to %.100s", get_remote_ipaddr());
    packet_close();
    exit(0);
}

/* Process an incoming connection.  Protocol version identifiers have already
   been exchanged.  This sends server key and performs the key exchange.
   Server and host keys will no longer be needed after this functions. */

void do_connection(int privileged_port)
{
    int i;
    MP_INT session_key_int;
    unsigned char session_key[SSH_SESSION_KEY_LENGTH];
    unsigned char check_bytes[8];
    char *user;
    unsigned int cipher_type, auth_mask, protocol_flags;

    /* Generate check bytes that the client must send back in the user packet
       in order for it to be accepted; this is used to defy ip spoofing 
       attacks.  Note that this only works against somebody doing IP spoofing 
       from a remote machine; any machine on the local network can still see 
       outgoing packets and catch the random cookie.  This only affects
       rhosts authentication, and this is one of the reasons why it is
       inherently insecure. */
    for (i = 0; i < 8; i++)
        check_bytes[i] = random_get_byte(&sensitive_data.random_state);

    /* Send our public key.  We include in the packet 64 bits of random
       data that must be matched in the reply in order to prevent IP spoofing. */
    packet_start(SSH_SMSG_PUBLIC_KEY);
    for (i = 0; i < 8; i++)
        packet_put_char(check_bytes[i]);

    /* Store our public server RSA key. */
    packet_put_int(public_key.bits);
    packet_put_mp_int(&public_key.e);
    packet_put_mp_int(&public_key.n);

    /* Store our public host RSA key. */
    packet_put_int(sensitive_data.host_key.bits);
    packet_put_mp_int(&sensitive_data.host_key.e);
    packet_put_mp_int(&sensitive_data.host_key.n);

    /* Put protocol flags. */
    packet_put_int(SSH_PROTOFLAG_HOST_IN_FWD_OPEN);

    /* Declare which ciphers we support. */
    packet_put_int(cipher_mask());

    /* Declare supported authentication types. */
    auth_mask = 0;
    if (options.rhosts_authentication)
        auth_mask |= 1 << SSH_AUTH_RHOSTS;
    if (options.rhosts_rsa_authentication)
        auth_mask |= 1 << SSH_AUTH_RHOSTS_RSA;
    if (options.rsa_authentication)
        auth_mask |= 1 << SSH_AUTH_RSA;
    if (options.password_authentication)
        auth_mask |= 1 << SSH_AUTH_PASSWORD;
    packet_put_int(auth_mask);

    /* Send the packet and wait for it to be sent. */
    packet_send();
    packet_write_wait();

    debug("Sent %d bit public key and %d bit host key.", public_key.bits, sensitive_data.host_key.bits);

    /* Read clients reply (cipher type and session key). */
    packet_read_expect(SSH_CMSG_SESSION_KEY);

    /* Get cipher type. */
    cipher_type = packet_get_char();

    /* Get check bytes from the packet.  These must match those we sent earlier
       with the public key packet. */
    for (i = 0; i < 8; i++)
        if (check_bytes[i] != packet_get_char())
            packet_disconnect("IP Spoofing check bytes do not match.");

    debug("Encryption type: %.200s", cipher_name(cipher_type));

    /* Get the encrypted integer. */
    mpz_init(&session_key_int);
    packet_get_mp_int(&session_key_int);

    /* Get protocol flags. */
    protocol_flags = packet_get_int();
    packet_set_protocol_flags(protocol_flags);

    /* Decrypt it using our private server key and private host key (key with 
       larger modulus first). */
    if (mpz_cmp(&sensitive_data.private_key.n, &sensitive_data.host_key.n) > 0) {
        /* Private key has bigger modulus. */
        assert(sensitive_data.private_key.bits >= sensitive_data.host_key.bits + SSH_KEY_BITS_RESERVED);
        rsa_private_decrypt(&session_key_int, &session_key_int, &sensitive_data.private_key);
        rsa_private_decrypt(&session_key_int, &session_key_int, &sensitive_data.host_key);
    } else {
        /* Host key has bigger modulus (or they are equal). */
        assert(sensitive_data.host_key.bits >= sensitive_data.private_key.bits + SSH_KEY_BITS_RESERVED);
        rsa_private_decrypt(&session_key_int, &session_key_int, &sensitive_data.host_key);
        rsa_private_decrypt(&session_key_int, &session_key_int, &sensitive_data.private_key);
    }

    /* Compute session id for this session. */
    compute_session_id(session_id, check_bytes, sensitive_data.host_key.bits, &sensitive_data.host_key.n, sensitive_data.private_key.bits, &sensitive_data.private_key.n);

    /* Extract session key from the decrypted integer.  The key is in the 
       least significant 256 bits of the integer; the first byte of the 
       key is in the highest bits. */
    mp_linearize_msb_first(session_key, sizeof(session_key), &session_key_int);

    /* Xor the first 16 bytes of the session key with the session id. */
    for (i = 0; i < 16; i++)
        session_key[i] ^= session_id[i];

    /* Destroy the decrypted integer.  It is no longer needed. */
    mpz_clear(&session_key_int);

    /* Set the session key.  From this on all communications will be
       encrypted. */
    packet_set_encryption_key(session_key, SSH_SESSION_KEY_LENGTH, cipher_type, 0);

    /* Destroy our copy of the session key.  It is no longer needed. */
    memset(session_key, 0, sizeof(session_key));

    debug("Received session key; encryption turned on.");

    /* Send an acknowledgement packet.  Note that this packet is sent
       encrypted. */
    packet_start(SSH_SMSG_SUCCESS);
    packet_send();
    packet_write_wait();

    /* Get the name of the user that we wish to log in as. */
    packet_read_expect(SSH_CMSG_USER);

    /* Get the user name. */
    user = packet_get_string(NULL);

    /* Destroy the private and public keys.  They will no longer be needed. */
    rsa_clear_public_key(&public_key);
    rsa_clear_private_key(&sensitive_data.private_key);
    rsa_clear_private_key(&sensitive_data.host_key);

    /* Do the authentication. */
    do_authentication(user, privileged_port, cipher_type);
}

/* Fails all authentication requests */
void do_authentication_fail_loop(void)
{
    /* The user does not exist. */
    packet_start(SSH_SMSG_FAILURE);
    packet_send();
    packet_write_wait();

    /* Keep reading packets, and always respond with a failure.  This is to
       avoid disclosing whether such a user really exists. */
    for (;;) {
        /* Read a packet.  This will not return if the client disconnects. */
        (void) packet_read();
        packet_get_all();

        /* Send failure.  This should be indistinguishable from a failed
           authentication. */
        packet_start(SSH_SMSG_FAILURE);
        packet_send();
        packet_write_wait();
    }
     /*NOTREACHED*/ abort();
}

/* Performs authentication of an incoming connection.  Session key has already
   been exchanged and encryption is enabled.  User is the user name to log
   in as (received from the clinet).  Privileged_port is true if the
   connection comes from a privileged port (used for .rhosts authentication).*/

void do_authentication(char *user, int privileged_port, int cipher_type)
{
    int type;
    int authenticated = 0;
    int authentication_type = 0;
    char *password;
    int row, col, xpixel, ypixel;
    int password_attempts = 0;

    if (strlen(user) > 255)
        do_authentication_fail_loop();

    /* Verify that the user is a valid user.  We disallow usernames starting
       with any characters that are commonly used to start NIS entries. */
    if (user[0] == '-' || user[0] == '+' || user[0] == '@')
        do_authentication_fail_loop();

    debug("Attempting authentication for %.100s.", user);

    /* If the user has no password, accept authentication immediately. */
    if (auth_password(user, "")) {
        /* Authentication with empty password succeeded. */
        authentication_type = SSH_AUTH_PASSWORD;
        authenticated = 1;
        /* Success packet will be sent after loop below. */
    } else {
        /* Indicate that authentication is needed. */
        packet_start(SSH_SMSG_FAILURE);
        packet_send();
        packet_write_wait();
    }

    /* Loop until the user has been authenticated or the connection is closed. */
    while (!authenticated) {
        /* Get a packet from the client. */
        type = packet_read();

        /* Process the packet. */
        switch (type) {

        case SSH_CMSG_AUTH_RHOSTS:
            packet_get_all();
            log_msg("Rhosts authentication disabled.");
            break;

        case SSH_CMSG_AUTH_RHOSTS_RSA:
            packet_get_all();
            log_msg("Rhosts with RSA authentication disabled.");
            break;

        case SSH_CMSG_AUTH_RSA:
            packet_get_all();
            log_msg("RSA authentication disabled.");
            break;

        case SSH_CMSG_AUTH_PASSWORD:
            if (cipher_type == SSH_CIPHER_NONE) {
                packet_get_all();
                log_msg("Password authentication not available for unencrypted session.");
                break;
            }

            /* Password authentication requested. */
            /* Read user password.  It is in plain text, but was transmitted
               over the encrypted channel so it is not visible to an outside
               observer. */
            password = packet_get_string(NULL);

            if (password_attempts >= 5) {       /* Too many password authentication attempts. */
                packet_disconnect("Too many password authentication attempts from %.100s for user %.100s.", get_canonical_hostname(), user);
             /*NOTREACHED*/}

            /* Count password authentication attempts, and log if appropriate. */
            if (password_attempts > 0) {
                /* Log failures if attempted more than once. */
                debug("Password authentication failed for user %.100s from %.100s.", user, get_canonical_hostname());
            }
            password_attempts++;

            /* Try authentication with the password. */
            if (auth_password(user, password)) {
                /* Successful authentication. */
                /* Clear the password from memory. */
                memset(password, 0, strlen(password));
                xfree(password);
                log_msg("Password authentication for %.100s accepted.", user);
                authentication_type = SSH_AUTH_PASSWORD;
                authenticated = 1;
                break;
            }
            debug("Password authentication for %.100s failed.", user);
            memset(password, 0, strlen(password));
            xfree(password);
            break;

        case SSH_CMSG_WINDOW_SIZE:
            debug("Window change received.");
            row = packet_get_int();
            col = packet_get_int();
            xpixel = packet_get_int();
            ypixel = packet_get_int();
            do_naws(row, col);
            break;
            
        default:
            /* Any unknown messages will be ignored (and failure returned)
               during authentication. */
            packet_get_all();
            log_msg("Unknown message during authentication: type %d", type);
            break;              /* Respond with a failure message. */
        }
        /* If successfully authenticated, break out of loop. */
        if (authenticated)
            break;

        /* Send a message indicating that the authentication attempt failed. */
        packet_start(SSH_SMSG_FAILURE);
        packet_send();
        packet_write_wait();
    }


    /* The user has been authenticated and accepted. */
    packet_start(SSH_SMSG_SUCCESS);
    packet_send();
    packet_write_wait();

    /* Perform session preparation. */
    do_authenticated(NULL);
}

/* Prepares for an interactive session.  This is called after the user has
   been successfully authenticated.  During this message exchange, pseudo
   terminals are allocated, X11, TCP/IP, and authentication agent forwardings
   are requested, etc. */

void do_authenticated(char *pw)
{
    int type;
    int compression_level = 0, enable_compression_after_reply = 0;
    int row, col, xpixel, ypixel;
    unsigned long max_size;
    char *display = NULL, *proto = NULL, *data = NULL;

    /* Cancel the alarm we set to limit the time taken for authentication. */
    alarm(0);

    /* Inform the channel mechanism that we are the server side and that
       the client may request to connect to any port at all.  (The user could
       do it anyway, and we wouldn\'t know what is permitted except by the
       client telling us, so we can equally well trust the client not to request
       anything bogus.) */

    /* We stay in this loop until the client requests to execute a shell or a
       command. */
    while (1) {
        /* Get a packet from the client. */
        type = packet_read();

        /* Process the packet. */
        switch (type) {
        case SSH_CMSG_REQUEST_COMPRESSION:
            /* COMMAN: k core said that compression is not useful */
            goto fail;
            compression_level = packet_get_int();
            if (compression_level < 1 || compression_level > 9) {
                packet_send_debug("Received illegal compression level %d.", compression_level);
                goto fail;
            }
            /* Enable compression after we have responded with SUCCESS. */
            enable_compression_after_reply = 1;
            break;

        case SSH_CMSG_MAX_PACKET_SIZE:
            /* Get maximum size from paket. */
            max_size = packet_get_int();

            /* Make sure that it is acceptable. */
            if (max_size < 4096 || max_size > 256 * 1024) {
                packet_send_debug("Received illegal max packet size %lu.", max_size);
                goto fail;
            }

            /* Set the size and return success. */
            packet_set_max_size(max_size);
            break;

        case SSH_CMSG_REQUEST_PTY:
            packet_get_string(NULL);
            row = packet_get_int();
            col = packet_get_int();
            xpixel = packet_get_int();
            ypixel = packet_get_int();
            do_naws(row, col);
            packet_get_all();
            debug("Allocating a pty not permitted for this authentication.");
            break;

        case SSH_CMSG_X11_REQUEST_FORWARDING:
            packet_get_all();
            debug("X11 forwarding disabled in this site.");
            packet_send_debug("X11 forwarding disabled in this site.");
            goto fail;

        case SSH_CMSG_AGENT_REQUEST_FORWARDING:
            packet_get_all();
            debug("Authentication agent forwarding not permitted for this authentication.");
            goto fail;
        case SSH_CMSG_PORT_FORWARD_REQUEST:
            packet_get_all();
            debug("All port forwardings disabled in this site.");
            packet_send_debug("All port forwardings disabled in this site.");
            goto fail;

        case SSH_CMSG_EXEC_SHELL:
            /* Set interactive/non-interactive mode. */
            packet_set_interactive(1, options.keepalives);

            if (forced_command != NULL)
                goto do_forced_command;
            debug("Forking shell.");
            do_exec_no_pty(NULL, pw, display, proto, data);
            return;

        case SSH_CMSG_EXEC_CMD:
            packet_get_all();
            debug("command executing disabled in this site.");
            packet_send_debug("command executing disabled in this site.");
            goto fail;

        case SSH_CMSG_WINDOW_SIZE:
            debug("Window change received.");
            row = packet_get_int();
            col = packet_get_int();
            xpixel = packet_get_int();
            ypixel = packet_get_int();
            do_naws(row, col);
            break;

        default:
            /* Any unknown messages in this phase are ignored, and a failure
               message is returned. */
            packet_get_all();
            log_msg("Unknown packet type received after authentication: %d", type);
            goto fail;
        }

        /* The request was successfully processed. */
        packet_start(SSH_SMSG_SUCCESS);
        packet_send();
        packet_write_wait();

        /* Enable compression now that we have replied if appropriate. */
        if (enable_compression_after_reply) {
            enable_compression_after_reply = 0;
            packet_start_compression(compression_level);
        }

        continue;

      fail:
        /* The request failed. */
        packet_get_all();
        packet_start(SSH_SMSG_FAILURE);
        packet_send();
        packet_write_wait();
        continue;

      do_forced_command:
        /* There is a forced command specified for this login.  Execute it. */
        debug("Executing forced command: %.900s", forced_command);
        return;
    }
}

/* This is called to fork and execute a command when we have no tty.  This
   will call do_child from the child, and server_loop from the parent after
   setting up file descriptors and such. */
void do_exec_no_pty(const char *command, char *pw, const char *display, const char *auth_proto, const char *auth_data)
{
    ssh_init();
    chdir(BBSHOME);
    dup2(packet_get_connection_in(), 0);
    bbs_entry();
    exit(0);
}
void ssh_end(void)
{
    packet_disconnect("sshd exit");
}
