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
LIBS     = -L. -L/usr/local/lib -lesmtp
	#-lcrypt -lpthread -lltdl -ldl -lbsd

#CFLAGS   = -O3
#LIBS     = -ltermcap -lbsd -lcrypt

INSTALL  = /usr/bin/install -c

BBSLIB = libBBS.a

#CSIE_DEF = -DSHOW_IDLE_TIME -D_DETAIL_UINFO_ 
CSIE_DEF = -DSHOW_IDLE_TIME -DBBSGID=99 -D_DETAIL_UINFO_
#-DTALK_LOG
ifeq (Makefile.spec ,$(wildcard Makefile.spec))
include	Makefile.spec
endif

#############################################################
###        You needn't modify the following stuff      ######
#############################################################
PROGNAME = bbsd_single
CFILE    = bbs.c record.c delete.c stuff.c mail.c register.c \
           xyz.c talk.c read.c comm_lists.c namecomplete.c chat.c \
           maintain.c edit.c more.c pass.c help.c bcache.c boards.c \
           screen.c userinfo.c vote.c announce.c sendmsg.c\
	   bm.c list.c goodbye.c fileshm.c cache.c bbsd_single.c \
	   newmain_single.c newio.c newterm.c convcode.c bbs_sendmail.c crypt.c \
	   ucache.c utmp.c log.c md5.c smth.c 

COBJS    = bbs.o record.o delete.o stuff.o mail.o register.o \
           xyz.o talk.o read.o comm_lists.o namecomplete.o chat.o \
           maintain.o edit.o more.o pass.o help.o bcache.o boards.o \
           screen.o userinfo.o vote.o announce.o sendmsg.o\
           bm.o list.o goodbye.o fileshm.o cache.o bbsd_single.o \
	   newmain_single.o newio.o newterm.o convcode.o bbs_sendmail.o crypt.o \
	   ucache.o utmp.o log.o md5.o smth.o 

BBSLIBOBJS = librecord.o liblog.o libucache.o libstuff.o libmd5.o libcrypt.o libpass.o libbcache.o libutmp.o

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
all: $(PROGNAME) chatd $(BBSLIB) newpop3d bbspost

ifeq (.depend,$(wildcard .depend))
include .depend
endif

dep:
	rm -f .depend
	ls *.c | xargs gcc $(CFLAGS) $(SITE_DEF) $(DEFINES) -DSAFE_KILL  -M  >>.depend

$(PROGNAME): $(COBJS) $(OTHERS)
	$(CC) -o bbsd_single $(CFLAGS) $(COBJS) $(LIBS) -g

bbstest: $(BBSLIB) $(OTHERS) newmain_single.o newio.o newterm.o convcode.o
	$(CC) -o bbstest $(CFLAGS) $(BBSLIB) $(LIBS) newmain_single.o newio.o newterm.o convcode.o
	cp -f bbstest ../bin/bbstest

bbspost: bbspost.o $(BBSLIB) smth.o 
	gcc -DDONT_HAVE_TM_GMTOFF $(OS_DEF) -DLEEWARD_X_RECORD -o bbspost bbspost.o smth.o $(BBSLIB) $(LIBS)
	#cp -f bbspost $(BBSHOME)/bin
	#chmod 6755 $(BBSHOME)/bin/bbspost

lint: $(LNFILES)
	lint -abh $(LNFILES)

newpop3.o: newpop3.c
	gcc -g $(SITE_DEF) $(DEFINES)  -c newpop3.c
	
newpop3d: newpop3.o $(BBSLIB) smth.o
	$(CC) $(CFLAGS) -o newpop3d $(SITE_DEF) $(DEFINES) newpop3.o smth.o $(BBSLIB) $(LIBS)
	#cp -f newpop3d $(BBSHOME)/bin
#	$(CC) $(CFLAGS) -o pop3d $(SITE_DEF) -D_BSD pop3d.c

thread: thread.c record.c
	$(CC) $(CFLAGS) -o thread $(OS_DEF) record.c thread.c $(LIBS)

#$(patsubst %.o,%_use.o,$(BBSLIBOBJS)):$(patsubst %_use.o,%.c,$@)
#%_use.o: %.c
#	$(CC) $(CFLAGS) $(SITE_DEF) $(DEFINES) -DSAFE_KILL -c $^ -o $@
lib%.o: %.c
	$(CC) $(CFLAGS) $(SITE_DEF) $(DEFINES) -DSAFE_KILL -UBBSMAIN -c $^ -o $@
	
$(BBSLIB) : $(BBSLIBOBJS)
	ar cru $@ $^
	ranlib $@
	
chatd: station.c 
	$(CC) -g $(CFLAGS) $(SITE_DEF) $(DEFINES) -o chatd $(OS_DEF) station.c $(LIBS)
#	cp -f chatd $(BBSHOME)/bin


ve: edit.o term.o screen.o io.o editor.c
	$(CC) -o ve -g edit.o term.o screen.o io.o editor.c

#install: all
#	./Install.sh

installbbs: $(PROGNAME) $(BBSHOME)/bin/bbsd_single

$(BBSHOME)/bin/bbsd_single: bbsd_single
	$(INSTALL) bbsd_single $@.new
	-rm -f $@.old
	-mv $@ $@.old
	mv $@.new $@

installthread: thread $(BBSHOME)/bin/thread
$(BBSHOME)/bin/thread: thread
	$(INSTALL) -s thread $@.new
	-rm -f $@.old
	-mv $@ $@.old
	mv $@.new $@

installchatd: chatd $(BBSHOME)/bin/chatd
$(BBSHOME)/bin/chatd: chatd
	-rm -f $(BBSHOME)/bin/chatd.old
	-cp -f $(BBSHOME)/bin/chatd $(BBSHOME)/bin/chatd.old
	-cp -f chatd $(BBSHOME)/bin/chatd
                                
clean: /tmp
	-rm -fr $(GARBAGE) $(COBJS) $(PROGNAME) $(LNFILES) chatd
	-rm -rf *.o
	-rm *.a

dist_clean: clean
	-rm -f CONFIG/config.log
	-rm -f CONFIG/config.status
	-rm -f CONFIG/config.cache
	-rm -f Install.sh

tags: /tmp
	ctags $(CFILE)

