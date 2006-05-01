lib_LTLIBRARIES = libBBS.la
#noinst_LIBRARIES = libBBS.a
libBBS_la_SOURCES = record.c log.c ucache.c stuff.c md5.c crypt.c pass.c bcache.c utmp.c \
	lvars.c $(top_builddir)/libBBS/site.c boards.c bbs_sendmail.c libmsg.c article.c convcode.c \
	sysconf.c libann.c filter.c mgrep.c $(top_builddir)/libBBS/default.c output.c libtmpl.c libpc.c \
	xml.c super_filter.c
#libBBS_la_SOURCES = $(libBBS_a_SOURCES)
AM_CFLAGS = -I$(top_srcdir)/src
CLEANFILES=@EXTRA_CLEAN@
#BBSLIB=`cd $(top_builddir)/.libs; pwd`
#all-local:
#	../mkinstalldirs $(top_builddir)/.libs
#	@list='$(lib_LTLIBRARIES)'; for p in $$list; do \
#      if test -f $$p; then \
#        f="`echo $$p | sed -e 's|^.*/||'`"; \
#		echo " $(LIBTOOL) --mode=install $(libLTLIBRARIES_INSTALL) $(INSTALL_STRIP_FLAG) $$p $(BBSLIB)/$$f"; \
#        $(LIBTOOL) --mode=install $(libLTLIBRARIES_INSTALL) $(INSTALL_STRIP_FLAG) $$p $(BBSLIB)/$$f; \
#      else :; fi; \
#    done
