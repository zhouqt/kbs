EXTRA_DIST = README INSTALL
SUBDIRS = lib phplib src
CLEANFILES=@EXTRA_CLEAN@

install-data-local:
	cp -r html/* @HTMPATH@
	rm -rf @HTMPATH@/CVS
