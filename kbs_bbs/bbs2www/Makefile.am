EXTRA_DIST = README INSTALL include/bbslib.h include/types.h \
             @HTML_FILES@
SUBDIRS = lib phplib src
CLEANFILES=@EXTRA_CLEAN@
AM_MAKEFLAGS=-s

install-data-local:
	cp -r html/* @HTMPATH@
	cp -r sites/@BBSSITE@/* @HTMPATH@
	find @HTMPATH@ -name CVS -print | xargs rm -rf
