# Process this file with autoconf to produce a configure script.
AC_INIT(FULL-PACKAGE-NAME, VERSION, BUG-REPORT-ADDRESS)
AC_CANONICAL_SYSTEM
case "${host}" in
  i*86-*-freebsd4*)
        CFLAGS="$CFLAGS -DFREEBSD"
	;;
  *)
	;;
esac
AM_INIT_AUTOMAKE([innbbsd],1.0)
AC_CONFIG_SRCDIR([config.h.in])
AM_CONFIG_HEADER(config.h)

m4_include(../config.icc)

# Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_MAKE_SET
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_YACC

# Checks for libraries.
m4_include(../config.lib)

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([fcntl.h limits.h memory.h netdb.h netinet/in.h sgtty.h stdlib.h string.h strings.h sys/file.h sys/ioctl.h sys/param.h sys/socket.h sys/time.h syslog.h termios.h unistd.h utime.h utmp.h utmpx.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UID_T
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_CHECK_MEMBERS([struct stat.st_rdev])
AC_HEADER_TIME
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_CHOWN
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_HEADER_MAJOR
AC_FUNC_MMAP
AC_FUNC_SETPGRP
AC_FUNC_SETVBUF_REVERSED
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_FUNC_UTIME_NULL
AC_FUNC_VPRINTF
AC_FUNC_WAIT3
AC_CHECK_FUNCS([alarm bzero dup2 ftruncate getcwd gethostbyaddr gethostbyname gethostname getpagesize gettimeofday getwd inet_ntoa memchr mkdir munmap putenv select setenv socket strcasecmp strchr strdup strerror strncasecmp strrchr strspn strstr utime])

AC_CONFIG_FILES([Makefile
                 convert/hztty-2.0/Makefile])
AC_OUTPUT
