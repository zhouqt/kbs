EXTRA_DIST = php_smth_bbs.h phpbbs.user.h phpbbs.announce.h phpbbs.vote.h phpbbs.system.h \
             phpbbs.board.h phpbbs.article.h phpbbs.msg.h phpbbs.mail.h phpbbs.bm.h
libdir=@PHPBBSLIBPATH@
lib_LTLIBRARIES = libphpbbslib.la
phpdir=@PHPBBSLIBPATH@
#php_DATA=funcs.php
libphpbbslib_la_SOURCES = phpbbslib.c phpbbs.user.c phpbbs.announce.c phpbbs.vote.c phpbbs.system.c \
                          phpbbs.board.c phpbbs.article.c phpbbs.msg.c phpbbs.mail.c phpbbs.bm.c
libphpbbslib_la_LIBADD = -L../../libBBS -lBBS -L../../libsystem -lsystem -L../lib -lbbslib
AM_CFLAGS  = -I../include -I../../src -I@PHPBASE@ -I@PHPBASE@/main -I@PHPBASE@/Zend -I@PHPBASE@/TSRM -DCOMPILE_DL_SMTH_BBS -Wall
CLEANFILES=@EXTRA_CLEAN@
