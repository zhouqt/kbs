EXTRA_DIST = php_kbs_bbs.h \
             phpbbs.errorno.h \
             phpbbs.announce.h \
             phpbbs.article.h \
             phpbbs.admin.h \
             phpbbs.bm.h \
             phpbbs.board.h \
             phpbbs.file.h \
             phpbbs.friend.h \
             phpbbs.mail.h \
             phpbbs.msg.h \
             phpbbs.post.h \
             phpbbs.reg.h \
             phpbbs.session.h \
             phpbbs.system.h \
             phpbbs.user.h  \
             phpbbs.vote.h \
             phpbbs.wforum.h \
			 bbslib.h
libdir=@PHPEXTDIR@
lib_LTLIBRARIES = libphpbbslib.la
phpdir=@PHPEXTDIR@
#php_DATA=funcs.php
libphpbbslib_la_SOURCES = phpbbslib.c \
                          phpbbs.announce.c \
                          phpbbs.article.c \
                          phpbbs.admin.c \
                          phpbbs.bm.c \
                          phpbbs.board.c \
                          phpbbs.file.c \
                          phpbbs.friend.c \
                          phpbbs.mail.c \
                          phpbbs.msg.c \
                          phpbbs.post.c \
                          phpbbs.reg.c \
                          phpbbs.session.c \
                          phpbbs.system.c \
                          phpbbs.user.c  \
                          phpbbs.vote.c \
                          phpbbs.wforum.c \
						  bbslib.c
libphpbbslib_la_LIBADD = $(top_builddir)/libBBS/libBBS.la $(top_builddir)/libsystem/libsystem.la
AM_CFLAGS  = @PHPCFLAGS@ -I$(top_srcdir)/src -DCOMPILE_DL_KBS_BBS
CLEANFILES=@EXTRA_CLEAN@
