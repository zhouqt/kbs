/*******************************************************************************
 *  The Elm Mail System  -  $Revision$   $State$
 *
 *			Copyright (c) 1992 USENET Community Trust
 *******************************************************************************
 * Bug reports, patches, comments, suggestions should be sent to:
 *
 *	Syd Weinstein, Elm Coordinator
 *	elm@DSI.COM			dsinc!elm
 *
 *******************************************************************************
 * $Log$
 * Revision 1.1  2002/04/27 05:47:25  kxn
 * Initial revision
 *
 * Revision 1.1  2001/07/04 06:07:11  bbsdev
 * bbs sshd
 *
 * Revision 1.1.1.1  1996/02/18 21:38:11  ylo
 * 	Imported ssh-1.2.13.
 *
 * Revision 1.1  1995/07/13  22:17:30  ylo
 * 	Added putenv.c (this file was missing from the original distribution).
 *
 * Revision 5.1  1992/10/03  22:41:36  syd
 * Initial checkin as of 2.4 Release at PL0
 *
 *
 ******************************************************************************/

/*
 * This code was stolen from cnews.  Modified to make "newenv" static so
 * that realloc() can be used on subsequent calls to avoid memory leaks.
 *
 * We only need this if Configure said there isn't a putenv() in libc.
 */

#include "includes.h"

/* peculiar return values */
#define WORKED 0
#define FAILED 1
#define YES 1
#define NO 0

int
putenv(var)			/* put var in the environment */
char *var;
{
	register char **envp;
	register int oldenvcnt;
	extern char **environ;
	static char **newenv = NULL;

	/* count variables, look for var */
	for (envp = environ; *envp != 0; envp++) {
		register char *varp = var, *ep = *envp;
		register int namesame;

		namesame = NO;
		for (; *varp == *ep && *varp != '\0'; ++ep, ++varp)
			if (*varp == '=')
				namesame = YES;
		if (*varp == *ep && *ep == '\0')
			return WORKED;	/* old & new var's are the same */
		if (namesame) {
			*envp = var;	/* replace var with new value */
			return WORKED;
		}
	}
	oldenvcnt = envp - environ;

	/* allocate new environment with room for one more variable */
	if (newenv == NULL)
	    newenv = (char **)malloc((unsigned)((oldenvcnt+1+1)*sizeof(*envp)));
	else
	    newenv = (char **)realloc((char *)newenv, (unsigned)((oldenvcnt+1+1)*sizeof(*envp)));
	if (newenv == NULL)
		return FAILED;

	/* copy old environment pointers, add var, switch environments */
	(void) bcopy((char *)environ, (char *)newenv, oldenvcnt*sizeof(*envp));
	newenv[oldenvcnt] = var;
	newenv[oldenvcnt+1] = NULL;
	environ = newenv;
	return WORKED;
}
