# Process this file with autoconf to produce a configure script.
AC_INIT(FULL-PACKAGE-NAME, VERSION, BUG-REPORT-ADDRESS)

AM_INIT_AUTOMAKE(smthbbs, 1.0)

AC_CONFIG_SRCDIR([config.h.in])
AM_CONFIG_HEADER(config.h)
#AM_CONDITIONAL(OWN_AR, TRUE)

AC_PREFIX_DEFAULT(/home/bbs)

AC_ARG_WITH(icc,
[  --with-icc=path
                          select intel compiler],
[
	if test "$withval" = "yes"; then
		withval=/opt/intel/compiler60/ia32
	fi
    if test -d $withval; then
		CC=icc
		CXX=icc
		if test -z "$CFLAGS"; then
			CFLAGS="$CFLAGS -tpp6 -axK -O3 -unroll256 -w1 -ipo -g -I$withval/include"
		fi
		if test -z "$LDFLAGS"; then
			LDFLAGS="$LDFLAGS -ipo -static"
		fi
		CONFIG_AR=xiar
		EXTRA_CLEAN="*.il"
    else
		AC_MSG_ERROR([can't find intel compiler in $withval!])
    fi
],[CONFIG_AR=ar; EXTRA_CLEAN=""])

AC_SUBST(CONFIG_AR)
AC_SUBST(EXTRA_CLEAN)

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lbsd':
AC_CHECK_LIB([bsd], [main])
# FIXME: Replace `main' with a function in `-ldl':
AC_CHECK_LIB([dl], [main])
# FIXME: Replace `main' with a function in `-lesmtp':
AC_CHECK_LIB([esmtp], [main])
# FIXME: Replace `main' with a function in `-lltdl':
AC_CHECK_LIB([ltdl], [main])

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
AC_CHECK_FUNCS([alarm atexit bzero dup2 ftruncate getcwd gethostbyaddr gethostbyname gethostname inet_ntoa isascii memchr memset mkdir munmap rmdir select socket strcasecmp strchr strdup strerror strncasecmp strpbrk strrchr strstr strtol utime])

dnl Select Error language
AC_ARG_ENABLE(site,
[  --enable-site=sitename
                          select site special files (see site dir) ],
[
    if test -f $srcdir/site/$enableval.c; then
        BBSSITE=$enableval
    else
        AC_MSG_ERROR([ERROR! Unknown site $enableval, see site/])
    fi
],[BBSSITE="smth"])
AC_SUBST(BBSSITE)

AC_ARG_ENABLE(ssh,
[  --enable-ssh
		enable ssh bbsd  support(need sshbbsd package)],
[
	if test -d sshbbsd; then
		AC_CONFIG_SUBDIRS(sshbbsd)
	else
		AC_MSG_ERROR([can't find sshbbsd package,please get it first!])
	fi
])

AC_CONFIG_LINKS(libBBS/site.c:site/$BBSSITE.c
		libBBS/site.h:site/$BBSSITE.h
		src/site.c:site/$BBSSITE.c
		src/site.h:site/$BBSSITE.h
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
                 local_utl/Makefile
                 src/bbsconfig.h
                 src/Makefile])
AC_OUTPUT
