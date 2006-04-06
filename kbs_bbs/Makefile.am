EXTRA_DIST = lib/* README INSTALL \
             site/fb2k-v2.c site/fb2k-v2.h site/default.c site/default.h \
             bbshome innbbsd config.icc config.lib doc/* makedist.sh \
             contrib/crontab.SMTH contrib/build-RH9.sh bbs2www.in 
SUBDIRS=libsystem libBBS rzsz src daemon local_utl mail2bbs service @BBSSUBDIRS@
CLEANFILES=@EXTRA_CLEAN@
AM_MAKEFLAGS=-s 
dist-hook:
	rm -rf `find $(distdir) -name CVS`
	rm -rf `find $(distdir) -name config.status`
	rm -rf `find $(distdir) -name .deps`
#	cd $(distdir);find . -path "./sshbbsd/gmp-2.0.2-ssh-2" -prune -o \
#       -name "./sshbbsd/acconfig.h" -prune -o -name "*.[c|h]" \
#	-exec indent -kr -l200 -i4 -bad -bap -cdb -sc -ip0 -nut {} -o {} \;
#	find $(distdir) -name "*.[c|h]" \
#       -exec $(srcdir)/filter.sh {} \;

install-home:
	sh makedist.sh ${prefix}
