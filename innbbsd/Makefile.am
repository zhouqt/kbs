bin_PROGRAMS = innbbsd makedbz mkhistory dbzserver bbsnnrp ctlinnbbsd dbz_query_sample bbslink
innbbsd_SOURCES = nocem.c inndchannel.c innbbsd.c connectsock.c rfc931.c \
		daemon.c file.c pmain.c parsdate.y his.c \
		dbz.c closeonexec.c dbztool.c inntobbs.c echobbslib.c \
		receive_article.c qp_conv.c lang.c r_port.c filter.c \
		wildmat.c
LDADD = -L../libBBS -lBBS -L../libsystem -lsystem
