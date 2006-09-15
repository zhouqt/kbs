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
             phpbbs.wforum.h 
libdir=@PHPBBSLIBPATH@
lib_LTLIBRARIES = libphpbbslib.la
phpdir=@PHPBBSLIBPATH@
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
                          phpbbs.wforum.c 
libphpbbslib_la_LIBADD = -L../../libBBS -lBBS -L../../libsystem -lsystem -L../lib -lbbslib
AM_CFLAGS  = -I$(top_srcdir)/src -I$(top_srcdir)/bbs2www/include -I@PHPBASE@ -I@PHPBASE@/main -I@PHPBASE@/Zend -I@PHPBASE@/TSRM -DCOMPILE_DL_KBS_BBS
CLEANFILES=@EXTRA_CLEAN@
