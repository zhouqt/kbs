bin_PROGRAMS = mail2bbs
EXTRA_FILES = mailgate.pl tcp.pl
mail2bbs_SOURCES= mail2bbs.c
LDADD=../libBBS/.libs/libBBS.a ../libsystem/.libs/libsystem.a @ZLIB_LIBS@
CLEANFILES=@EXTRA_CLEAN@
AM_CFLAGS=-I$(top_builddir)/src @ZLIB_INC@
