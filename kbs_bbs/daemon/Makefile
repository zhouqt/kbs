# Modified by Leeward 1997.11 1998.04-05
# The home directory for the user 'bbs'

# The macro "LEEWARD_X_RECORD" need defining for *.c under local_utl */
#CC       = cc -DAIX -DSYSV -DLEEWARD_X_RECORD
#CC       = cc -g -DLINUX -DSYSV -DLEEWARD_X_RECORD -I../include -I..
#CPP       = gcc -g -DLINUX -DSYSV -DLEEWARD_X_RECORD -I../include -I..
CC = gcc -g
OS_DEF   = -DLINUX -DSYSV 
#CSIE_DEF = -DSHOW_IDLE_TIME -DBBSGID=502 -D_DETAIL_UINFO_ 
CSIE_DEF = -DSHOW_IDLE_TIME -D_DETAIL_UINFO_ 

ifeq (../Makefile.spec ,$(wildcard ../Makefile.spec))
include ../Makefile.spec
endif

CPP=$(CC)

SITE_DEF = -DBBSHOME='"$(BBSHOME)"' -DBBSSITE_H='"$(BBSSITE).h"' -DINVISIBLE
DEFINES	=  $(CSIE_DEF) $(OS_DEF)
CFLAGS += -I..

BINFILE=utmpd miscd

.SUFFIXES: .o .c .ln

.c.o:   ;   $(CC) $(CFLAGS) $(SITE_DEF) $(DEFINES) -DSAFE_KILL -DBBSMAIN -c $*.c
.c.ln:  ;   lint -abhi $*.c


all:  $(BINFILE)

$(BINFILE): % : %.o ../libBBS.a
	$(CC) $^ -o $*

install: all
	cp -f $(BINFILE) $(BBSHOME)/bin
