lib_LTLIBRARIES = libsystem.la
libsystem_la_SOURCES = f_cat.c f_cp.c f_rm.c f_mv.c f_touch.c f_ln.c
AM_CFLAGS=-I$(top_builddir)/src
CLEANFILES=@EXTRA_CLEAN@
all-local:
	../mkinstalldirs $(top_builddir)/.libs
	cd $(top_builddir)/.libs ; ln -f -s ../libsystem/.libs/* .
