EXTRA_DIST = README INSTALL site/devel.c site/devel.h site/default.c site/default.h bbshome innbbsd config.icc config.lib
SUBDIRS=libsystem libBBS rzsz src daemon local_utl mail2bbs service contrib @subdirs@
CLEANFILES=@EXTRA_CLEAN@

dist-hook:
	rm -rf `find $(distdir) -name CVS`
	rm -rf `find $(distdir) -name config.status`
	cd $(distdir);find . -path "./sshbbsd/gmp-2.0.2-ssh-2" -prune -o \
        -name "./sshbbsd/acconfig.h" -prune -o -name "*.[c|h]" \
	-exec indent -kr -l200 -i4 -bad -bap -cdb -sc -ip0 -nut {} -o {} \;
	find $(distdir) -name "*.[c|h]" \
        -exec $(srcdir)/filter.sh {} \;
