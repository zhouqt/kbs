bin_PROGRAMS = mail2bbs
EXTRA_FILES = mailgate.pl tcp.pl
mail2bbs_SOURCES= mail2bbs.c
LDADD=../libBBS/libBBS.a ../libsystem/libsystem.a -lbsd -lesmtp -lltdl -ldl
CLEANFILES=@EXTRA_CLEAN@
AM_CFLAGS=-I$(top_builddir)/src
