/*
    Pirate Bulletin Board System
    Copyright (C) 1990, Edward Luke, lush@Athena.EE.MsState.EDU
    Eagles Bulletin Board System
    Copyright (C) 1992, Raymond Rocker, rocker@rock.b11.ingr.com
                        Guy Vega, gtvega@seabass.st.usm.edu
                        Dominic Tynes, dbtynes@seabass.st.usm.edu

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

extern char *getenv();
extern char fromhost[];

int
dashf( fname )
char *fname;
{
    struct stat st;

    return ( stat( fname, &st ) == 0 && S_ISREG( st.st_mode ) );
}

int
dashd( fname )
char    *fname;
{
    struct stat st;

    return ( stat( fname, &st ) == 0 && S_ISDIR( st.st_mode ) );
}

int
pressanykey()
{
    extern int showansi;

    showansi=1;
    move( t_lines-1,0);
    clrtoeol();
    prints( "[m                                [5;1;33m°´ÈÎºÎ¼ü¼ÌÐø ..[m" );
    egetch();
    move( t_lines-1, 0 );
    clrtoeol();
    return 0;
}

int
pressreturn()
           {
               extern int showansi;
               char buf[3] ;

               showansi=1;
               move(t_lines-1,0);
               clrtoeol();
               getdata(t_lines-1,0,"                              [33mÇë°´ ¡ô[36mEnter[33m¡ô ¼ÌÐø[m",buf,2,NOECHO,NULL,YEA);
               move(t_lines-1,0) ;
               clrtoeol() ;
               refresh() ;
               return 0 ;
           }

           askyn(str,defa)
           char str[STRLEN];
int defa;
{
    int x,y;
    char realstr[STRLEN*2];
    char ans[6];

    sprintf(realstr,"%s (Y/N)? [%c]: ",str,(defa)?'Y':'N');
    getyx(&x,&y);
    getdata( x, y, realstr, ans,3,DOECHO,NULL,YEA);
    if(ans[0]!='Y' && ans[0]!='y' &&
            ans[0]!='N' && ans[0]!='n')
    {
        return defa;
    }else if(ans[0]=='Y' || ans[0]=='y')
        return 1;
    else if(ans[0]=='N' || ans[0]=='n')
        return 0;
}




void
printdash( mesg )
char    *mesg;
{
    char        buf[ 80 ], *ptr;
    int         len;

    memset( buf, '=', 79 );
    buf[ 79 ] = '\0';
    if( mesg != NULL ) {
        len = strlen( mesg );
        if( len > 76 )  len = 76;
        ptr = &buf[ 40 - len / 2 ];
        ptr[ -1  ] = ' ';
        ptr[ len ] = ' ';
        strncpy( ptr, mesg, len );
    }
    prints( "%s\n", buf );
}

void
bell()
{
    /* change by KCN 1999.09.08    fprintf(stderr,"%c",Ctrl('G')) ;*/
    char sound;

    sound= Ctrl('G');
    output( &sound, 1);

}

void
touchnew()
{
    int fd ;

    sprintf( genbuf, "touch by: %s", currentuser.userid );
    if((fd = open(FLUSH,O_WRONLY|O_CREAT,0644)) == -1)
        return ;
    write(fd, genbuf, strlen(genbuf) );
    close(fd) ;
}

/* rrr - Snagged from pbbs 1.8 */

#define LOOKFIRST  (0)
#define LOOKLAST   (1)
#define QUOTEMODE  (2)
#define MAXCOMSZ (1024)
#define MAXARGS (40)
#define MAXENVS (20)
#define BINDIR "/bin/"

char *bbsenv[MAXENVS] ;
int numbbsenvs = 0 ;

/* Case Independent strncmp */

int
ci_strncmp(s1,s2,n)
register char *s1,*s2 ;
register int n ;
{
    char        c1, c2;

    while( n-- > 0 ) {
        c1 = *s1++;
        c2 = *s2++;
        if( c1 >= 'a' && c1 <= 'z' )
            c1 &= 0xdf;
        if( c2 >= 'a' && c2 <= 'z' )
            c2 &= 0xdf;
        if( c1 != c2 )
            return (c1 - c2);
        if( c1 == 0 )
            return 0;
    }
    return 0;
}

int
ci_strcmp( s1, s2 )
register char   *s1, *s2;
{
    char        c1, c2;

    while( 1 ) {
        c1 = *s1++;
        c2 = *s2++;
        if( c1 >= 'a' && c1 <= 'z' )
            c1 &= 0xdf;
        if( c2 >= 'a' && c2 <= 'z' )
            c2 &= 0xdf;
        if( c1 != c2 )
            return (c1 - c2);
        if( c1 == 0 )
            return 0;
    }
}

int
bbssetenv(env,val)
char *env, *val ;
{
    register int i,len ;

    if(numbbsenvs == 0)
        bbsenv[0] = NULL ;
    len = strlen(env) ;
    for(i=0;bbsenv[i];i++)
        if(!strncasecmp(env,bbsenv[i],len))
            break ;
    if(i>=MAXENVS)
        return -1 ;
    if(bbsenv[i])
        free(bbsenv[i]) ;
    else
        bbsenv[++numbbsenvs] = NULL ;
    bbsenv[i] = malloc(strlen(env)+strlen(val)+2) ;
    strcpy(bbsenv[i],env) ;
    strcat(bbsenv[i],"=") ;
    strcat(bbsenv[i],val) ;
    return 0;
}

int
do_exec(com,wd)
char *com, *wd ;
{
    char path[MAXPATHLEN] ;
    char pcom[MAXCOMSZ] ;
    char *arglist[MAXARGS] ;
    char *tz;
    register int i,len ;
    register int argptr ;
    int status, pid, w ;
    int pmode ;
    void (*isig)(), (*qsig)() ;

    strncpy(path,BINDIR,MAXPATHLEN) ;
    strncpy(pcom,com,MAXCOMSZ) ;
    len = Min(strlen(com)+1,MAXCOMSZ) ;
    pmode = LOOKFIRST ;
    for(i=0,argptr=0;i<len;i++) {
        if(pcom[i] == '\0')
            break ;
        if(pmode == QUOTEMODE) {
            if(pcom[i] == '\001') {
                pmode = LOOKFIRST ;
                pcom[i] = '\0' ;
                continue ;
            }
            continue ;
        }
        if(pcom[i] == '\001') {
            pmode = QUOTEMODE ;
            arglist[argptr++] = &pcom[i+1] ;
            if(argptr+1 == MAXARGS)
                break ;
            continue ;
        }
        if(pmode == LOOKFIRST)
            if(pcom[i] != ' ') {
                arglist[argptr++] = &pcom[i] ;
                if(argptr+1 == MAXARGS)
                    break ;
                pmode = LOOKLAST ;
            } else continue ;
        if(pcom[i] == ' ') {
            pmode = LOOKFIRST ;
            pcom[i] = '\0' ;
        }
    }
    arglist[argptr] = NULL ;
    if(argptr == 0)
        return -1 ;
    if(*arglist[0] == '/')
        strncpy(path,arglist[0],MAXPATHLEN) ;
    else
        strncat(path,arglist[0],MAXPATHLEN) ;
    reset_tty() ;
    alarm(0) ;
#ifdef IRIX
    if((pid = fork()) == 0) {
#else
if((pid = vfork()) == 0) {
#endif
        if(wd)
            if(chdir(wd)) {
                /* change by KCN 1999.09.08
                              fprintf(stderr,"Unable to chdir to '%s'\n",wd) ;
                */
                prints("Unable to chdir to '%s'\n",wd) ;
                oflush();
                exit(-1) ;
            }
        bbssetenv("PATH", "/bin:.");
        bbssetenv("TERM", "vt100");
        bbssetenv("USER", currentuser.userid);
        bbssetenv("USERNAME", currentuser.username);
        /* added for tin's reply to */
        bbssetenv("REPLYTO", currentuser.email);
        bbssetenv("FROMHOST", fromhost);
        /* end of insertion */
        if ((tz = getenv("TZ")) != NULL)
            bbssetenv("TZ", tz);
        if(numbbsenvs == 0)
            bbsenv[0] = NULL ;
        execve(path,arglist,bbsenv) ;
        /* change by KCN 1999.09.08
                fprintf(stderr,"EXECV FAILED... path = '%s'\n",path) ;
        	*/
        prints("EXECV FAILED... path = '%s'\n",path) ;
        oflush();
        exit(-1) ;
    }
    isig = signal(SIGINT, SIG_IGN) ;
    qsig = signal(SIGQUIT, SIG_IGN) ;
    while((w = wait(&status)) != pid && w != 1)
        /* NULL STATEMENT */ ;
    signal(SIGINT, isig) ;
    signal(SIGQUIT, qsig) ;
    restore_tty() ;
#ifdef DOTIMEOUT
    alarm(IDLE_TIMEOUT) ;
#endif
    return((w == -1)? w: status) ;
}

#ifdef kill
#undef kill
#endif

int safe_kill(int x, int y)
{  if(x<=0) return -1;
   return kill(x,y);
}
