#
# configure.ac
#
# Author: Tatu Ylonen <ylo@cs.hut.fi>
#
# Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
#                    All rights reserved
#
# Created: Wed Mar 22 18:02:48 1995 ylo
#
#

AC_INIT(sshd.c)
AM_INIT_AUTOMAKE(sshbbs, 1.0)

AH_TOP([
#ifndef KBS_SSH_CONFIG_H_INCLUDED
#define KBS_SSH_CONFIG_H_INCLUDED
])
AH_BOTTOM([
#endif /* KBS_SSH_CONFIG_H_INCLUDED */
])
AM_CONFIG_HEADER(kbs_ssh_config.h)

AC_PREREQ(2.10)

# So many systems seem to need this that it is better do it here automatically.
#LIBS="-L/usr/local/lib $LIBS"

m4_include(../config.icc)
no_x="yes"

AC_CANONICAL_HOST
AC_MSG_CHECKING(cached information)
hostcheck="$host"
AC_CACHE_VAL(ac_cv_hostcheck, [ ac_cv_hostcheck="$hostcheck" ])
if test "$ac_cv_hostcheck" != "$hostcheck"; then
  AC_MSG_RESULT(changed)
  AC_MSG_WARN(config.cache exists!)
  AC_MSG_ERROR(you must do 'make distclean' first to compile for different host or different parameters.)
else
  AC_MSG_RESULT(ok)
fi

AC_PROG_CC
AC_ISC_POSIX

if test "$GCC" = "yes"; then
  CFLAGS="$CFLAGS -Wall"
fi

AC_DEFINE_UNQUOTED(HOSTTYPE, "$host", [ ])

case "$host" in
  *-*-sunos4.1.1*)
    os_sunos=yes
    # Tim Adam <tma@osa.com.au> says speed_t is defined in stdtypes.h
    AC_DEFINE(SPEED_T_IN_STDTYPES_H, 1, [ ])
    ;;
  *-*-solaris*)
    # solaris stuff. appro@fy.chalmers.se
    AC_DEFINE(SECURE_RPC, 1, [ ])
    AC_DEFINE(SECURE_NFS, 1, [ ])
    # NIS+ is forced so that we don't have to recompile
    # if we move to NIS+. appro@fy.chalmers.se
    AC_DEFINE(NIS_PLUS, 1, [ ])
	CFLAGS="$CFLAGS -DSOLARIS -D_POSIX_PTHREAD_SEMANTICS"
    ;;
  *-*-sunos*)
    os_sunos=yes
    CFLAGS="$CFLAGS -D_POSIX_PTHREAD_SEMANTICS"
    ;;
  *-sgi-irix5*)
    # Irix stuff from snabb@niksula.hut.fi, tsurmacz@asic.ict.pwr.wroc.pl,
    # C.Martin@sheffield.ac.uk, raistlin@uni-paderborn.de
    no_libsocket=yes
    no_libsun=yes
    no_libnsl=yes
    # force /etc/shadow support. they can run /sbin/pwconv at any time.
    # if they do so, don't let sshd down:-) appro@fy.chalmers.se
    AC_DEFINE(HAVE_ETC_SHADOW, 1, [ ])
    no_shadows_password_checking=yes
    ;;
  *-sgi-irix6*)
    # from d-champion@uchicago.edu
    no_libsocket=yes
    no_libnsl=yes
    no_libsun=yes
    if test -z "$GCC";then
        if test "`uname -s`" = "IRIX64"; then
            CFLAGS="-n32 -D_LONG_LONG_LIMB $CFLAGS"
            LDFLAGS="-n32 $LDFLAGS"
        else
            CFLAGS="-n32 $CFLAGS"
            LDFLAGS="-n32 $LDFLAGS"
        fi
    fi
    # force /etc/shadow support. they can run /sbin/pwconv at any time.
    # if they do so, don't let sshd down:-) appro@fy.chalmers.se
    AC_DEFINE(HAVE_ETC_SHADOW, 1, [ ])
    no_shadows_password_checking=yes
    # We want support for <proj.h> eivind@ii.uib.no
    AC_TRY_COMPILE([#include <proj.h>],
		[int foo = MAXPROJNAMELEN; ],
		AC_DEFINE(HAVE_SGI_PROJ_H, 1, [ ]) )
    ;;
  *-ibm-aix3.2|*-ibm-aix3.2.0|*-ibm-aix3.2.1|*-ibm-aix3.2.2|*-ibm-aix3.2.3|*-ibm-aix3.2.4)
    os_aix=yes
    AC_CHECK_LIB(s, getuserattr)
    ;;
  *-ibm-aix4.2|*-ibm-aix4.2.*|*-ibm-aix4.3*|*-ibm-aix4.4*)
    no_utmpx=yes
    os_aix=yes
    AC_CHECK_LIB(s, getuserattr)
    ;;
  *-ibm-aix*)
    os_aix=yes
    AC_CHECK_LIB(s, getuserattr)
    ;;
  mips-dec-mach3*)
    # Mach3 stuff from kivinen@hut.fi
    no_vhangup=yes
    no_setsid=yes
    ;;
  *-dec-ultrix*)
    # Ultrix stuff from dmckilli@qc.bell.ca, jbotz@orixa.mtholyoke.edu,
    # corey@cac.washington.edu
    AC_DEFINE(O_NONBLOCK_BROKEN, 1, [ ])
    AC_DEFINE(HAVE_ULTRIX_SHADOW_PASSWORDS, 1, [ ])
    no_vhangup=yes
    no_utmpx=yes
    no_termios=yes
    # Ultrix shadow passwords implemented in auth-passwd.c.
    no_shadows_password_checking=yes
    AC_CHECK_LIB(auth, authenticate_user)
    AC_TRY_COMPILE([#include <syslog.h>],
		   [int foo = LOG_DAEMON; ],
		   ,
		   AC_DEFINE(NEED_SYS_SYSLOG_H, 1, [ ]) )
    ;;
  *-*-hpux7.*)
    # Kludge for ancient HPUX 7.x (from Nenad Babajic <saca@uis0.uis.kg.ac.yu>)
    AC_DEFINE(HPSUX7_KLUDGES, 1, [ ])
    # HPUX flags from jgotts@engin.umich.edu
    if test -z "$GCC"; then
      CFLAGS="$CFLAGS -Aa -D_HPUX_SOURCE"
    fi
    AC_DEFINE(NONSTANDARD_IP_ADDRESS_X11_KLUDGE, 1, [ ])
    AC_DEFINE(HPSUX_NONSTANDARD_X11_KLUDGE, 1, [ ])
    ;;
  *-*-hpux*)
    # HPUX flags from jgotts@engin.umich.edu
    if test -z "$GCC"; then
      CFLAGS="$CFLAGS -Ae -D_HPUX_SOURCE"
    fi
    AC_MSG_CHECKING(for HPUX tcb auth option)
    if test -f /tcb/files/auth/system/pw_id_map; then
      AC_MSG_RESULT(yes)
      AC_DEFINE(HAVE_HPUX_TCB_AUTH, 1, [ ])
      LIBS="$LIBS -lsec"
    else
      AC_MSG_RESULT(no)
    fi
    AC_MSG_CHECKING(for keyserv)
    if test -f /usr/sbin/keyserv; then
      AC_MSG_RESULT(yes)
      AC_DEFINE(SECURE_RPC, 1, [ ])
      LIBS="$LIBS -lrpcsvc"
    else
      AC_MSG_RESULT(no)
    fi
    AC_DEFINE(NONSTANDARD_IP_ADDRESS_X11_KLUDGE, 1, [ ])
    AC_DEFINE(HPSUX_NONSTANDARD_X11_KLUDGE, 1, [ ])
    no_utmpx=yes
    ;;
  alpha-dec-osf*)
    AC_DEFINE(TTY_GROUP, "terminal", 1, [ ])
    # The man page says that we need -lsecurity -ldb -laud -lm to quickstart
    # programs using enchanced security. 
    AC_CHECK_LIB(security, set_auth_parameters)
    AC_CHECK_LIB(aud, audgen)
    AC_CHECK_LIB(db, dbopen)
    AC_CHECK_LIB(m, sin)
    AC_CHECK_FUNCS(setluid)
    no_utmpx=yes
    AC_DEFINE(HAVE_SIA, 1, [ ])
    SSHDCONFOBJS="$SSHDCONFOBJS sshsia.o"
    OLD_CFLAGS="$CFLAGS"
    CFLAGS="$CFLAGS -I."
    AC_TRY_COMPILE([#include <sys/types.h>
#include <sys/security.h>
#include <prot.h>], , , [
      AC_MSG_WARN([Could not include <prot.h> file.
The digital unix 4.0 prot.h tries to include <acl.h> instead of <sys/acl.h>.
Making a local copy of prot.h and patching it.])
      sed 's@<acl.h>@<sys/acl.h>@g' </usr/include/prot.h >prot.h
    ])
    CFLAGS="$OLD_CFLAGS"
    ;;
  *-*-nextstep*)
    # Nextstep support from a person who wants to remain anonymous
    no_termios=yes
    if test -f /usr/include/bsd/sys/termios.h; then
      AC_DEFINE(SPEED_T_IN_STDTYPES_H, 1, [ ])
    fi
    ;;
  *-*-linux*|*-*-mklinux*)
    CFLAGS="-D_GNU_SOURCE $CFLAGS"
    AC_CHECK_FUNC(getspnam)
    if test $ac_cv_func_getspnam = no; then
       AC_CHECK_LIB(shadow, getspnam)
    fi
    if test $ac_cv_func_getspnam = yes; then
      AC_DEFINE(HAVE_ETC_SHADOW, 1, [ ])
    fi
    no_shadows_password_checking=yes
    AC_CHECK_FUNCS(pw_encrypt, pwencrypt=yes)
    if test $ac_cv_func_pw_encrypt = no; then
      AC_CHECK_LIB(shadow, pw_encrypt, [
	pwencrypt=yes
	pwencryptlib="-lshadow" ])
    fi
    AC_MSG_CHECKING([whether to enable pw_encrypt])
    AC_ARG_ENABLE(deprecated-linux-pw-encrypt,
[  --enable-deprecated-linux-pw-encrypt
                          Enable using of deprecated linx pw_encrypt function.],
[     if test -z "$pwencrypt"; then
	AC_MSG_RESULT(no)
      else
	AC_DEFINE(crypt,pw_encrypt, 1, [ ])
	AC_MSG_RESULT(no)
	if test -n "$pwencryptlib"; then
	  LIBS="$LIBS $pwencryptlib"
	fi
      fi ],
      AC_MSG_RESULT(no) )
    ;;
  i*86-*-bsdi2.1*)
    no_pipe=yes
    X_EXTRA_LIBS="$X_EXTRA_LIBS -lipc"
    ;;
  i*86-*-bsdi*)
    no_pipe=yes
    ;;
  i*86-unknown-bsd*)
    # Assume 386BSD.  pgut01@cs.auckland.ac.nz reported this makes it compile.
    AC_DEFINE(__FreeBSD__, 1, [ ])
	CFLAGS="$CFLAGS -DFREEBSD"
    ;;
  mips-sony-newsos6)
    AC_DEFINE(HAVE_NO_TZ_IN_GETTIMEOFDAY, 1, [ ])
    ;;
  m68k-sony-newsos*)
    # From snabb@niksula.hut.fi
    no_vhangup=yes
    ;;
  mips-sony-bsd*)
    no_vhangup=yes
    ;;
  m68k-apple-aux*)
    # Macintosh A/UX tweaks - prune LIB = -L/usr/local/lib, exceeds ld capacity
    #   add -lposix to it, needed for termio stuff
    LIBS="$LIBS -lposix"
    AC_DEFINE(_POSIX_SOURCE, 1, [ ])
    ;;
  m88k-dg-dgux*)
    AC_DEFINE(BROKEN_INET_ADDR, 1, [ ])
    ;;
  t3e-*-*)
    CFLAGS="$CFLAGS -I."
    LIBS="$LIBS -lrsc"
    ;;
  *-cray-unicos*)
    CFLAGS="$CFLAGS -DCRAY_STACKSEG_END=_getb67"
    LIBS="$LIBS -lrsc"
    ;;
  i386-sni-sysv4*)
    AC_DEFINE(NONSTANDARD_IP_ADDRESS_X11_KLUDGE, 1, [ ])
    AC_CHECK_LIB(gen, openlog)
    ;;
  *-*-sysv4*)
    AC_CHECK_LIB(gen, openlog)
    ;;
  *-*-machten*)
    AC_DEFINE(USE_STRLEN_FOR_AF_UNIX, 1, [ ])
    no_vhangup=yes
    ;;
  i*86-unknown-sco3.2v4*)
    # From moi@dio.com
    AC_DEFINE(HAVE_SCO_ETC_SHADOW, 1, [ ])
    AC_DEFINE(SCO, 1, [ ])
    no_ranlib=yes
    LIBS="-L/usr/lib/libp -lprot -lx $LIBS"
    CFLAGS="$CFLAGS -UM_I86SM"
    ;;
  i*86-unknown-sco3.2v5*)
    # From brian@ilinx.com
    AC_DEFINE(HAVE_SCO_ETC_SHADOW, 1, [ ])
    AC_DEFINE(SCO, 1, [ ])
    AC_DEFINE(SCO5, 1, [ ])
    no_ranlib=yes
    LIBS="-L/usr/lib/libp -lprot -lx $LIBS"
    CFLAGS="$CFLAGS -UM_I86SM"
    ;;
  *-convex-bsd*)
    # From mark.martinec@nsc.ijs.si
    # On Convex, getpwnam sets pw_passwd if running as root
    no_shadows_password_checking=yes
    ;;
  *-sequent-ptx*)
    # From geek+@cmu.edu
    test_libinet=yes
    ;;
  *-sequent-dynix*|*-sequent-ptx*)
    LIBS="-lseq -lsec $LIBS"
    X_EXTRA_LIBS="-lsocket -linet -lnsl $X_EXTRA_LIBS"
    ;;
  *-*-freebsd*)
    # From Ollivier Robert: FreeBSD and NetBSD use master.passwd, but set
    # pw_passwd automatically when running as root.
    no_shadow_password_checking=yes
	CFLAGS="$CFLAGS -DFREEBSD"
    ;;
  *-*-netbsd*|*-*-openbsd*)
    # From Ollivier Robert: FreeBSD and NetBSD use master.passwd, but set
    # pw_passwd automatically when running as root.
    no_shadow_password_checking=yes
    ;;
  *-*-cygwin*)
    CFLAGS="$CFLAGS -DCYGWIN"
	LIBS="$LIBS"
	;;
  *)
    ;;
esac

export CFLAGS CC

# Socket pairs appear to be broken on several systems.  I don't know exactly
# where, so I'll use pipes everywhere for now.
#AC_DEFINE(USE_PIPES, 1, [ ])

dnl AC_MSG_CHECKING([that the compiler works])
dnl AC_TRY_RUN([ main(int ac, char **av) { return 0; } ],
dnl     AC_MSG_RESULT(yes),
dnl     AC_MSG_RESULT(no)
dnl     AC_MSG_ERROR(Could not compile and run even a trivial ANSI C program - check CC.),
dnl     AC_MSG_ERROR(Could not compile and run even a trivial ANSI C program - check CC.))
dnl 
if test -z "$no_pipe"; then
if test -n "$GCC"; then
  AC_MSG_CHECKING([if the compiler understands -pipe])
  OLDCC="$CC"
  CC="$CC -pipe"
  AC_TRY_COMPILE(,,
    AC_MSG_RESULT(yes),
    CC="$OLDCC"
    AC_MSG_RESULT(no))
fi
fi

AC_MSG_CHECKING([whether to enable -Wall])
AC_ARG_ENABLE(warnings,
[  --enable-warnings       Enable -Wall if using gcc.],
[ if test -n "$GCC"; then 
    AC_MSG_RESULT(adding -Wall to CFLAGS.)
    CFLAGS="$CFLAGS -Wall"
  fi],AC_MSG_RESULT(no))

AC_TYPE_SIGNAL
AC_TYPE_SIZE_T
AC_TYPE_UID_T
AC_TYPE_OFF_T
AC_TYPE_MODE_T
AC_STRUCT_ST_BLKSIZE

AC_C_CONST
AC_C_INLINE
AC_C_BIGENDIAN
AC_CHECK_SIZEOF(long,4)
AC_CHECK_SIZEOF(int,4)
AC_CHECK_SIZEOF(short,2)

if test -z "$no_termios"; then
  AC_CHECK_HEADERS(termios.h)
fi

if test -z "$no_utmpx"; then
  AC_CHECK_HEADERS(utmpx.h)
  AC_MSG_CHECKING([whether utmpx have ut_syslen field])
  AC_EGREP_HEADER(ut_syslen, utmpx.h, [ AC_DEFINE(HAVE_SYSLEN_IN_UTMPX, 1, [ ])
    AC_MSG_RESULT(yes)], AC_MSG_RESULT(no))
fi

AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(unistd.h rusage.h sys/time.h lastlog.h utmp.h shadow.h)
AC_CHECK_HEADERS(sgtty.h sys/select.h sys/ioctl.h machine/endian.h)
AC_CHECK_HEADERS(paths.h usersec.h utime.h netinet/in_systm.h)
AC_CHECK_HEADERS(netinet/in_system.h netinet/ip.h netinet/tcp.h ulimit.h)
AC_CHECK_HEADERS(sys/resource.h login_cap.h sys/stream.h sys/conf.h)
AC_HEADER_TIME
AC_HEADER_DIRENT
AC_HEADER_STAT
AC_HEADER_MAJOR
AC_MSG_CHECKING([whether utmp have ut_pid field])
AC_EGREP_HEADER(ut_pid, utmp.h, [ AC_DEFINE(HAVE_PID_IN_UTMP, 1, [ ])
  AC_MSG_RESULT(yes)], AC_MSG_RESULT(no))
AC_MSG_CHECKING([whether utmp have ut_name field])
AC_EGREP_HEADER(ut_name, utmp.h, [ AC_DEFINE(HAVE_NAME_IN_UTMP, 1, [ ])
  AC_MSG_RESULT(yes)], AC_MSG_RESULT(no))
AC_MSG_CHECKING([whether utmp have ut_id field])
AC_EGREP_HEADER(ut_id, utmp.h, [ AC_DEFINE(HAVE_ID_IN_UTMP, 1, [ ])
  AC_MSG_RESULT(yes)], AC_MSG_RESULT(no))
AC_MSG_CHECKING([whether utmp have ut_host field])
AC_EGREP_HEADER(ut_host, utmp.h, [ AC_DEFINE(HAVE_HOST_IN_UTMP, 1, [ ])
  AC_MSG_RESULT(yes)], AC_MSG_RESULT(no))
AC_MSG_CHECKING([whether utmp have ut_addr field])
AC_EGREP_HEADER(ut_addr, utmp.h, [ AC_DEFINE(HAVE_ADDR_IN_UTMP, 1, [ ])
  AC_MSG_RESULT(yes)], AC_MSG_RESULT(no))
AC_MSG_CHECKING([whether you have incompatible SIGINFO macro])
AC_EGREP_CPP([p_siginfo], [#include <sys/siginfo.h>
SIGINFO(p,1)], 
[ AC_DEFINE(HAVE_INCOMPATIBLE_SIGINFO, 1, [ ])
  AC_MSG_RESULT(yes)] , AC_MSG_RESULT(no))

AC_CHECK_LIB(c, crypt, [true], AC_CHECK_LIB(crypt, crypt))
AC_CHECK_LIB(sec, getspnam)
AC_CHECK_LIB(seq, get_process_stats)
AC_CHECK_LIB(bsd, bcopy)
if test -z "$no_libnsl"; then
  AC_CHECK_LIB(nsl, main)
fi
if test -n "$test_libinet"; then
  AC_CHECK_LIB(inet, inet_network)
fi
if test -z "$no_libsocket"; then
  AC_CHECK_LIB(socket, socket)
fi
if test -z "$no_libsun"; then
  AC_CHECK_LIB(sun, getpwnam)
fi
if test -z "$no_libbsd"; then
  AC_CHECK_LIB(bsd, openpty)
fi
AC_CHECK_LIB(util, login, AC_DEFINE(HAVE_LIBUTIL_LOGIN, 1, [ ])
			  LIBS="$LIBS -lutil")

if test -z "$no_vhangup"; then
  AC_CHECK_FUNCS(vhangup)
fi

if test -z "$no_setsid"; then
  AC_CHECK_FUNCS(setsid)
fi

AC_CHECK_FUNCS(gettimeofday times getrusage ftruncate revoke makeutx)
AC_CHECK_FUNCS(strchr memcpy setlogin openpty _getpty clock fchmod ulimit)
AC_CHECK_FUNCS(gethostname getdtablesize umask innetgr initgroups setpgrp)
AC_CHECK_FUNCS(setpgid daemon waitpid ttyslot authenticate getpt isastream)

AC_REPLACE_FUNCS(strerror memmove remove random putenv crypt socketpair snprintf)

AC_PROG_LN_S
AC_PROG_INSTALL
AC_CHECK_PROG(AR, ar, ar, echo)
if test -z "$no_ranlib"; then
  AC_PROG_RANLIB
else
  RANLIB=":"
fi
AC_PROG_LIBTOOL
AC_PROGRAMS_CHECK(MAKEDEP, makedepend makedep, makedepend)

AC_PATH_XTRA

AC_PATH_PROG(PASSWD_PATH, passwd)
if test -n "$PASSWD_PATH"; then
  AC_DEFINE_UNQUOTED(PASSWD_PATH, "$PASSWD_PATH", [ ])
fi

AC_PATH_PROG(XAUTH_PATH, xauth)
if test -n "$XAUTH_PATH"; then
  AC_DEFINE_UNQUOTED(XAUTH_PATH, "$XAUTH_PATH", [ ])
fi

if test "$no_x" = yes; then
  X_PROGRAMS=""
else
  if test "x$XAUTH_PATH" = "x"; then
    AC_MSG_ERROR(configuring with X but xauth not found - aborting, make sure xauth is in your path or add --without-x to configure to disable X11)
  fi
  X_PROGRAMS="ssh-askpass"
fi
AC_MSG_CHECKING(for X11 unix domain socket directory)
AC_SUBST(X_PROGRAMS)
if test '!' -d /tmp/.X11-unix; then
  if test -d /var/X/.X11-unix; then
    AC_DEFINE(X11_DIR, "/var/X/.X11-unix", 1, [ ])
    AC_MSG_RESULT(/var/X/.X11-unix)
  else
    if test -d /tmp/.X11-pipe; then
      AC_DEFINE(X11_DIR, "/tmp/.X11-pipe", 1, [ ])
      AC_MSG_RESULT(/tmp/.X11-pipe)
    else
      AC_MSG_RESULT(/tmp/.X11-unix)
    fi
  fi
else
  AC_MSG_RESULT(/tmp/.X11-unix)
fi

AC_PATH_PROGS(PERL, perl5 perl, not found)
if test "x$PERL" = "xnot found" || $PERL -e 'exit ($] >= 5)'; then
  unset ac_cv_path_PERL
  PERL="/usr/local/bin/perl"
  AC_MSG_WARN(perl version 5 not found - make-ssh-known-hosts will not work)
fi

AC_CHECK_FUNCS(getpseudotty)
AC_MSG_CHECKING(for pseudo ttys)
if test -c /dev/getpty && test $ac_cv_func_getpseudotty = yes
then
  AC_DEFINE(HAVE_GETPSEUDOTTY, 1, [ ])
  AC_MSG_RESULT(getpseudotty)
else
if test -c /dev/ptmx && test -c /dev/pts/0
then
  AC_DEFINE(HAVE_DEV_PTMX, 1, [ ])
  AC_MSG_RESULT(streams ptys)
else
if test -c /dev/ptc && test -c /dev/pts || test -d /dev/pts
then
  AC_DEFINE(HAVE_DEV_PTS_AND_PTC, 1, [ ])
  AC_MSG_RESULT(/dev/pts and /dev/ptc)
else
if test -c /dev/ptyp10
then
  AC_DEFINE(HAVE_DEV_PTYP10, 1, [ ])
  AC_MSG_RESULT(sco-style ptys)
else
  AC_MSG_RESULT(bsd-style ptys)
fi
fi
fi
fi

AC_MSG_CHECKING(for /etc/default/login)
if test -f /etc/default/login; then
  AC_DEFINE(HAVE_ETC_DEFAULT_LOGIN, 1, [ ])
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
fi

if test -z "$no_shadows_password_checking"; then
  AC_MSG_CHECKING(for shadow passwords)
  if test -f /etc/shadow; then
      # If we don't have shadow.h, this might be some nonstandard
      # kludging... So better check it out.
    if test "x$ac_cv_header_shadow_h" = "xyes"; then
      AC_DEFINE(HAVE_ETC_SHADOW, 1, [ ])
      AC_MSG_RESULT(/etc/shadow)
      # SunOS C2 security uses this style of shadow passwords, but does not
      # have getspent in a system library.  However, a libshadow.a library
      # contaning these is publicly available.
      AC_CHECK_LIB(shadow, getspent)
      AC_MSG_CHECKING([whether spwd have sp_expire field])
      AC_EGREP_HEADER(sp_expire, shadow.h, [ AC_DEFINE(HAVE_STRUCT_SPWD_EXPIRE, 1, [ ])
      	AC_MSG_RESULT(yes)], AC_MSG_RESULT(no))
      AC_MSG_CHECKING([whether spwd have sp_inact field])
      AC_EGREP_HEADER(sp_inact, shadow.h, [ AC_DEFINE(HAVE_STRUCT_SPWD_INACT, 1, [ ])
      	AC_MSG_RESULT(yes)], AC_MSG_RESULT(no))
    else
      AC_MSG_RESULT(no)
    fi
  elif test -n "$os_aix"; then
    AC_DEFINE(HAVE_ETC_SECURITY_PASSWD, 1, [ ])
    AC_MSG_RESULT(/etc/security/passwd)
  elif test -n "$os_sunos"; then
    AC_DEFINE(HAVE_ETC_SECURITY_PASSWD_ADJUNCT, 1, [ ])
    AC_MSG_RESULT(/etc/security/passwd.adjunct)
  else
    AC_MSG_RESULT(no)
  fi
fi

AC_MSG_CHECKING(location of mail spool files)
for dir in /var/spool/mail /var/mail /usr/spool/mail /usr/mail FILE
do
  if test "$dir" = "FILE"; then
    if test -f $HOME/.MailBox; then
      AC_MSG_WARN(mail spool directory was not found: assuming you use \$HOME/.MailBox)
      AC_DEFINE_UNQUOTED(MAIL_SPOOL_FILE, ".MailBox", [ ])
      AC_MSG_RESULT(\$HOME/.MailBox)
    else
      if test -f $HOME/Mailbox; then
          AC_MSG_WARN(mail spool directory was not found: assuming you use \$HOME/Mailbox)
          AC_DEFINE_UNQUOTED(MAIL_SPOOL_FILE, "Mailbox", [ ])
          AC_MSG_RESULT(\$HOME/Mailbox)
      else
          AC_MSG_WARN(mail spool directory was not found: assuming you use \$HOME/newmail)
          AC_DEFINE_UNQUOTED(MAIL_SPOOL_FILE, "newmail", [ ])
          AC_MSG_RESULT(\$HOME/newmail)
      fi
    fi
  elif test -d $dir; then
    AC_DEFINE_UNQUOTED(MAIL_SPOOL_DIRECTORY, "$dir", [ ])
    AC_MSG_RESULT($dir)
    break
  fi
done

AC_MSG_CHECKING(location of utmp)
if test -f /var/run/utmp; then
  AC_DEFINE(SSH_UTMP, "/var/run/utmp", 1, [ ])
  AC_MSG_RESULT(/var/run/utmp)
elif test -f /var/log/utmp; then
  AC_DEFINE(SSH_UTMP, "/var/log/utmp", 1, [ ])
  AC_MSG_RESULT(/var/log/utmp)
elif test -f /var/adm/utmp; then
  AC_DEFINE(SSH_UTMP, "/var/adm/utmp", 1, [ ])
  AC_MSG_RESULT(/var/adm/utmp)
elif test -f /usr/adm/utmp; then
  AC_DEFINE(SSH_UTMP, "/usr/adm/utmp", 1, [ ])
  AC_MSG_RESULT(/usr/adm/utmp)
elif test -f /etc/utmp; then
  AC_DEFINE(SSH_UTMP, "/etc/utmp", 1, [ ])
  AC_MSG_RESULT(/etc/utmp)
else
  AC_MSG_RESULT(not found)
fi

AC_MSG_CHECKING(location of wtmp)
if test -f /var/log/wtmp; then
  AC_DEFINE(SSH_WTMP, "/var/log/wtmp", 1, [ ])
  AC_MSG_RESULT(/var/log/wtmp)
elif test -f /var/adm/wtmp; then
  AC_DEFINE(SSH_WTMP, "/var/adm/wtmp", 1, [ ])
  AC_MSG_RESULT(/var/adm/wtmp)
elif test -f /usr/adm/wtmp; then
  AC_DEFINE(SSH_WTMP, "/usr/adm/wtmp", 1, [ ])
  AC_MSG_RESULT(/usr/adm/wtmp)
elif test -f /etc/wtmp; then
  AC_DEFINE(SSH_WTMP, "/etc/wtmp", 1, [ ])
  AC_MSG_RESULT(/etc/wtmp)
else
  AC_DEFINE(SSH_WTMP, "/var/adm/wtmp", 1, [ ])
  AC_MSG_RESULT(not found)
fi

AC_MSG_CHECKING(location of lastlog)
if test -f /var/log/lastlog || test -d /var/log/lastlog; then
  AC_DEFINE(SSH_LASTLOG, "/var/log/lastlog", 1, [ ])
  AC_DEFINE(HAVE_LASTLOG, 1, [ ])
  LASTLOG=/var/log/lastlog
  AC_MSG_RESULT(/var/log/lastlog)
elif test -f /var/adm/lastlog || test -d /var/adm/lastlog; then
  AC_DEFINE(SSH_LASTLOG, "/var/adm/lastlog", 1, [ ])
  AC_DEFINE(HAVE_LASTLOG, 1, [ ])
  LASTLOG=/var/adm/lastlog
  AC_MSG_RESULT(/var/adm/lastlog)
elif test -f /usr/adm/lastlog || test -d /usr/adm/lastlog; then
  AC_DEFINE(SSH_LASTLOG, "/usr/adm/lastlog", 1, [ ])
  AC_DEFINE(HAVE_LASTLOG, 1, [ ])
  LASTLOG=/usr/adm/lastlog
  AC_MSG_RESULT(/usr/adm/lastlog)
elif test -f /etc/lastlog || test -d /etc/lastlog; then
  AC_DEFINE(SSH_LASTLOG, "/etc/lastlog", 1, [ ])
  AC_DEFINE(HAVE_LASTLOG, 1, [ ])
  LASTLOG=/etc/lastlog
  AC_MSG_RESULT(/etc/lastlog)
else
  AC_MSG_RESULT(not found)
  AC_DEFINE(SSH_LASTLOG, "/var/log/lastlog", 1, [ ])
  LASTLOG=/var/log/lastlog
fi

AC_MSG_CHECKING(whether $LASTLOG is a directory)
if test -d $LASTLOG
then
  AC_MSG_RESULT(yes)
  AC_DEFINE(LASTLOG_IS_DIR, 1, [ ])
else
  AC_MSG_RESULT(no)
fi

AC_MSG_CHECKING(whether to include the IDEA encryption algorithm)
AC_ARG_WITH(idea,
[  --with-idea             Use IDEA (default).
  --without-idea          Don't use IDEA: avoids patent problems in commercial use],
[ case "$withval" in
  no)
    AC_MSG_RESULT(no)
    ;;
  *)
    AC_MSG_RESULT(yes)
    AC_DEFINE(WITH_IDEA, 1, [ ])
    CONFOBJS="$CONFOBJS idea.o"
    ;;
  esac ],
#ifndef F_SECURE_COMMERCIAL
  AC_MSG_RESULT(yes)
  AC_DEFINE(WITH_IDEA, 1, [ ])
  CONFOBJS="$CONFOBJS idea.o"
#endif F_SECURE_COMMERCIAL
#ifdef F_SECURE_COMMERCIAL
#
#endif F_SECURE_COMMERCIAL
)

AC_MSG_CHECKING(whether to include the Blowfish encryption algorithm)
AC_ARG_WITH(blowfish,
[  --with-blowfish         Include blowfish (default).
  --without-blowfish      Don't include blowfish],
[ case "$withval" in
  no)
    AC_MSG_RESULT(no)
    ;;
  *)
    AC_MSG_RESULT(yes)
    AC_DEFINE(WITH_BLOWFISH, 1, [ ])
    ;;
  esac ],
  AC_MSG_RESULT(yes)
  AC_DEFINE(WITH_BLOWFISH, 1, [ ])
)

AC_MSG_CHECKING(whether to include the DES encryption algorithm)
AC_ARG_WITH(des,
[  --with-des              Include single-DES support.
  --without-des           Don't allow single DES (default)],
[ case "$withval" in
  yes)
    AC_MSG_RESULT(yes)
    AC_DEFINE(WITH_DES, 1, [ ])
    ;;
  *)
    AC_MSG_RESULT(no)
    ;;
  esac ],
  AC_MSG_RESULT(no)
)

AC_MSG_CHECKING(whether to include the ARCFOUR encryption algorithm)
AC_ARG_WITH(arcfour,
[  --with-arcfour          Include arcfour (DO NOT ENABLE, unless you know the security implications of this settings. See README.CIPHERS for more info).
  --without-arcfour       Don't include arcfour (default)],
[ case "$withval" in
  yes)
    AC_MSG_RESULT(yes)
    AC_DEFINE(WITH_ARCFOUR, 1, [ ])
    ;;
  *)
    AC_MSG_RESULT(no)
    ;;
  esac ],
  AC_MSG_RESULT(no)
)

AC_MSG_CHECKING(whether to include the none encryption algorithm)
AC_ARG_WITH(none,
[  --with-none             Include support for unencrypted connections
  --without-none          Don't allow unencrypted connections (default)],
[ case "$withval" in
  yes)
    AC_MSG_RESULT(yes)
    AC_DEFINE(WITH_NONE, 1, [ ])
    ;;
  *)
    AC_MSG_RESULT(no)
    ;;
  esac ],
  AC_MSG_RESULT(no)
)

AC_MSG_CHECKING(whether to use login)
AC_ARG_WITH(login,
[  --with-login[=PATH]     Use login -f to finish login connections. ],
[ case "$withval" in
    no)
	AC_MSG_RESULT(no)
	;;
    yes)
	AC_MSG_RESULT(yes)
	AC_PATH_PROGS(PATH_LOGIN, login)
	AC_DEFINE(USELOGIN, 1, [ ])
	AC_DEFINE_UNQUOTED(PATH_LOGIN, "$PATH_LOGIN", [ ])
	;;
    *)
	AC_MSG_RESULT($withval)
	AC_DEFINE(USELOGIN, 1, [ ])
	AC_DEFINE_UNQUOTED(PATH_LOGIN, "$withval", [ ])
	;;
  esac ],
  [ AC_MSG_RESULT(no) ]
)

AC_MSG_CHECKING(whether to use rsh)
AC_ARG_WITH(rsh, 
[  --with-rsh=PATH         Specify where to find rsh.
  --without-rsh           Do not use rsh under any conditions. ],
[ case "$withval" in
    no) 
        AC_MSG_RESULT(no)
	;;
    yes) 
	 AC_MSG_RESULT(yes)
	 AC_PATH_PROGS(RSH_PATH, remsh resh rsh)
         AC_DEFINE_UNQUOTED(RSH_PATH, "$RSH_PATH", [ ])
	;;
    *) 
       AC_MSG_RESULT($withval)
       AC_DEFINE_UNQUOTED(RSH_PATH, "$withval", [ ])
       RSH_PATH="$withval"
       ;;
  esac ],
  [ AC_MSG_RESULT(yes)
    AC_PATH_PROGS(RSH_PATH, remsh resh rsh)
    AC_DEFINE_UNQUOTED(RSH_PATH, "$RSH_PATH", [ ]) ]
)

# Check that we didn't accidentally get ssh as rsh.
if test -n "$RSH_PATH"; then
  if test -n "`$RSH_PATH </dev/null 2>&1 | grep listen-port:host:port`"; then
    AC_MSG_WARN(Found rsh in $RSH_PATH but it appears to actually be ssh.)
    AC_MSG_ERROR(Probably forgot to specify --with-rsh=PATH-TO-REAL-RSH.)
  fi
fi

# Code to permit setting default path for users (alden@math.ohio-state.edu)
AC_MSG_CHECKING(default path)
AC_ARG_WITH(path,
[  --with-path=PATH        Default path passed to user shell by sshd.],
[ case "$withval" in
  no)
    AC_MSG_RESULT(use system default)
    ;;
  *)
    AC_MSG_RESULT($withval)
    AC_DEFINE_UNQUOTED(DEFAULT_PATH, "$withval", [ ])
    ;;
  esac ],
  AC_MSG_RESULT(use system default)
)

AC_MSG_CHECKING(etcdir)
AC_ARG_WITH(etcdir,
[  --with-etcdir=PATH      Directory containing ssh system files (default /etc).],
[ case "$withval" in
  no)
    AC_MSG_ERROR(Need ETCDIR.)
    ;;
  yes)
    ETCDIR="/etc"
    AC_MSG_RESULT(/etc)
    ;;
  *)
    ETCDIR="$withval"
    AC_MSG_RESULT($withval)
    ;;
  esac ],
  ETCDIR="/etc"
  AC_MSG_RESULT(/etc)
)

AC_MSG_CHECKING(whether to use nologin.allow file to override nologin)
AC_ARG_WITH(nologin-allow,
[  --with-nologin-allow[=PATH]     If a nologin.allow override should be used (default /etc/nologin.allow). ],
[ case "$withval" in
    no)
	AC_MSG_RESULT(no)
	;;
    yes)
	AC_MSG_RESULT(yes)
	AC_DEFINE_UNQUOTED(NOLOGIN_ALLOW, "/etc/nologin.allow", [ ])
	;;
    *)
	AC_MSG_RESULT($withval)
	AC_DEFINE_UNQUOTED(NOLOGIN_ALLOW, "$withval", [ ])
	;;
  esac ],
  [ AC_MSG_RESULT(no) ]
)

AC_MSG_CHECKING(whether to support SecurID)
AC_ARG_WITH(securid,
[  --with-securid[=PATH]   Enable support for Security Dynamics SecurID card.],
[ case "$withval" in
  no)
    AC_MSG_RESULT(no)
    ;;
  yes)
    AC_MSG_RESULT(yes)
    if test '!' -f /usr/ace/sdiclient.a; then
      AC_ERROR(SecurID sdiclient.a not found in /usr/ace: you must supply the path.)
    fi
    AC_MSG_RESULT(yes)
    AC_MSG_RESULT(Assuming SecurID headers and libraries are in /usr/ace.)
    AC_DEFINE(HAVE_SECURID, 1, [ ])
    CFLAGS="$CFLAGS -I/usr/ace"
    LIBS="/usr/ace/sdiclient.a $LIBS"
    ;;
  *)
    AC_MSG_RESULT(yes)
    if test '!' -f $withval/sdiclient.a; then
      AC_ERROR(SecurID sdiclient.a not found in $withval: please supply the correct path.)
    fi
    AC_MSG_RESULT(Assuming SecurID headers and libraries are in $withval.)
    AC_DEFINE(HAVE_SECURID, 1, [ ])
    CFLAGS="$CFLAGS -I$withval"
    LIBS="$withval/sdiclient.a $LIBS"
    ;;
  esac ],
  AC_MSG_RESULT(no)
)

AC_MSG_CHECKING(whether to support TIS authentication server)
AC_ARG_WITH(tis,
[  --with-tis[=DIR]        Enable support for TIS authentication server.],
[ case "$withval" in
  no)
    AC_MSG_RESULT(no)
    ;;
  *)
    AC_MSG_RESULT(yes)
    if test '!' -f $withval/libauth.a -a -f $withval/libfwall.a -a -f $withval/firewall.h; then
      AC_ERROR(TIS libauth.a or libfwall.a or firewall.h not found in $withval: please supply the correct path.)
    fi
    AC_MSG_RESULT(Assuming TIS headers and libraries are in $withval.)
    AC_DEFINE(HAVE_TIS, 1, [ ])
    CFLAGS="$CFLAGS -I$withval -DHAVE_TIS"
    LIBS="-L$withval -lauth -lfwall $LIBS"
    AC_MSG_WARN(Remember to read README.TIS. The connection between sshd and TIS authentication
server is clear text!)
    ;;
  esac ],
  AC_MSG_RESULT(no)
)

AC_MSG_CHECKING(whether to use Kerberos)
AC_ARG_WITH(kerberos5,
[  --with-kerberos5=[KRB_PREFIX] Compile in Kerberos5 support.],
[ case "$withval" in
  yes)
    with_kerberos5=/usr/local
    ;;
  esac ],
[ with_kerberos5=no ]
)
case "$with_kerberos5" in
no)
  AC_MSG_RESULT(no)
  ;;
*)
  AC_MSG_RESULT(yes)
  AC_DEFINE(KERBEROS, 1, [ ])
  AC_DEFINE(KRB5, 1, [ ])
  KERBEROS_ROOT="$with_kerberos5"
  KERBEROS_INCS="-I${KERBEROS_ROOT}/include"
  KERBEROS_LIBS="-L${KERBEROS_ROOT}/lib -lgssapi_krb5 -lkrb5 -lcrypto -lcom_err"
  AC_CHECK_LIB(ndbm, dbm_open, KERBEROS_LIBS="$KERBEROS_LIBS -lndbm")
  KERBEROS_OBJS="auth-kerberos.o"
  ;;
esac
AC_SUBST(KERBEROS_ROOT)
AC_SUBST(KERBEROS_INCS)
AC_SUBST(KERBEROS_LIBS)
AC_SUBST(KERBEROS_OBJS)

AC_MSG_CHECKING(whether to enable passing the Kerberos TGT)
AC_ARG_ENABLE(kerberos-tgt-passing,
[  --enable-kerberos-tgt-passing  Pass Kerberos ticket-granting-ticket.],
[ case "$enableval" in
  no)
    AC_MSG_RESULT(no)
    ;;
  *)
    if test "$with_kerberos5" = no ; then
      AC_MSG_RESULT(no)
      AC_MSG_WARN("Passing Kerberos TGT requires Kerberos5 support.")
    else
      AC_MSG_RESULT(yes)
      AC_DEFINE(KERBEROS_TGT_PASSING, 1, [ ])
    fi
    ;;
  esac ],
  AC_MSG_RESULT(no)
)

AC_MSG_CHECKING(whether to use libwrap)
AC_ARG_WITH(libwrap,
[  --with-libwrap[=PATH]   Compile in libwrap (tcp_wrappers) support.],
[ case "$withval" in
  no)
    AC_MSG_RESULT(no)
    ;;
  yes)
    AC_MSG_RESULT(yes)
    AC_CHECK_LIB(wrap, request_init, [
	AC_DEFINE(LIBWRAP, 1, [ ])
        WRAPLIBS="-lwrap"
        AC_DEFINE(HAVE_LIBWRAP, 1, [ ]) ])
    ;;
  *)
    AC_MSG_RESULT(yes)
    AC_DEFINE(LIBWRAP, 1, [ ])
    if test -d "$withval"; then
    	WRAPLIBS="-L$withval -lwrap"
    else
	WRAPLIBS="$withval"
    fi
    OLDLIBS="$LIBS"
    LIBS="$WRAPLIBS $LIBS"
    AC_TRY_LINK([ int allow_severity; int deny_severity; ],
                [ hosts_access(); ],
		[],
		[ AC_MSG_ERROR(Could not find the $withval library.  You must first install tcp_wrappers.) ])
    LIBS="$OLDLIBS"
    ;;
  esac ],
  AC_MSG_RESULT(no)
)

AC_SUBST(WRAPLIBS)

AC_MSG_CHECKING(whether to support SOCKS)
AC_ARG_WITH(socks,
[  --with-socks            Compile with SOCKS firewall traversal support.],
[ case "$withval" in
  no)
    AC_MSG_RESULT(no)
    ;;
  yes)
    AC_MSG_RESULT(yes)
    AC_CHECK_LIB(socks5, SOCKSconnect, [
	    socks=5
	    LIBS="-lsocks5 $LIBS"], [
	AC_CHECK_LIB(socks, Rconnect, [
	    socks=4
	    LIBS="-lsocks $LIBS"], [
		AC_MSG_ERROR(Could not find socks library.  You must first install socks.) ] ) ] )
    ;;
  esac ],
  AC_MSG_RESULT(no)
)

if test "x$socks" = "x"; then
	AC_MSG_CHECKING(whether to support SOCKS5)
	AC_ARG_WITH(socks5,
	[  --with-socks5[=PATH]    Compile with SOCKS5 firewall traversal support.],
	[ case "$withval" in
	  no)
	    AC_MSG_RESULT(no)
	    ;;
	  *)
	    AC_MSG_RESULT(yes)
	    socks=5
	    if test "x$withval" = "xyes"; then
	      withval="-lsocks5"
	    else
	      if test -d "$withval"; then
		if test -d "$withval/include"; then
		  CFLAGS="$CFLAGS -I$withval/include"
		else
		  CFLAGS="$CFLAGS -I$withval"
		fi
	        if test -d "$withval/lib"; then
		  withval="-L$withval/lib -lsocks5"
		else
		  withval="-L$withval -lsocks5"
		fi
	      fi
	    fi
	    LIBS="$withval $LIBS"
	    # If Socks was compiled with Kerberos support, we will need
	    # to link against kerberos libraries.  Temporarily append
	    # to LIBS.  This is harmless if there is no kerberos support.
	    TMPLIBS="$LIBS"
	    LIBS="$LIBS $KERBEROS_LIBS"
	    AC_TRY_LINK([],
	                [ SOCKSconnect(); ],
			[],
			[ AC_MSG_ERROR(Could not find the $withval library.  You must first install socks5.) ])
	    LIBS="$TMPLIBS"
	    ;;
	  esac ],
	  AC_MSG_RESULT(no)
	)
fi

if test "x$socks" = "x"; then
	AC_MSG_CHECKING(whether to support SOCKS4)
	AC_ARG_WITH(socks4,
	[  --with-socks4[=PATH]    Compile with SOCKS4 firewall traversal support.],
	[ case "$withval" in
	  no)
	    AC_MSG_RESULT(no)
	    ;;
	  *)
	    AC_MSG_RESULT(yes)
	    socks=4
	    if test "x$withval" = "xyes"; then
	      withval="-lsocks"
	    else
	      if test -d "$withval"; then
	        withval="-L$withval -lsocks"
	      fi
	    fi
	    LIBS="$withval $LIBS"
	    AC_TRY_LINK([],
	                [ Rconnect(); ],
			[],
			[ AC_MSG_ERROR(Could not find the $withval library.  You must first install socks.) ])
	    ;;
	  esac ],
	  AC_MSG_RESULT(no)
	)
fi


if test "x$socks" = "x4"; then
    AC_DEFINE(SOCKS, 1, [ ])
    AC_DEFINE(SOCKS4, 1, [ ])
fi

if test "x$socks" = "x5"; then
    AC_DEFINE(SOCKS, 1, [ ])
    AC_DEFINE(SOCKS5, 1, [ ])
    AC_DEFINE(Rconnect,SOCKSconnect, 1, [ ])
    AC_DEFINE(Rgetsockname,SOCKSgetsockname, 1, [ ])
    AC_DEFINE(Rgetpeername,SOCKSgetpeername, 1, [ ])
    AC_DEFINE(Rbind,SOCKSbind, 1, [ ])
    AC_DEFINE(Raccept,SOCKSaccept, 1, [ ])
    AC_DEFINE(Rlisten,SOCKSlisten, 1, [ ])
    AC_DEFINE(Rselect,SOCKSselect, 1, [ ])
    AC_DEFINE(Rrecvfrom,SOCKSrecvfrom, 1, [ ])
    AC_DEFINE(Rsendto,SOCKSsendto, 1, [ ])
    AC_DEFINE(Rrecv,SOCKSrecv, 1, [ ])
    AC_DEFINE(Rsend,SOCKSsend, 1, [ ])
    AC_DEFINE(Rread,SOCKSread, 1, [ ])
    AC_DEFINE(Rwrite,SOCKSwrite, 1, [ ])
    AC_DEFINE(Rrresvport,SOCKSrresvport, 1, [ ])
    AC_DEFINE(Rshutdown,SOCKSshutdown, 1, [ ])
    AC_DEFINE(Rlisten,SOCKSlisten, 1, [ ])
    AC_DEFINE(Rclose,SOCKSclose, 1, [ ])
    AC_DEFINE(Rdup,SOCKSdup, 1, [ ])
    AC_DEFINE(Rdup2,SOCKSdup2, 1, [ ])
    AC_DEFINE(Rfclose,SOCKSfclose, 1, [ ])
    AC_DEFINE(Rgethostbyname,SOCKSgethostbyname, 1, [ ])
fi

AC_MSG_CHECKING(whether to use rsaref)
AC_ARG_WITH(rsaref,
[  --with-rsaref[=PATH]    Use RSAREF (try to avoid patent problems in U.S.)
  --without-rsaref        Use normal RSA routines (default). ],
[ case "$withval" in
  no)
       AC_MSG_RESULT(no)
       ;;
  *)
       AC_MSG_RESULT(yes)
       if test "x$withval" = "xyes"; then
	  withval="-lrsaref"
          RSAREFDEP="rsaref2/source/librsaref.a"
          LDFLAGS="-Lrsaref2/source $LDFLAGS"
       else
	  if test -d "$withval"; then
	     withval="-L$withval -lrsaref"
	  fi
       fi
       AC_DEFINE(RSAREF, 1, [ ])
       LIBS="$withval $LIBS"
       ;;
  esac ],
  AC_MSG_RESULT(no)
)

# This allows group writeability in userfile_check_owner_permissions()
AC_MSG_CHECKING(whether to allow group writeability)
AC_ARG_ENABLE(group-writeability,
  [  --enable-group-writeability   Allow group writeability in auth-rsa. ],
  [ AC_MSG_RESULT(yes)
    AC_DEFINE(ALLOW_GROUP_WRITEABILITY, 1, [ ]) ],
  [ AC_MSG_RESULT(no) ]
)

AC_MSG_CHECKING(whether to disable forwardings in server)
AC_ARG_ENABLE(server-port-forwardings,
[  --disable-server-port-forwardings
                          Disable all port forwardings in server (except X11)],
[ case "$enableval" in
  no)
       AC_DEFINE(SSHD_NO_PORT_FORWARDING, 1, [ ])
       AC_MSG_RESULT(yes)
       ;;
  *)   AC_MSG_RESULT(no)
       ;;
  esac ],
  AC_DEFINE(SSHD_NO_PORT_FORWARDING, 1, [ ])
  AC_MSG_RESULT(no)
)

AC_MSG_CHECKING(whether to disable forwardings in client)
AC_ARG_ENABLE(client-port-forwardings,
[  --disable-client-port-forwardings
                          Disable all port forwardings in client (except X11)],
[ case "$enableval" in
  no)
       AC_DEFINE(SSH_NO_PORT_FORWARDING, 1, [ ])
       AC_MSG_RESULT(yes)
       ;;
  *)   AC_MSG_RESULT(no)
       ;;
  esac ],
  AC_DEFINE(SSH_NO_PORT_FORWARDING, 1, [ ])
  AC_MSG_RESULT(no)
)

AC_MSG_CHECKING(whether to disable X11 forwarding in server)
AC_ARG_ENABLE(server-x11-forwarding,
[  --disable-server-x11-forwarding
                          Disable X11 forwarding in server],
[ case "$enableval" in
  no)
       AC_DEFINE(SSHD_NO_X11_FORWARDING, 1, [ ])
       AC_MSG_RESULT(yes)
       ;;
  *)   AC_MSG_RESULT(no)
       ;;
  esac ],
  AC_DEFINE(SSHD_NO_X11_FORWARDING, 1, [ ])
  AC_MSG_RESULT(no)
)

AC_MSG_CHECKING(whether to disable X11 forwarding in client)
AC_ARG_ENABLE(client-x11-forwarding,
[  --disable-client-x11-forwarding
                          Disable X11 forwarding in client],
[ case "$enableval" in
  no)
       AC_DEFINE(SSH_NO_X11_FORWARDING, 1, [ ])
       AC_MSG_RESULT(yes)
       ;;
  esac ],
  AC_DEFINE(SSH_NO_X11_FORWARDING, 1, [ ])
  AC_MSG_RESULT(no)
)

AC_MSG_CHECKING(whether to install ssh as suid root)
AC_ARG_ENABLE(suid-ssh,
[  --enable-suid-ssh       Install ssh as suid root (default)
  --disable-suid-ssh      Install ssh without suid bit],
[ case "$enableval" in
  no)
       AC_MSG_RESULT(no)
       SSHINSTALLMODE=0711
       ;;
  *)   AC_MSG_RESULT(yes)
       SSHINSTALLMODE=04711
       ;;
  esac ],
  AC_MSG_RESULT(yes)
  SSHINSTALLMODE=04711
)

AC_MSG_CHECKING(whether to enable TCP_NODELAY)
AC_ARG_ENABLE(tcp-nodelay,
[  --disable-tcp-nodelay   Disable TCP_NODELAY socket option],
[ case "$enableval" in
  no)
       AC_MSG_RESULT(no)
       ;;
  *)
       AC_DEFINE(ENABLE_TCP_NODELAY, 1, [ ])
       AC_MSG_RESULT(yes)
  esac ],
  AC_MSG_RESULT(yes)
  AC_DEFINE(ENABLE_TCP_NODELAY, 1, [ ])
)

AC_MSG_CHECKING(whether to enable SO_LINGER)
AC_ARG_ENABLE(so-linger,
[  --enable-so-linger      Enable setting SO_LINGER socket option],
[ case "$enableval" in
  yes)
       AC_MSG_RESULT(yes)
       AC_DEFINE(ENABLE_SO_LINGER, 1, [ ])
       ;;
  *)
       AC_MSG_RESULT(no)
  esac ],
  AC_MSG_RESULT(no)
)

AC_MSG_CHECKING(whether to include scp statistics at all)
AC_ARG_WITH(scp-stats,
[  --without-scp-stats     Without scp statistics code],
[ case "$withval" in
  no)
       AC_MSG_RESULT(no)
       ;;
  *)
       AC_MSG_RESULT(yes)
  esac ],
  AC_MSG_RESULT(yes)
)

AC_MSG_CHECKING(whether to enable scp statistics)
AC_ARG_ENABLE(scp-stats,
[  --disable-scp-stats     Disable scp statistics display],
[ case "$enableval" in
  no)
       AC_MSG_RESULT(no)
       ;;
  *)
       AC_MSG_RESULT(yes)
  esac ],
  AC_MSG_RESULT(yes)
)

AC_MSG_CHECKING(whether to enable scp statistics for all files)
AC_ARG_ENABLE(all-scp-stats,
[  --disable-all-scp-stats Disable all files scp statistics display],
[ case "$enableval" in
  no)
       AC_MSG_RESULT(no)
       ;;
  *)
       AC_MSG_RESULT(yes)
  esac ],
  AC_MSG_RESULT(yes)
)

# We include this here only to make it visible in --help; this is only used
# in the gmp subdirectory.
AC_ARG_ENABLE(asm,
  [  --disable-asm           Disable assembly language optimizations. ],
  [])

PIDDIR="./"
AC_MSG_CHECKING(where to put sshd.pid)
if test '!' -d $PIDDIR; then
  PIDDIR="$ETCDIR"
fi
AC_MSG_RESULT($PIDDIR)

AC_CONFIG_SUBDIRS(gmp-2.0.2-ssh-2)

if test '!' -d ../src && '!' -d ../libBBS; then
	AC_MSG_ERROR([can't find bbs source directory])
fi

AC_ARG_ENABLE([site-term],
[  --enable-site-term
                          Turn on site term C file ],
[ siteterm=true ],[ siteterm=false ])
AM_CONDITIONAL([SITETERM], [test x$siteterm = xtrue])


AH_TEMPLATE([LASTLOG_IS_DIR],
			[Define if /var/adm/lastlog or whatever it is called is a directory
   			(e.g. SGI IRIX).])

AH_TEMPLATE([RSAREF],
			[Define to use RSAREF.])

AH_TEMPLATE([RSH_PATH],
			[Define this to be the path of the rsh program to support executing rsh.])

AH_TEMPLATE([XAUTH_PATH],
			[Define this to be the path of the xauth program.])

AH_TEMPLATE([SSH_UTMP],
			[Default path for utmp.  Determined by configure.])

AH_TEMPLATE([SSH_WTMP],
			[Default path for wtmp.  Determined by configure.])

AH_TEMPLATE([SSH_LASTLOG],
			[Default path for lastlog.  Determined by configure.])
AH_TEMPLATE([_POSIX_SOURCE],
			[Define if you need to in order for stat and other things to work.])

m4_include(../deplibs.in)

if test -f "../wrapper.sh"; then
abssrcdir=`pwd`
LIBTOOL="$abssrcdir/../wrapper.sh"
MAKEFLAG="-s"
else
MAKEFLAG=""
fi
AC_SUBST(MAKEFLAG)
AC_SUBST(ETCDIR)
AC_SUBST(PIDDIR)
AC_SUBST(RSAREFDEP)
AC_SUBST(CONFOBJS)
AC_SUBST(SSHCONFOBJS)
AC_SUBST(SSHDCONFOBJS)
AC_SUBST(SSHINSTALLMODE)

AC_OUTPUT(Makefile)
