EXTRA_DIST = README INSTALL include/bbslib.h include/types.h \
             @HTML_FILES@ xml/*
SUBDIRS = lib phplib
CLEANFILES=@EXTRA_CLEAN@
AM_MAKEFLAGS=@MAKEFLAG@

install-data-local:
#	cp -r html/* @HTMPATH@
#	@cp -r sites/@BBSSITE@/* @HTMPATH@ 2>/dev/null || echo You can copy your site-special files in sites/@BBSSITE@
#	find @HTMPATH@ -name CVS -print | xargs rm -rf
	if test @CONFIG_HAVE_WAP@ = "yes"; then \
	if test '!' -d @WAPPATH@; then mkdir @WAPPATH@; fi; \
	cp wap/*.php @WAPPATH@; \
	fi;

install-data-hook:
	if test @CONFIG_HAVE_WAP@ = "yes"; then \
	ln -s @HTMPATH@/site.php @WAPPATH@/site.php; \
	fi;
