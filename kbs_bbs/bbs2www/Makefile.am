EXTRA_DIST = README INSTALL include/bbslib.h include/types.h \
             @HTML_FILES@
SUBDIRS = lib phplib src
CLEANFILES=@EXTRA_CLEAN@

install-data-local:
	cp -r html/* @HTMPATH@
	cp -r sites/@BBSSITE@/* @HTMLPATH@
	find @HTMPATH@ -name CVS -exec rm -rf {} \;
