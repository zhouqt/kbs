EXTRA_DIST = README INSTALL include/bbslib.h include/types.h \
             @HTML_FILES@
SUBDIRS = lib phplib
CLEANFILES=@EXTRA_CLEAN@
AM_MAKEFLAGS=@MAKEFLAG@

install-data-local:
	cp -r html/* @HTMPATH@
	@cp -r sites/@BBSSITE@/* @HTMPATH@ 2>/dev/null || echo You can copy your site-special files in sites/@BBSSITE@
	find @HTMPATH@ -name CVS -print | xargs rm -rf
