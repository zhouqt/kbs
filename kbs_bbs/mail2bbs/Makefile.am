bin_PROGRAMS = mail2bbs qmail2bbs qmailpost
EXTRA_FILES = mailgate.pl tcp.pl
mail2bbs_SOURCES= mail2bbs.c qp_conv.c
qmail2bbs_SOURCES= qmail2bbs.c qp_conv.c
qmailpost_SOURCES= qmailpost.c qp_conv.c
LDADD=../libBBS/.libs/libBBS.a ../libsystem/.libs/libsystem.a
CLEANFILES=@EXTRA_CLEAN@
AM_CFLAGS=-I$(top_builddir)/src

install-exec-local:
	cp -f bbsmail mailgate.pl $(bindir)
