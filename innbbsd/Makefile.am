bin_PROGRAMS = innbbsd makedbz mkhistory bbsnnrp dbzserver \
		ctlinnbbsd dbz_query_sample bbslink
EXTRA_DIST = AUTHORS INSTALL doc/* README
CLEANFILES = @EXTRA_CLEAN@
innbbsd_SOURCES = nocem.c inndchannel.c innbbsd.c connectsock.c rfc931.c \
		daemon.c file.c pmain.c parsdate.c his.c \
		dbz.c closeonexec.c dbztool.c inntobbs.c echobbslib.c \
		receive_article.c qp_conv.c lang.c r_port.c filter.c \
		wildmat.c
makedbz_SOURCES = makedbz.c bbslib.c file.c dbz.c closeonexec.c
mkhistory_SOURCES = mkhistory.c bbslib.c file.c his.c dbz.c port.c closeonexec.c
dbzserver_SOURCES = dbzserver.c dbzinnbbsd.c pmain.c connectsock.c bbslib.c his.c \
		daemon.c file.c dbz.c dbztool.c rfc931.c port.c closeonexec.c
bbsnnrp_SOURCES = bbsnnrp.c pmain.c bbslib.c connectsock.c file.c killspam.c
ctlinnbbsd_SOURCES = ctlinnbbsd.c pmain.c bbslib.c connectsock.c file.c
bbslink_SOURCES = bbslink.c pmain.c inntobbs.c echobbslib.c connectsock.c file.c \
		port.c parsdate.y lang.c filter.c wildmat.c
noinst_HEADERS = bbslib.h clibrary.h daemon.h dbz.h externs.h \
		his.h innbbsconf.h innbbsd.h inntobbs.h lang.h nntp.h \
		nocem.h version.h
AM_CFLAGS = -DGETRUSAGE -DUSE_NCM_PATCH -DPGP5 -DDBZDEBUG -DFILTER -DBIG2GB -DGB2BIG -I$(top_srcdir)/src
innbbsd_CFLAGS = -DWITH_ECHOMAIL -DWITH_RECORD_O $(AM_CFLAGS)
bbslink_CFLAGS = -DWITH_ECHOMAIL $(AM_CFLAGS)
dbzserver_CFLAGS = -DDBZSERVER $(AM_CFLAGS)
LDADD=-L../libBBS -lBBS -L../libsystem -lsystem

install-data-local:
	bash linkinnd.sh
