/*
 * $Id$
 */
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include "clibrary.h"

#ifdef LINUX
#ifndef FIOCLEX
#define FIOCLEX 0x5451
#endif
#ifndef FIONCLEX
#define FIONCLEX 0x5450
#endif
#endif

#ifndef CLX_IOCTL
#define CLX_IOCTL
#endif
#ifndef CLX_FCNTL
#define CLX_FCNTL
#endif

#if	defined(CLX_IOCTL) && !defined(IRIX)
#ifdef __linux
#include <termios.h>
#else
#include <sgtty.h>
#endif

/*
**  Mark a file close-on-exec so that it doesn't get shared with our
**  children.  Ignore any error codes.
*/
void closeOnExec(fd, flag)
int fd;
int flag;
{
    int oerrno;

    oerrno = errno;
    (void) ioctl(fd, flag ? FIOCLEX : FIONCLEX, (char *) NULL);
    errno = oerrno;
}
#endif                          /* defined(CLX_IOCTL) */

#if	defined(CLX_FCNTL)
#include <fcntl.h>

/*
**  Mark a file close-on-exec so that it doesn't get shared with our
**  children.  Ignore any error codes.
*/
void CloseOnExec(fd, flag)
int fd;
int flag;
{
    int oerrno;

    oerrno = errno;
    (void) fcntl(fd, F_SETFD, flag ? 1 : 0);
    errno = oerrno;
}
#endif                          /* defined(CLX_FCNTL) */
