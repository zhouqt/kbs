EXTRA_DIST = README INSTALL sites/site.c sites/site.h
SUBDIRS=libsystem libBBS rzsz src daemon local_utl mail2bbs @subdirs@
CLEANFILES=@EXTRA_CLEAN@

dist-hook:
	rm -rf `find $(distdir) -name CVS`
	rm -rf `find $(distdir) -name config.status`
