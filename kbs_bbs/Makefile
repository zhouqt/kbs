# Generated automatically from Makefile.in by configure.

#################################################################
# Makefile for Firebird BBS 2.0                                 #
#################################################################
# 1. This package is derived from PhoenixBBS 4.0 and is developed at
#    Department of Computer Engineering and Information Science, 
#    National Chung-Chang University, Chia-Yi, Taiwan
#
# 2. The maintainer of the package can be reached by
#    chj83u@cs.ccu.edu.tw or SmallPig.bbs@bbs.cs.ccu.edu.tw
#
# 3. This Makefile is for "IBM_AIX"

#############################################################
####   Setup the following site-specific information     ####
#############################################################

# The home directory for the user 'bbs'
#BBSHOME=/home/system/bbs
# The uid for the user 'bbs'
#BBSUID=9999

OS_DEF   = -DAIX -DSYSV 
#OS_DEF   = -DLINUX -DSYSV 
#OS_DEF	= -DFREEBSD


CC       = gcc
#CC       = cc
CFLAGS   = -W -g -Ilibesmtp -static 
#CFLAGS   = -g -O3 -qstrict -qcompact -qarch=ppc
#LIBS     = -L. -L/usr/local/lib -lesmtp
	#-lcrypt -lltdl -ldl -lbsd
LIBS     = -lbsd -lesmtp -Llibesmtp/.libs -lltdl -ldl

#CFLAGS   = -O3
#LIBS     = -ltermcap -lbsd -lcrypt

#INSTALL  = /usr/bin/install -c
INSTALL  = ./install.sh 

BBSLIB = libBBS.a

#CSIE_DEF = -DSHOW_IDLE_TIME -D_DETAIL_UINFO_ 
CSIE_DEF = -DSHOW_IDLE_TIME -DBBSGID=99 -D_DETAIL_UINFO_
#-DTALK_LOG

PROGNAME = bbsd_single

all: $(PROGNAME) chatd $(BBSLIB) newpop3d libsystem.a

ifeq (Makefile.spec ,$(wildcard Makefile.spec))
include	Makefile.spec
endif

LIBS  += $(LDFLAGS)
#############################################################
###        You needn't modify the following stuff      ######
#############################################################
CFILE    = bbs.c record.c delete.c stuff.c mail.c register.c \
           xyz.c talk.c read.c comm_lists.c namecomplete.c chat.c \
           maintain.c edit.c more.c pass.c help.c bcache.c boards.c \
           screen.c userinfo.c vote.c announce.c sendmsg.c boards_t.c\
	   bm.c list.c goodbye.c fileshm.c cache.c bbsd_single.c lvars.c\
	   newmain_single.c newio.c newterm.c convcode.c bbs_sendmail.c crypt.c \
	   ucache.c utmp.c log.c md5.c libmsg.c article.c sysconf.c ${BBSSITE}.c

COBJS    = bbs.o record.o delete.o stuff.o mail.o register.o \
           xyz.o talk.o read.o comm_lists.o namecomplete.o chat.o \
           maintain.o edit.o more.o pass.o help.o bcache.o boards.o \
           screen.o userinfo.o vote.o announce.o sendmsg.o boards_t.o\
           bm.o list.o goodbye.o fileshm.o cache.o bbsd_single.o lvars.o\
	   newmain_single.o newio.o newterm.o convcode.o bbs_sendmail.o crypt.o \
	   ucache.o utmp.o log.o md5.o libmsg.o article.o sysconf.o $(BBSSITE).o

BBSLIBOBJS = cache.o record.o log.o ucache.o stuff.o md5.o crypt.o pass.o bcache.o utmp.o \
	lvars.o $(BBSSITE).o boards.o lvars.o bbs_sendmail.o libmsg.o article.o convcode.o \
	sysconf.o

OTHERS   = bbs.h edit.h

GARBAGE  = a.out core installchatd adminbbs chatd

# The -DINVISIBLE makes bbs users invisible to 'finger' and 'who'.
# Simply remove it if you don't want this feature.

SITE_DEF = -DBBSHOME='"$(BBSHOME)"' -DBBSSITE_H='"$(BBSSITE).h"' -DINVISIBLE

DEFINES	=  $(CSIE_DEF) $(OS_DEF)

.SUFFIXES: .o .c .ln

.c.o:   ;   $(CC) $(CFLAGS) $(SITE_DEF) $(DEFINES) -DSAFE_KILL -DBBSMAIN -c $*.c
.c.ln:  ;   lint -abhi $*.c

#--------------------- Dependency starts here -------------------
dep:
	rm -f .depend
	echo $(CFILE) | xargs $(CC) $(CFLAGS) $(SITE_DEF) $(DEFINES) -DSAFE_KILL  -M -DBBSMAIN >>.depend


ifeq (.depend,$(wildcard .depend))
include .depend
endif

libsystem.a:
	cd lib; make
	
$(PROGNAME): $(COBJS) $(OTHERS) libsystem.a
	$(CC) -o bbsd_single $(CFLAGS) $(COBJS) $(LIBS) libsystem.a -g

lint: $(LNFILES)
	lint -abh $(LNFILES)

newpop3.o: newpop3.c
	$(CC) -g $(SITE_DEF) $(DEFINES)  -c newpop3.c
	
newpop3d: newpop3.o $(BBSLIB) libsystem.a
	$(CC) $(CFLAGS) -o newpop3d $(SITE_DEF) $(DEFINES) newpop3.o $(BBSLIB) $(LIBS) libsystem.a

thread: thread.c record.c
	$(CC) $(CFLAGS) -o thread $(OS_DEF) record.c thread.c $(LIBS)

#$(patsubst %.o,%_use.o,$(BBSLIBOBJS)):$(patsubst %_use.o,%.c,$@)
#%_use.o: %.c
#	$(CC) $(CFLAGS) $(SITE_DEF) $(DEFINES) -DSAFE_KILL -c $^ -o $@
lib%.o: %.c
	$(CC) $(CFLAGS) $(SITE_DEF) $(DEFINES) -DSAFE_KILL -UBBSMAIN -c $^ -o $@
	
$(BBSLIB) : $(patsubst %.o,lib%.o,$(BBSLIBOBJS))
	ar cru $@ $^
	ranlib $@
	
chatd: station.c libBBS.a libsystem.a
	$(CC) -g $(CFLAGS) $(SITE_DEF) $(DEFINES) -o chatd $(OS_DEF) station.c $(LIBS) libBBS.a libsystem.a


ve: edit.o term.o screen.o io.o editor.c
	$(CC) -o ve -g edit.o term.o screen.o io.o editor.c

install: $(PROGNAME) $(BBSHOME)/bin/bbsd_single
	$(INSTALL) newpop3d $(BBSHOME)/bin
	$(INSTALL) chatd $(BBSHOME)/bin

$(BBSHOME)/bin/bbsd_single: bbsd_single
	$(INSTALL) $@ $@.old
	$(INSTALL) bbsd_single $@

installthread: thread $(BBSHOME)/bin/thread
$(BBSHOME)/bin/thread: thread
	$(INSTALL) $@ $@.old
	$(INSTALL) thread $@

installchatd: chatd $(BBSHOME)/bin/chatd
$(BBSHOME)/bin/chatd: chatd
	$(INSTALL) $@ $@.old
	$(INSTALL) chatd $@
                                
clean: /tmp
	-rm -fr $(GARBAGE) $(COBJS) $(PROGNAME) $(LNFILES) chatd
	-rm -rf *.o
	-rm -rf *.a

dist_clean: clean
	-rm -f CONFIG/config.log
	-rm -f CONFIG/config.status
	-rm -f CONFIG/config.cache
	-rm -f Install.sh

tags: /tmp
	ctags $(CFILE)

