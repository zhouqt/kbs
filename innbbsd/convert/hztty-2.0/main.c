/* $Id$ */

/*
 * hztty -- version 2.0
 *
 * This program turns a tty session from one encoding to another.
 * For example, running hztty on cxterm can allow you to read/write
 * Chinese in HZ format, which was not supported by cxterm.
 * If you have many applications in different encodings but your
 * favor terminal program only supports one, hztty can make life easy.
 * For example, hztty can your GB cxterm into a HZ terminal, a
 * Unicode (16bit, or UTF8, or UTF7) terminal, or a Big5 terminal.
 * 
 * The idea is to open a new shell session on top of the current one
 * and to translate the encoding between the new tty and the orignal.
 * For example, if your application uses encoding A and your terminal
 * supports encoding B.  Hztty catches the output of the application
 * and converts them from A to B before sending to the terminal.
 * Similarly, hztty converts all the terminal input from B to A before
 * sending to the application.
 *
 * The conversion is implemented in a configurable I/O stream style.
 * Conversion modules are specified in command line options "-O" (for
 * output) and "-I" (for input).  In each direction, conversion modules
 * can be piped one to one using connection character ':'.  For example,
 * specifying "hz2gb:gb2big" in output stream translate the cxterm screen
 * output from zW/HZ to GB, then from GB to Big5.  
 *
 * One interesting application is to emulate a 16bit Unicode terminal
 * on a GB or Big5 terminal.  Although some information is lost (since
 * Unicode is a superset), you get a reasonable environment to try
 * running your Unicode program.
 * 
 *	Yongguang ZHANG 		(ygz@cs.purdue.edu)
 *	Purdue University		August 4, 1992
 * 
 *	Yongguang ZHANG 		(ygz@isl.hrl.hac.com)
 *	Hughes Research	Labs		January 28, 1995
 */

/*
 * Copyright 1992,1995 by Yongguang Zhang
 */

#ifndef lint
static char *rcs_id = "$Id$";
#endif                          /* lint */

#include "config.h"
#include "io.h"

extern int get_pty();
extern int get_tty();
extern void get_term_mode();
extern void set_term_mode();
extern void make_raw();
extern void addutmp();
extern void rmutmp();

static int loginsh = 0;
static int utmp = 1;            /* update utmp by default */
static char *term = "vt100";

int master;
int slave;
int child;
int subchild = 0;
int in_raw = 0;
char *i_stream = NULL;
char *o_stream = NULL;
int debug = 0;                  /* the debug flag */

static char *shell[] = { "sh", "-i", (char *) 0 };
static char **cmdargv;
static char *cmd;
static char *progname;

static struct term_mode defmode, rawmode;

static void usage();
static void getmaster();
static void getslave();
static void doinput();
static void dooutput();
static void doshell();

#ifdef WINSIZE
static struct WINSIZE winsz;
static SIGNAL_T sigwinch();
#endif
static SIGNAL_T finish();
static void fail();
static void done();


main(argc, argv)
int argc;
char *argv[];
{
    extern int getopt();
    extern char *optarg;
    extern int optind;
    int ch;

    progname = *argv;
    while ((ch = getopt(argc, argv, "I:O:T:ludh")) != EOF)
        switch (ch) {
        case 'I':
            i_stream = optarg;
            break;
        case 'O':
            o_stream = optarg;
            break;
        case 'T':
            term = optarg;
            break;
        case 'l':
            loginsh = 1;
            break;
        case 'u':
            utmp = 0;           /* disable utmp */
            break;
        case 'd':
            debug = 1;
            break;
        case 'h':
        case '?':
        default:
            usage();
        }
    argc -= optind;
    argv += optind;

    if ((!i_stream) && (!o_stream)) {
        i_stream = DEF_ISTREAM;
        o_stream = DEF_OSTREAM;
    }

    if (argc == 0) {
        cmdargv = shell;
        cmd = (char *) getenv("SHELL");
        if (!cmd)
            cmd = "/bin/sh";
    } else {
        cmdargv = argv;
        cmd = *argv;
    }
    if (loginsh)
        *cmdargv = "-";

    getmaster();

    get_term_mode(0, &defmode);
    make_raw(&defmode, &rawmode);
    set_term_mode(0, &rawmode);
    in_raw = 1;

    (void) signal(SIGCHLD, finish);
    child = fork();
    if (child < 0) {
        perror("fork");
        fail();
    }
    if (child == 0) {
        (void) signal(SIGCHLD, finish);
        subchild = child = fork();
        if (child < 0) {
            perror("fork");
            fail();
        }
        if (child == 0) {
            doshell();
        }
#if defined(SIGWINCH) && defined(WINSIZE)
        (void) signal(SIGWINCH, sigwinch);
#endif
        dooutput();
    }
    doinput();
    exit(0);
}

static void usage()
{
    struct mod_def *pmod = moduleTable;

    fprintf(stderr, "usage: %s [ -lu ] [-T term] [-I input_steams] [-O output_streams] [command ...]\n", progname);
    fprintf(stderr, "\n");
    fprintf(stderr, "input/output streams:   module[:module[...]]\n");
    fprintf(stderr, "conversion module supported:\n");
    while (pmod->name) {
        fprintf(stderr, "   %s%s\t\t%s\n", pmod->name, pmod->needarg ? "(arg)" : "", pmod->annotation);
        pmod++;
    }
    exit(1);
}


/* stdin --> pty */
static void doinput()
{
    register int cc;
    char ibuf[BUFSIZ];

    if (in_stream_setup(i_stream) < 0)
        fail();
    for (;;) {
        cc = stream_read(0, ibuf, BUFSIZ);
        if (cc < 0)
            break;
        if (cc > 0)
            (void) write(master, ibuf, cc);
    }
    done();
}

/* pty --> stdout */
static void dooutput()
{
    register int cc;
    char obuf[BUFSIZ];

    if (out_stream_setup(o_stream) < 0)
        fail();
    (void) close(0);
    for (;;) {
        cc = read(master, obuf, sizeof(obuf));
        if (cc <= 0)
            break;
        (void) stream_write(1, obuf, cc);
    }
    done();
}

/* tty <-> shell */
static void doshell()
{
    char envbuf[20];

    getslave();
    (void) close(master);
    (void) dup2(slave, 0);
    (void) dup2(slave, 1);
    (void) dup2(slave, 2);
    (void) close(slave);
    if (utmp)
        addutmp();
#if defined(sequent) || defined(__convex__)
    setenv("TERM", term, 1);
#else
    sprintf(envbuf, "TERM=%s", term);
    putenv(envbuf);
#endif
    printf("[%s started]  [using %s]\n", progname, ttyname(0));
    sleep(1);

    /*
     * now execute the shell command 
     */
    execvp(cmd, cmdargv);

    perror(cmd);
    fail();
}

static SIGNAL_T finish()
{
#if defined(SYSV) || defined(POSIX)
    int status;
#else
    union wait status;
#endif
    register int pid;
    register int die = 0;

#if defined(SYSV) || defined(POSIX)
    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0)
#else
    while ((pid = wait3(&status, WNOHANG | WUNTRACED, (struct rusage *) 0)) > 0)
#endif
        if (pid == child)
            die = 1;

    fflush(stdout);
    if (die)
        done();

    SIGNAL_RETURN;
}

static void fail()
{
    if (child) {
        kill(child, SIGTERM);
        sleep(1);               /* wait for the child to die */
        kill(child, SIGKILL);
    }
    done();
}

static void done()
{
    if (subchild) {
        (void) close(master);
        if (utmp)
            rmutmp(subchild);
    } else {
        if (in_raw)
            set_term_mode(0, &defmode);
        printf("\n[%s exited]\n", progname);
    }
    exit(0);
}

static void getmaster()
{
    if (get_pty(&master) != 0) {
        fprintf(stderr, "Out of pty's\n");
        fail();
    }
#ifdef	GETWINSZ
    (void) ioctl(0, GETWINSZ, &winsz);
#endif
}

static void getslave()
{
    if (get_tty(master, &slave) != 0) {
        fprintf(stderr, "Fail to open tty\n");
        fail();
    }
#ifdef	SETWINSZ
    (void) ioctl(slave, SETWINSZ, &winsz);
#endif
    set_term_mode(slave, &defmode);
}

#ifdef WINSIZE
static SIGNAL_T sigwinch()
{
    struct WINSIZE ws;

    if (ioctl(1, GETWINSZ, &ws) != 0)
        SIGNAL_RETURN;
    (void) ioctl(master, SETWINSZ, &ws);
#ifdef notdef                   /* SIGWINCH */
    {
        int pgrp;

        if (ioctl(master, TIOCGPGRP, (char *) &pgrp))
            killpg(pgrp, SIGWINCH);
    }
#endif
    SIGNAL_RETURN;
}
#endif
