/*-
 * Copyright (c) 2003 SMTH BBS.
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/* A quick and dirty implementatiion of flock() for Solaris port. */
#include <unistd.h>
#include <fcntl.h>

#ifndef HAVE_FLOCK
#include <flock.h>
int flock(int fd, int operation)
{
	struct flock fl;
	int ret;

	fl.l_whence = SEEK_SET;
	fl.l_start = 0;
	fl.l_len = 0;
	switch (operation)
	{
	case LOCK_SH:
		fl.l_type = F_RDLCK;
		ret = fcntl(fd, F_SETLKW, &fl);
		break;
	case LOCK_EX:
		fl.l_type = F_WRLCK;
		ret = fcntl(fd, F_SETLKW, &fl);
		break;
	case LOCK_UN:
		fl.l_type = F_UNLCK;
		ret = fcntl(fd, F_SETLKW, &fl);
		break;
	default:
		ret = -1;
	}
	return ret;
}
#endif /* !HAVE_FLOCK */
