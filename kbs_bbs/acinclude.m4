dnl Copy from acinclude.m4 of php.
AC_DEFUN([BBS_PROG_SENDMAIL],[
BBS_ALT_PATH=/usr/bin:/usr/sbin:/usr/etc:/etc:/usr/ucblib:/usr/lib
AC_PATH_PROG(PROG_SENDMAIL, sendmail,[], $PATH:$BBS_ALT_PATH)
if test -n "$PROG_SENDMAIL"; then
  AC_DEFINE(HAVE_SENDMAIL,1,[whether you have sendmail])
  AC_DEFINE_UNQUOTED(OWNSENDMAIL,"$PROG_SENDMAIL",[path of sendmail program])
else
  AC_MSG_ERROR(not found)
fi
])

