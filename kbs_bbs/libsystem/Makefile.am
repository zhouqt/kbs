lib_LTLIBRARIES = libsystem.la
libsystem_la_SOURCES = f_cat.c f_cp.c f_rm.c f_mv.c f_touch.c f_ln.c f_lock.c bm_strcmp.c uuencode.c memmem.c flock.c
AM_CFLAGS=-I$(top_builddir)/src
CLEANFILES=@EXTRA_CLEAN@
#BBSLIB=`cd $(top_builddir)/.libs; pwd`
#all-local:
#	../mkinstalldirs $(top_builddir)/.libs
#	@list='$(lib_LTLIBRARIES)'; for p in $$list; do \
#	if test -f $$p; then \
#		f="`echo $$p | sed -e 's|^.*/||'`"; \
#		echo " $(LIBTOOL) --mode=install $(libLTLIBRARIES_INSTALL) $(INSTALL_STRIP_FLAG) $$p $(BBSLIB)/$$f"; \
#		$(LIBTOOL) --mode=install $(libLTLIBRARIES_INSTALL) $(INSTALL_STRIP_FLAG) $$p $(BBSLIB)/$$f; \
#		else :; fi; \
#	done
