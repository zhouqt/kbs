/*

userfile.c

Author: Tatu Ylonen <ylo@cs.hut.fi>

Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
                   All rights reserved

Created: Wed Jan 24 20:19:53 1996 ylo

*/

/* Protocol for communication between the child and the parent: 

      Each message starts with a 32-bit length (msb first; includes
      type but not length itself), followed by one byte containing
      the packet type.

        1 USERFILE_OPEN
          string        file name
          int32         flags
          int32         mode

        2 USERFILE_OPEN_REPLY
          int32         handle (-1 if error)

        3 USERFILE_READ
          int32         handle
          int32         max_bytes

        4 USERFILE_READ_REPLY
          string        data      ;; empty data means EOF

        5 USERFILE_WRITE
          int32         handle
          string        data

        6 USERFILE_WRITE_REPLY
          int32         bytes_written  ;; != length of data means error
        
        7 USERFILE_CLOSE
          int32         handle

        8 USERFILE_CLOSE_REPLY
          int32         return value

        9 USERFILE_LSEEK
          int32         handle
          int32         offset
          int32         whence

       10 USERFILE_LSEEK_REPLY
          int32         returned_offset

       11 USERFILE_MKDIR
          string        path
          int32         mode

       12 USERFILE_MKDIR_REPLY
          int32         return value

       13 USERFILE_STAT
          string        path

       14 USERFILE_STAT_REPLY
          int32         return value
          sizeof(struct stat) binary bytes (in host order and layout)

       15 USERFILE_REMOVE
          string        path

       16 USERFILE_REMOVE_REPLY
          int32         return value

       17 USERFILE_POPEN
          string        command
          string        type

       18 USERFILE_POPEN_REPLY
          int32         handle (-1 if error)

       19 USERFILE_PCLOSE
          int32         handle

       20 USERFILE_PCLOSE_REPLY
          int32         return value

       21 USERFILE_GET_DES_1_MAGIC_PHRASE

       22 USERFILE_GET_DES_1_MAGIC_PHRASE_REPLY
          string        data
          
          */

#include "includes.h"
#include <gmp.h>
#include "userfile.h"
#include "getput.h"
#include "buffer.h"
#include "bufaux.h"
#include "xmalloc.h"
#include "ssh.h"

/* Protocol message types. */
#define USERFILE_OPEN           1
#define USERFILE_OPEN_REPLY     2
#define USERFILE_READ           3
#define USERFILE_READ_REPLY     4
#define USERFILE_WRITE          5
#define USERFILE_WRITE_REPLY    6
#define USERFILE_CLOSE          7
#define USERFILE_CLOSE_REPLY    8
#define USERFILE_LSEEK          9
#define USERFILE_LSEEK_REPLY   10
#define USERFILE_MKDIR         11
#define USERFILE_MKDIR_REPLY   12
#define USERFILE_STAT          13
#define USERFILE_STAT_REPLY    14
#define USERFILE_REMOVE        15
#define USERFILE_REMOVE_REPLY  16
#define USERFILE_POPEN         17
#define USERFILE_POPEN_REPLY   18
#define USERFILE_PCLOSE        19
#define USERFILE_PCLOSE_REPLY  20
#define USERFILE_GET_DES_1_MAGIC_PHRASE        21
#define USERFILE_GET_DES_1_MAGIC_PHRASE_REPLY  22


/* Flag indicating whether we have forked. */

/* Forks and execs the given command.  Returns a file descriptor for
   communicating with the program, or -1 on error.  The program will
   be run with empty environment to avoid LD_LIBRARY_PATH and similar
   attacks. */

int do_popen(const char *command, const char *type)
{
    int fds[2];
    int pid, i, j;
    char *args[100];
    char *env[100];
    extern char **environ;

    if (pipe(fds) < 0)
        fatal("pipe: %.100s", strerror(errno));

    pid = fork();
    if (pid < 0)
        fatal("fork: %.100s", strerror(errno));

    if (pid == 0) {             /* Child */


        /* Set up file descriptors. */
        if (type[0] == 'r') {
            if (dup2(fds[1], 1) < 0)
                perror("dup2 1");
        } else {
            if (dup2(fds[0], 0) < 0)
                perror("dup2 0");
        }
        close(fds[0]);
        close(fds[1]);

        /* Build argument vector. */
        i = 0;
        args[i++] = "/bin/sh";
        args[i++] = "-c";
        args[i++] = (char *) command;
        args[i++] = NULL;

        /* Prune environment to remove any potentially dangerous variables. */
        i = 0;
        for (j = 0; environ[j] && i < sizeof(env) / sizeof(env[0]) - 1; j++)
            if (strncmp(environ[j], "HOME=", 5) == 0 ||
                strncmp(environ[j], "USER=", 5) == 0 ||
                strncmp(environ[j], "HOME=", 5) == 0 ||
                strncmp(environ[j], "PATH=", 5) == 0 ||
                strncmp(environ[j], "LOGNAME=", 8) == 0 ||
                strncmp(environ[j], "TZ=", 3) == 0 ||
                strncmp(environ[j], "MAIL=", 5) == 0 ||
                strncmp(environ[j], "SHELL=", 6) == 0 ||
                strncmp(environ[j], "TERM=", 5) == 0 ||
                strncmp(environ[j], "DISPLAY=", 8) == 0 || strncmp(environ[j], "PRINTER=", 8) == 0 || strncmp(environ[j], "XAUTHORITY=", 11) == 0 || strncmp(environ[j], "TERMCAP=", 8) == 0)
                env[i++] = environ[j];
        env[i] = NULL;

        execve("/bin/sh", args, env);
        fatal("execv /bin/sh failed: %.100s", strerror(errno));
    }

    /* Parent. */
    if (type[0] == 'r') {       /* It is for reading. */
        close(fds[1]);
        return fds[0];
    } else {                    /* It is for writing. */
        close(fds[0]);
        return fds[1];
    }
}

/* Data structure for UserFiles. */

struct UserFile {
    enum { USERFILE_LOCAL, USERFILE_REMOTE } type;
    int handle;                 /* Local: file handle; remote: index to descriptor array. */
    unsigned char buf[512];
    unsigned int buf_first;
    unsigned int buf_last;
};

/* Allocates a UserFile handle and initializes it. */

static UserFile userfile_make_handle(int type, int handle)
{
    UserFile uf;

    uf = xmalloc(sizeof(*uf));
    uf->type = type;
    uf->handle = handle;
    uf->buf_first = 0;
    uf->buf_last = 0;
    return uf;
}

/* Opens a file using the given uid.  The uid must be either the current
   effective uid (in which case userfile_init need not have been called) or
   the uid passed to a previous call to userfile_init.  Returns a pointer
   to a structure, or NULL if an error occurred.  The flags and mode arguments
   are identical to open(). */

UserFile userfile_open(uid_t uid, const char *path, int flags, mode_t mode)
{
    int handle;

    if (uid == geteuid()) {
        handle = open(path, flags, mode);
        if (handle < 0)
            return NULL;
        return userfile_make_handle(USERFILE_LOCAL, handle);
    }

    return NULL;

}

/* Closes the userfile handle.  Returns >= 0 on success, and < 0 on error. */

int userfile_close(UserFile uf)
{
    int ret;

    switch (uf->type) {
    case USERFILE_LOCAL:
        ret = close(uf->handle);
        xfree(uf);
        return ret;

    default:
        fatal("userfile_close: type %d", uf->type);
         /*NOTREACHED*/ return -1;
    }
}

/* Get more data from the child into the buffer.  Returns false if no more
   data is available (EOF). */

static int userfile_fill(UserFile uf)
{
    int ret;

    if (uf->buf_first < uf->buf_last)
        fatal("userfile_fill: buffer not empty");

    switch (uf->type) {
    case USERFILE_LOCAL:
        ret = read(uf->handle, uf->buf, sizeof(uf->buf));
        if (ret <= 0)
            return 0;
        uf->buf_first = 0;
        uf->buf_last = ret;
        break;

    default:
        fatal("userfile_fill: type %d", uf->type);
    }

    return 1;
}

/* Returns the next character from the file (as an unsigned integer) or -1
   if an error is encountered. */

int userfile_getc(UserFile uf)
{
    if (uf->buf_first >= uf->buf_last) {
        if (!userfile_fill(uf))
            return -1;

        if (uf->buf_first >= uf->buf_last)
            fatal("userfile_getc/fill error");
    }

    return uf->buf[uf->buf_first++];
}

/* Reads data from the file.  Returns as much data as is the buffer
   size, unless end of file is encountered.  Returns the number of bytes
   read, 0 on EOF, and -1 on error. */

int userfile_read(UserFile uf, void *buf, unsigned int len)
{
    unsigned int i;
    int ch;
    unsigned char *ucp;

    ucp = buf;
    for (i = 0; i < len; i++) {
        ch = userfile_getc(uf);
        if (ch == -1)
            break;
        ucp[i] = ch;
    }

    return i;
}

/* Writes data to the file.  Writes all data, unless an error is encountered.
   Returns the number of bytes actually written; -1 indicates error. */

int userfile_write(UserFile uf, const void *buf, unsigned int len)
{
    switch (uf->type) {
    case USERFILE_LOCAL:
        return write(uf->handle, buf, len);

    default:
        fatal("userfile_write: type %d", uf->type);
         /*NOTREACHED*/ return 0;
    }
}

/* Reads a line from the file.  The line will be null-terminated, and
   will include the newline.  Returns a pointer to the given buffer,
   or NULL if no more data was available.  If a line is too long,
   reads as much as the buffer can accommodate (and null-terminates
   it).  If the last line of the file does not terminate with a
   newline, returns the line, null-terminated, but without a
   newline. */

char *userfile_gets(char *buf, unsigned int size, UserFile uf)
{
    unsigned int i;
    int ch;

    for (i = 0; i < size - 1;) {
        ch = userfile_getc(uf);
        if (ch == -1)
            break;
        buf[i++] = ch;
        if (ch == '\n')
            break;
    }
    if (i == 0)
        return NULL;

    buf[i] = '\0';

    return buf;
}

/* Performs lseek() on the given file. */

off_t userfile_lseek(UserFile uf, off_t offset, int whence)
{
    switch (uf->type) {
    case USERFILE_LOCAL:
        return lseek(uf->handle, offset, whence);


    default:
        fatal("userfile_lseek: type %d", uf->type);
         /*NOTREACHED*/ return 0;
    }
}

/* Creates a directory using the given uid. */

int userfile_mkdir(uid_t uid, const char *path, mode_t mode)
{
    /* Perform directly if with current effective uid. */
    if (uid == geteuid())
        return mkdir(path, mode);
    return -1;
}

/* Performs stat() using the given uid. */

int userfile_stat(uid_t uid, const char *path, struct stat *st)
{
    /* Perform directly if with current effective uid. */
    return (uid==getuid()?stat(path,st):(-1));
}

/* Performs remove() using the given uid. */

int userfile_remove(uid_t uid, const char *path)
{
    /* Perform directly if with current effective uid. */
    if (uid == geteuid())
        return remove(path);

    return -1;
}

/* Performs popen() on the given uid; returns a file from where the output
   of the command can be read (type == "r") or to where data can be written
   (type == "w"). */

UserFile userfile_popen(uid_t uid, const char *command, const char *type)
{
    int handle;

    if (uid == geteuid()) {
        handle = do_popen(command, type);
        if (handle < 0)
            return NULL;
        return userfile_make_handle(USERFILE_LOCAL, handle);
    }

    return NULL;
}

/* Performs pclose() on the given uid.  Returns <0 if an error occurs. */

int userfile_pclose(UserFile uf)
{
    int ret, ret2;

    switch (uf->type) {
    case USERFILE_LOCAL:
        ret = close(uf->handle);
        ret2 = wait(NULL);
        if (ret >= 0)
            ret = ret2;
        xfree(uf);
        return ret;

    default:
        fatal("userfile_close: type %d", uf->type);
         /*NOTREACHED*/ return -1;
    }
}
