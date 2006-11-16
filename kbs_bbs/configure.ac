# Process this file with autoconf to produce a configure script.
# AC_INIT(FULL-PACKAGE-NAME, VERSION, BUG-REPORT-ADDRESS)
AC_INIT(kbsbbs, 2.0dev, dev@dev.kcn.cn)

AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([foreign dist-bzip2])

case "${host}" in
  i*86-*-freebsd*)
        OSTYPE="FREEBSD"
        ;;      
  *-*-solaris*) 
        OSTYPE="SOLARIS"
        CFLAGS="-D_POSIX_PTHREAD_SEMANTICS $CFLAGS"
	LIBS="$LIBS -lnsl -lsocket"
        ;;  
  *-*-sunos*)
        OSTYPE="SUNOS"
        CFLAGS="-D_POSIX_PTHREAD_SEMANTICS $CFLAGS"
	LIBS="$LIBS -lnsl -lsocket"
        ;;  
  *-*-linux*)        
        OSTYPE="LINUX"
        ;;  
  *-*-cygwin*)        
        OSTYPE="CYGWIN"
	LIBS="$LIBS -lcygipc"
        ;;  
  *)        
        OSTYPE="GENERIC"
        ;;        
esac              
CFLAGS="$CFLAGS -D$OSTYPE"

AH_TOP([
#ifndef KBS_CONFIG_H_INCLUDED
#define KBS_CONFIG_H_INCLUDED
])
AH_BOTTOM([
#endif /* KBS_CONFIG_H_INCLUDED */
])

AM_CONFIG_HEADER(kbs_config.h)

AC_PREFIX_DEFAULT(/home/bbs)
if test "x$prefix" = xNONE; then
  BBSHOME="/home/bbs"
else
  BBSHOME=$prefix
fi
AC_DEFINE_UNQUOTED([BBSHOME], "$BBSHOME", [Path to BBS installation directory])


m4_include(config.icc)


AC_ARG_ENABLE(admin,
[  --enable-admin
                          enable admin  support(need admin package)],
[
	if test "$enableval" = "yes" ; then
	        if test -d $srcdir/admin; then
                    AC_CONFIG_FILES(admin/Makefile)
                    BBSSUBDIRS="$BBSSUBDIRS admin"
	        else
        	        AC_MSG_ERROR([can't find admin package,please get it first!])
	        fi
	fi
])

AC_ARG_ENABLE(innbbsd,
[  --enable-innbbsd
                          enable innbbsd  support(need innbbsd package)],
[
	if test "$enableval" = "yes" ; then
	        if test -d $srcdir/innbbsd; then
                    BBS_LIB_ICONV
                    AC_CONFIG_FILES(innbbsd/Makefile
                                    innbbsd/linkinnd.sh)
                    BBSSUBDIRS="$BBSSUBDIRS innbbsd"
	        else
        	        AC_MSG_ERROR([can't find innbbsd package,please get it first!])
	        fi
	fi
])

AC_ARG_ENABLE(ssh,
[  --enable-ssh
                          enable ssh bbsd  support(need sshbbsd package)],
[
  if test "$enableval" = "yes" ; then
		if test -d $srcdir/sshbbsd; then
			AC_CONFIG_SUBDIRS(sshbbsd)
            BBSSUBDIRS="$BBSSUBDIRS sshbbsd"
		else
			AC_MSG_ERROR([can't find sshbbsd package,please get it first!])
		fi
  fi
])

if [ test -z "$enable_www" ]; then
	enable_www=yes
fi

AC_ARG_ENABLE(www,
[  --enable-www
                          enable web support(need bbs2www package)],
[
	if test "$enableval" = "yes" ; then
		if test -d $srcdir/bbs2www; then
            AC_CONFIG_FILES([bbs2www/Makefile
                             bbs2www/lib/Makefile
                             bbs2www/phplib/Makefile
                             bbs2www/phplib/config.m4])
            BBSSUBDIRS="$BBSSUBDIRS bbs2www"
		else
			AC_MSG_ERROR([can't find bbs2www package,please get it first!])
		fi
		AC_DEFINE([HAVE_WWW], [1], [Define to enable www support.])
		CONFIG_HAVE_WWW=1
	else
		CONFIG_HAVE_WWW=0
	fi
])
AC_SUBST(CONFIG_HAVE_WWW)
AC_SUBST(BBSSUBDIRS)

m4_include(bbs2www.in)

AC_ARG_ENABLE(customizing,
[  --enable-customizing
                          enable customizing in INI file],
[
  if test "$enableval" = "yes" ; then
    AC_DEFINE(ENABLE_CUSTOMIZING,1,[ Enable customizing in INI file ])
  fi
])


# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([arpa/inet.h dlfcn.h fcntl.h limits.h malloc.h netdb.h netinet/in.h sgtty.h stddef.h stdint.h stdlib.h string.h strings.h sys/file.h sys/ioctl.h sys/param.h sys/socket.h sys/time.h termio.h termios.h unistd.h utime.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STAT
AC_C_CONST
AC_C_INLINE
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_CHECK_TYPES([struct utimbuf],[],[],[#include <utime.h>])

AC_CHECK_SIZEOF(int *)
AC_CHECK_SIZEOF(long long int)
AC_CHECK_SIZEOF(long int)
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(int64)
AC_CHECK_SIZEOF(int32)

# Checks for library functions.
AC_CONFIG_LIBOBJ_DIR(lib)
AC_FUNC_ALLOCA
AC_FUNC_FORK
AC_FUNC_GETLOADAVG
AC_PROG_GCC_TRADITIONAL
if test "$cross_compiling" != "yes" ; then
  AC_FUNC_MALLOC
  AC_FUNC_MEMCMP
else
  dnl A quick and dirty workaround.
  AC_DEFINE(HAVE_MALLOC,1,[ ])
  AC_DEFINE(HAVE_MEMCMP,1,[ ])
fi
AC_FUNC_MKTIME
AC_FUNC_MMAP
if test "$cross_compiling" != "yes" ; then
  AC_FUNC_SETPGRP
fi
AC_FUNC_SETVBUF_REVERSED
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_FUNC_UTIME_NULL
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([alarm atexit bcopy bzero dup2 flock ftruncate getcwd gethostbyaddr gethostbyname gethostname \
                getloadavg inet_aton inet_ntoa inet_pton isascii isblank memchr memmem memset mkdir munmap rmdir \
                select socket strcasecmp strcasestr strchr strdup strerror strncasecmp strpbrk strrchr strsep \
                strstr strtol usleep utime])
AC_CHECK_TYPES(bool)
AC_CHECK_TYPES(byte)
AC_CHECK_TYPES(sig_t,,,[#include <signal.h>])
AC_CHECK_TYPES(socklen_t,,,[
#include <unistd.h>
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif /* HAVE_SYS_SOCKET_H */
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif /* HAVE_ARPA_INET_H */
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif /* HAVE_NETINET_IN_H */
])

dnl Select Error language
AC_ARG_ENABLE(site,
[  --enable-site=sitename
                          select site special files (see site dir) ],
[
    BBSSITE=$enableval
],[BBSSITE="fb2k-v2"
SITEDIR=site])
if test -f $srcdir/site/$BBSSITE.c; then
	SITEDIR=site
else
        AC_MSG_ERROR([ERROR! Unknown site $BBSSITE, see site/])
fi
AC_SUBST(BBSSITE)

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_LIBTOOL
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_YACC
m4_include(config.lib)

if test "$GCC" = "yes"; then
  CFLAGS="$CFLAGS -Wpointer-arith -Wall"
fi

# Checks for libraries.
AC_CONFIG_LINKS(libBBS/site.c:$SITEDIR/$BBSSITE.c
		site.h:$SITEDIR/$BBSSITE.h
		libBBS/default.c:$SITEDIR/default.c
		default.h:$SITEDIR/default.h
		)

ORIGIN_LIBTOOL=$LIBTOOL
AC_SUBST(ORIGIN_LIBTOOL)
AC_SUBST(SED)
abssrcdir=`pwd`
LIBTOOL="$abssrcdir/wrapper.sh"
AC_SUBST(abssrcdir)
AC_CONFIG_FILES([Makefile
                 libBBS/Makefile
                 libsystem/Makefile
                 rzsz/Makefile
                 daemon/Makefile
                 mail2bbs/Makefile
                 local_utl/Makefile
                 service/Makefile
                 service/pip/Makefile
                 service/worker/Makefile
                 service/personaldns/Makefile
                 src/Makefile])
AC_CONFIG_FILES(wrapper.sh, chmod +x wrapper.sh)
AC_OUTPUT

