# Process this file with autoconf to produce a configure script.
AC_INIT(FULL-PACKAGE-NAME, VERSION, BUG-REPORT-ADDRESS)
AM_INIT_AUTOMAKE([smthwwwbbs],1.0)
AC_CONFIG_SRCDIR([config.h.in])
AM_CONFIG_HEADER(config.h)


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
CFLAGS="$CFLAGS -DSMTH -DNJU_WWWBBS -DBBS2WWW"

AC_SUBST(CONFIG_AR)
AC_SUBST(EXTRA_CLEAN)

with_php="yes"

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

with_phpsuffix="jsp"
AC_ARG_WITH(phpsuffix,
[  --with-phpsuffix=SUFFIX
                          select suffix of php file ])

PHPSUFFIX=$with_phpsuffix

AC_SUBST(PHPBASE)
AC_SUBST(PHPSUFFIX)

with_www="yes"

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
            AC_MSG_ERROR("can't find php in $withval")
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
# FIXME: Replace `main' with a function in `-ldl':
AC_CHECK_LIB([dl], [main])
# FIXME: Replace `main' with a function in `-lesmtp':
AC_CHECK_LIB([esmtp], [main])
# FIXME: Replace `main' with a function in `-lltdl':
AC_CHECK_LIB([ltdl], [main])

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
AC_CHECK_FUNCS([alarm bzero ftruncate getcwd memset mkdir munmap socket strcasecmp strchr strdup strerror strncasecmp strrchr strstr])

AC_CONFIG_LINKS(mkinstalldirs:../mkinstalldirs)
AC_CONFIG_LINKS(missing:../missing)
AC_CONFIG_LINKS(depcomp:../depcomp)

AC_CONFIG_FILES([Makefile
                 lib/Makefile
                 phplib/Makefile
                 src/Makefile])
AC_OUTPUT
