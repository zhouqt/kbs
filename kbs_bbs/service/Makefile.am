SUBDIRS=pip
noinst_HEADERS = tcplib.h
lib_LTLIBRARIES = libfriendtest.la libbbsnet.la
libfriendtest_la_SOURCES = friendtest.c
libbbsnet_la_SOURCES = bbsnet.c tcplib.cpp
libdir=${prefix}/service
AM_CFLAGS = -DINVISIBLE -DSAFE_KILL -D_DETAIL_UINFO_ -DSHOW_IDLE_TIME -I$(top_builddir)/src
CLEANFILES=@EXTRA_CLEAN@
