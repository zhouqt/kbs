# Process this file with autoconf to produce a configure script.
#AC_INIT(FULL-PACKAGE-NAME, VERSION, BUG-REPORT-ADDRESS)
AC_INIT(smthbbs, 1.1, SYSOP@smth.org)
AM_INIT_AUTOMAKE([foreign dist-bzip2])

AC_CANONICAL_TARGET
case "${host}" in
  i*86-*-freebsd*)
        OSTYPE="FREEBSD"
        ;;      
  *-*-solaris*) 
        OSTYPE="SOLARIS"
	LIBS="$LIBS -lnsl -lsocket"
        ;;  
  *-*-sunos*)
        OSTYPE="SUNOS"
	LIBS="$LIBS -lnsl -lsocket"
        ;;  
  *-*-linux*)        
        OSTYPE="LINUX"
        ;;  
  *)        
        OSTYPE="GENERIC"
        ;;        
esac              
CFLAGS="$CFLAGS -D$OSTYPE"

AC_CONFIG_SRCDIR([config.h.in])
AM_CONFIG_HEADER(config.h)
#AM_CONDITIONAL(OWN_AR, TRUE)

AC_PREFIX_DEFAULT(/home/bbs)

m4_include(config.icc)

AC_ARG_ENABLE(innbbsd,
[  --enable-innbbsd
                enable innbbsd  support(need innbbsd package)],
[
	if test "$enableval" = "yes" ; then
	        if test -d innbbsd; then
        	        AC_CONFIG_SUBDIRS(innbbsd)
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
		if test -d sshbbsd; then
			AC_CONFIG_SUBDIRS(sshbbsd)
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
		if test -d bbs2www; then
			AC_CONFIG_SUBDIRS(bbs2www)
		else
			AC_MSG_ERROR([can't find bbs2www package,please get it first!])
		fi
		CONFIG_HAVE_WWW=1
        else
		CONFIG_HAVE_WWW=0
	fi
])
AC_SUBST(CONFIG_HAVE_WWW)


# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([arpa/inet.h fcntl.h limits.h malloc.h netdb.h netinet/in.h sgtty.h stdlib.h string.h sys/file.h sys/ioctl.h sys/param.h sys/socket.h sys/time.h termio.h termios.h unistd.h utime.h])

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

# Checks for library functions.
AC_CONFIG_LIBOBJ_DIR(lib)
AC_FUNC_ALLOCA
AC_FUNC_FORK
AC_FUNC_GETLOADAVG
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_MKTIME
AC_FUNC_MMAP
AC_FUNC_SETPGRP
AC_FUNC_SETVBUF_REVERSED
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_FUNC_UTIME_NULL
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([alarm atexit bzero dup2 ftruncate getcwd gethostbyaddr gethostbyname gethostname inet_ntoa isascii memchr memset mkdir munmap rmdir select socket strcasecmp strcasestr strchr strdup strerror strncasecmp strpbrk strrchr strstr strtol utime memmem flock inet_aton inet_pton isblank])
AC_CHECK_TYPES(bool)
AC_CHECK_TYPES(sig_t,,,[#include <signal.h>])

dnl Select Error language
AC_ARG_ENABLE(site,
[  --enable-site=sitename
                          select site special files (see site dir) ],
[
    BBSSITE=$enableval
],[BBSSITE="smth"
SITEDIR=site])
if test -f $srcdir/site/$BBSSITE.c; then
	SITEDIR=site
else
  if test -f $srcdir/sites/$BBSSITE.c; then
	SITEDIR=sites
  else
        AC_MSG_ERROR([ERROR! Unknown site $BBSSITE, see site/])
  fi
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
AC_PROG_RANLIB
m4_include(config.lib)

# Checks for libraries.
AC_CONFIG_LINKS(libBBS/site.c:$SITEDIR/$BBSSITE.c
		libBBS/site.h:$SITEDIR/$BBSSITE.h
		src/site.c:$SITEDIR/$BBSSITE.c
		src/site.h:$SITEDIR/$BBSSITE.h
		libBBS/default.c:$SITEDIR/default.c
		libBBS/default.h:$SITEDIR/default.h
		src/default.c:$SITEDIR/default.c
		src/default.h:$SITEDIR/default.h
		)

cd libBBS; for i in *.c ; do 
AC_CONFIG_LINKS(src/$i:libBBS/$i)
done
cd ..

AC_CONFIG_FILES([Makefile
                 libBBS/Makefile
                 libsystem/Makefile
                 rzsz/Makefile
                 daemon/Makefile
                 contrib/Makefile
                 contrib/fb2k2smth/Makefile
                 mail2bbs/Makefile
                 local_utl/Makefile
                 service/Makefile
                 service/pip/Makefile
                 service/worker/Makefile
                 service/personaldns/Makefile
                 src/bbsconfig.h
                 src/Makefile])
AC_OUTPUT
