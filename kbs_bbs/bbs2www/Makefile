BBSINCLUDE = ../include
BBSINCLUDE2 = ../../
BBSLIBRARY = ../lib
BBSLIBRARY2 = ../../

all:allthing

#ifeq (Makefile.spec ,$(wildcard Makefile.spec))
include	../Makefile.spec
#endif

MAKE = make
#CC 	= gcc
CFLAGS += -I$(BBSINCLUDE2)
LDFLAGS += -L$(BBSLIBRARY) -L$(BBSLIBRARY2) -lbbslib -lBBS -lsystem -lesmtp -lltdl -ldl
OS_DEF = -DLINUX
SITE_DEF = -DINVISIBLE -DSMTH -DNJU_WWWBBS -DBBS2WWW
#DEFINES = $(OS_DEF) $(SITE_DEF)


DIRS = lib src

#.c.o:   ;   $(CC) $(CFLAGS) $(DEFINES) -c $*.c

MAKEFLAG = MAKE=$(MAKE) CC=$(CC) BBSHOME=$(BBSHOME) BBSSITE=$(BBSSITE) CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" OS_DEF="$(OS_DEF)" SITE_DEF="$(SITE_DEF)"

allthing:
	@echo "== Making main program and relatve src =="
	@for x in $(DIRS); \
	do \
		echo "--> $$x"; \
		(cd $$x; $(MAKE) $(MAKEFLAG) all ; cd .. ); \
		echo "<-- $$x"; \
		echo " "; \
	done
	echo "done."

install: all
	@echo "== Installing Everything =="
	@for x in $(DIRS); \
	do \
		echo "--> $$x"; \
		(cd $$x; make install ; cd .. ); \
		echo "<-- $$x"; \
		echo " "; \
	done
	echo "done."
	#rm -rf $(CGIPATH)/bbslogin
	#cp -f $(CGI) $(CGIPATH)
	chmod +s $(CGIPATH)/*
	cp html/* $(HTMPATH)

clean:
	@echo "== cleanning junks, object files, and binaries .. =="
	@for x in $(DIRS); \
	do \
		echo "--> $$x"; \
		(cd $$x; make clean ; cd .. ); \
		echo "<-- $$x"; \
		echo " "; \
	done
	echo "done."
#	rm -f *.o core *.core libbbslib.a $(CGI)
