SUBDIRS = convert
bin_PROGRAMS = innbbsd makedbz mkhistory dbzserver bbsnnrp \
		ctlinnbbsd dbz_query_sample bbslink
innbbsd_SOURCES = nocem.c inndchannel.c innbbsd.c connectsock.c rfc931.c \
		daemon.c file.c pmain.c parsdate.y his.c \
		dbz.c closeonexec.c dbztool.c inntobbs.c echobbslib.c \
		receive_article.c qp_conv.c lang.c r_port.c filter.c \
		wildmat.c convert/hztty-2.0/b2g.o convert/hztty-2.0/b2g_tables.o \
		convert/hztty-2.0/hz2gb.o convert/hztty-2.0/log.o
makedbz_SOURCES = makedbz.c bbslib.c file.c dbz.c closeonexec.c
mkhistory_SOURCES = mkhistory.c bbslib.c file.c his.c dbz.c port.c closeonexec.c
dbzserver_SOURCES = dbzserver.c dbzinnbbsd.c pmain.c connectsock.c bbslib.c his.c \
		daemon.c file.c dbz.c dbztool.c rfc931.c port.c closeonexec.c
bbsnnrp_SOURCES = bbsnnrp.c pmain.c bbslib.c connectsock.c file.c killspam.c
ctlinnbbsd_SOURCES = ctlinnbbsd.c pmain.c bbslib.c connectsock.c file.c
bbslink_SOURCES = bbslink.c pmain.c inntobbs.c echobbslib.c connectsock.c file.c \
		port.c parsdate.c lang.c filter.c wildmat.c
noinst_HEADERS = bbslib.h clibrary.h daemon.h dbz.h externs.h \
		innbbsconf.h innbbsd.h inntobbs.h lang.h nntp.h \
		nocem.h version.h
AM_CFLAGS = -DLINUX -DGETRUSAGE -DUSE_NCM_PATCH -DPGP5 -DDBZSERVER -DFirebirdBBS -DDBZDEBUG -DFILTER -DBIG2GB -DGB2BIG -I../src
LDADD = ../libBBS/.libs/libBBS.a ../libsystem/.libs/libsystem.a
CLEANFILES = @EXTRA_CLEAN@
