# Process this file with autoconf to produce a configure script.
AC_INIT(FULL-PACKAGE-NAME, VERSION, BUG-REPORT-ADDRESS)
AC_CANONICAL_SYSTEM
case "${host}" in
  i*86-*-freebsd*)
	CFLAGS="$CFLAGS -DFREEBSD"
	;;
  *)
	;;
esac
AM_INIT_AUTOMAKE([smthwwwbbs],1.0)
AC_CONFIG_SRCDIR([config.h.in])
AM_CONFIG_HEADER(config.h)

m4_include(../config.icc)

if [ test -z "$with_php" ]; then
  with_php="yes"
fi

AC_ARG_WITH(php,
[  --with-php=path
                          select php source directory],
[
    if test "$withval" = "yes"; then
	if test -f /home/share/php/Zend/zend.h; then
            withval=/home/share/php
	else
	if test -f /usr/include/php/Zend/zend.h; then
            withval=/usr/include/php
	else
	if test -f /usr/local/include/php/Zend/zend.h; then
            withval=/usr/local/include/php
	fi
	fi
	fi
    fi
    if test '!' -f $withval/Zend/zend.h; then
	    AC_MSG_ERROR("can't find php source in $withval")
    fi
    PHPBASE=$withval
],[AC_MSG_ERROR("must have php source to compile php module!")])

if [ test -z "$with_phpsuffix" ]; then
  with_phpsuffix="jsp"
fi

AC_ARG_WITH(phpsuffix,
[  --with-phpsuffix=SUFFIX
                          select suffix of php file ])

PHPSUFFIX=$with_phpsuffix

AC_SUBST(PHPBASE)
AC_SUBST(PHPSUFFIX)

if [ test -z "$with_www" ]; then
  with_www="yes"
fi

AC_ARG_WITH(www,
[  --with-www=path
                          select the root path of www],
[
    if test "$withval" = "yes"; then
        if test -f /home0/www/bin/apachectl; then
            withval=/home0/www
        else
        if test -d /usr/local/apache; then
            withval=/usr/local/apache
        else
        if test -d /var/www; then
            withval=/var/www
        fi
        fi
        fi
    fi
    if test '!' -d $withval; then
            AC_MSG_ERROR("can't find www in $withval")
    fi
    WWWBASE=$withval
    CGIPATH=$WWWBASE/cgi-bin/bbs
    if test -d $withval/html; then
    	HTMPATH=$WWWBASE/html
    else
    	HTMPATH=$WWWBASE/htdocs
    fi
    PHPBBSLIBPATH=$WWWBASE/libexec/bbs
],[AC_MSG_ERROR("must have www support!")])

AC_SUBST(WWWBASE)
AC_SUBST(CGIPATH)
AC_SUBST(HTMPATH)
AC_SUBST(PHPBBSLIBPATH)

HTML_FILES=`find html -path '*/CVS/*' -prune -o -type f -print | xargs`
AC_SUBST(HTML_FILES)

OLDCFLAGS="$CFLAGS"
CFLAGS="$OLDCFLAGS  -DINVISIBLE -DNJU_WWWBBS -DBBS2WWW"
# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AC_PROG_LIBTOOL
AC_SUBST(CONFIG_AR)
AC_SUBST(EXTRA_CLEAN)

# Checks for libraries.
m4_include(../config.lib)

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([netdb.h netinet/in.h sys/socket.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_CHECK_FUNCS([alarm bzero ftruncate getcwd memset mkdir munmap socket strcasecmp strcasestr strchr strdup strerror strncasecmp strrchr strstr])
WWWSRCDIR=`pwd`
AC_SUBST(WWWSRCDIR)

AC_CONFIG_FILES([Makefile
                 lib/Makefile
                 phplib/Makefile
                 phplib/config.m4
                 src/Makefile])
AC_OUTPUT
