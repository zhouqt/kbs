EXTRA_DIST = README INSTALL site/devel.c site/devel.h bbshome innbbsd
SUBDIRS=libsystem libBBS rzsz src daemon local_utl mail2bbs service @subdirs@
CLEANFILES=@EXTRA_CLEAN@

dist-hook:
	rm -rf `find $(distdir) -name CVS`
	rm -rf `find $(distdir) -name config.status`
	find $(distdir) -name "*.[ch]" -exec -path "./sshbbsd/gmp-2.0.2-ssh-2" -prune\
		indent -kr -l200 -i4 -bad -bap -cdb -sc -ip0 -nut {} -o {} \;
