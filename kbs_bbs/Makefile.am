EXTRA_DIST = README INSTALL
SUBDIRS=libsystem libBBS rzsz src daemon local_utl mail2bbs @subdirs@
CLEANFILES=@EXTRA_CLEAN@

dist-hook:
	rm -rf `find $(distdir) -name CVS`
	rm -rf `find $(distdir) -name Makefile.am`
	rm -rf `find $(distdir) -name configure.ac`
	rm -rf `find $(distdir) -name configure.in`
