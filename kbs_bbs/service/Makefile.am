EXTRA_DIST = definekey.* ansihelp.txt
SUBDIRS=pip worker personaldns
noinst_HEADERS = tcplib.h
lib_LTLIBRARIES = libfriendtest.la libbbsnet.la libpandora.la libdict.la libquiz.la libkiller.la libcalendar.la libcalc.la libansieditor.la libiquery.la
libfriendtest_la_SOURCES = friendtest.c
libbbsnet_la_SOURCES = bbsnet.c tcplib.c
libpandora_la_SOURCES = pandora.c tcplib.c
libdict_la_SOURCES = dict.c
libquiz_la_SOURCES = quiz.c
libkiller_la_SOURCES = killer.c
libcalendar_la_SOURCES = calendar.c
libcalc_la_SOURCES = calc.c
libansieditor_la_SOURCES = ansieditor.c
libiquery_la_SOURCES = iquery.c
libdir=${prefix}/service
AM_CFLAGS = -DINVISIBLE -DSAFE_KILL -D_DETAIL_UINFO_ -DSHOW_IDLE_TIME -I$(top_builddir)/src
CLEANFILES=@EXTRA_CLEAN@
install-data-local:
	cp -f definekey.* ansihelp.txt $(libdir)
