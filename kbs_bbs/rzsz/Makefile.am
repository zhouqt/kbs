lib_LTLIBRARIES = libzmodem.la
libzmodem_la_SOURCES = lsz.c zreadline.c crctab.c rbsb.c zm.c canit.c 
noinst_HEADERS = zglobal.h zmodem.h
AM_CFLAGS = -DNFGVMIN -I$(top_builddir)/src
CLEANFILES=@EXTRA_CLEAN@
all-local:
	../mkinstalldirs $(top_builddir)/.libs
	cd $(top_builddir)/.libs ; ln -s -f ../rzsz/.libs/* .
