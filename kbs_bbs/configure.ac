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
AM_INIT_AUTOMAKE(smthbbs, 1.0.1)

AC_CONFIG_SRCDIR([config.h.in])
AM_CONFIG_HEADER(config.h)
#AM_CONDITIONAL(OWN_AR, TRUE)

AC_PREFIX_DEFAULT(/home/bbs)

m4_include(config.icc)

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

if [ test -z "$enable_www" ]; then
	enable_www=yes
fi

AC_ARG_ENABLE(www,
[  --enable-www
        enable web support(need bbs2www package)],
[
    if [ test "$enableval" = "yes" ]; then
	    if test -d bbs2www; then
		AC_CONFIG_SUBDIRS(bbs2www)
		else
		AC_MSG_ERROR([can't find bbs2www package,please get it first!])
	   fi
    fi
])


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
AC_CHECK_TYPES(bool)

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

AH_TEMPLATE([USE_SSL],
[Define to use openssl library])

AC_ARG_WITH(openssl,
[  --with-openssl[=path],Build with OpenSSL [yes|no|path].
	],
    use_openssl="$withval", use_openssl="no")
case "$use_openssl" in
        no)
                AC_MSG_RESULT(no)
                DST_OPENSSL_INC=""
                USE_OPENSSL=""
                ;;
        *)
            # User did not specify a path - guess it
            openssldirs="/usr /usr/local /usr/local/ssl /usr/pkg"
            for d in $openssldirs
            do
                  if test -f $d/include/openssl/opensslv.h
                  then
                      use_openssl=$d
                      break
                  fi
            done
            if test "$use_openssl" = "yes"
            then
                  AC_MSG_RESULT(not found)
                  AC_MSG_ERROR(
[OpenSSL was not found in any of $openssldirs; use --with-openssl=/path])
            fi

    	    AC_DEFINE(USE_SSL)
            OPENSSL_INC="-I$use_openssl/include"
            OPENSSL_LIBS="-L$use_openssl/lib -lcrypto -lssl"
            AC_MSG_RESULT(using openssl from $use_openssl/lib and $use_openssl/include)

            AC_MSG_CHECKING(OpenSSL library version)
            saved_cflags="$CFLAGS"
            saved_libs="$LIBS"
            CFLAGS="$CFLAGS $DST_OPENSSL_INC"
            LIBS="$LIBS $DNS_OPENSSL_LIBS"
            AC_TRY_RUN([
#include <stdio.h>
#include <openssl/opensslv.h>
int main() {
        printf("\n\nFound   OPENSSL_VERSION_NUMBER %#010x\n",
                OPENSSL_VERSION_NUMBER);
        return (0);
}
],
            [AC_MSG_RESULT(ok)],
            [AC_MSG_RESULT(not compatible)],
            [AC_MSG_RESULT(assuming target platform has compatible version)])
            CFLAGS="$saved_cflags"
            LIBS="$saved_libs"
            ;;
esac

AC_SUBST(OPENSSL_INC)
AC_SUBST(OPENSSL_LIBS)

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
                 mail2bbs/Makefile
                 local_utl/Makefile
                 service/Makefile
                 service/pip/Makefile
                 src/bbsconfig.h
                 src/Makefile])
AC_OUTPUT
