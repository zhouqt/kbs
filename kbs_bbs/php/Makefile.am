EXTRA_DIST = php_kbs_bbs.h \
             phpbbs_errorno.h \
             phpbbs_announce.h \
             phpbbs_article.h \
             phpbbs_admin.h \
             phpbbs_bm.h \
             phpbbs_board.h \
             phpbbs_file.h \
             phpbbs_friend.h \
             phpbbs_mail.h \
             phpbbs_msg.h \
             phpbbs_post.h \
             phpbbs_reg.h \
             phpbbs_session.h \
             phpbbs_system.h \
             phpbbs_user.h  \
             phpbbs_vote.h \
             phpbbs_wforum.h \
             bbslib.h
libdir=@PHPEXTDIR@
lib_LTLIBRARIES = libphpbbslib.la
phpdir=@PHPEXTDIR@
#php_DATA=funcs.php
libphpbbslib_la_SOURCES = phpbbslib.c \
                          phpbbs_announce.c \
                          phpbbs_article.c \
                          phpbbs_admin.c \
                          phpbbs_bm.c \
                          phpbbs_board.c \
                          phpbbs_file.c \
                          phpbbs_friend.c \
                          phpbbs_mail.c \
                          phpbbs_msg.c \
                          phpbbs_post.c \
                          phpbbs_reg.c \
                          phpbbs_session.c \
                          phpbbs_system.c \
                          phpbbs_user.c  \
                          phpbbs_vote.c \
                          phpbbs_wforum.c \
                          bbslib.c
libphpbbslib_la_LIBADD = $(top_builddir)/libBBS/libBBS.la $(top_builddir)/libsystem/libsystem.la
AM_CFLAGS  = @PHPCFLAGS@ -I$(top_srcdir)/src -DCOMPILE_DL_KBS_BBS
if MACOSX
AM_LDFLAGS = -module
else !MACOSX
AM_LDFLAGS = 
endif !MACOSX
CLEANFILES=@EXTRA_CLEAN@
