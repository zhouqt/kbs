# Modified by Leeward 1997.11 1998.04-05
# The home directory for the user 'bbs'

# The uid for the user 'bbs'


# The macro "LEEWARD_X_RECORD" need defining for *.c under local_utl */
#CC       = cc -DAIX -DSYSV -DLEEWARD_X_RECORD
#CC       = cc -g -DLINUX -DSYSV -DLEEWARD_X_RECORD -I../include -I..
#CPP       = gcc -g -DLINUX -DSYSV -DLEEWARD_X_RECORD -I../include -I..
PTHREAD = -lpthread
LIBBSD = -lbsd

all:allthing

ifeq (../Makefile.spec ,$(wildcard ../Makefile.spec))
include	../Makefile.spec
endif

CPP=$(CC)

SITE_DEF = -DBBSHOME='"$(BBSHOME)"' -DBBSSITE_H='"$(BBSSITE).h"' -DINVISIBLE
DEFINES	=  $(CSIE_DEF) $(OS_DEF)
CFLAGS += -I.. -DBBSHOME='"$(BBSHOME)"' -DBBSSITE_H='"$(BBSSITE).h"' 

BINFILE= poststat static usage Xclean Xhtml Bhtml refriend chlevel listonline showBM idx0Ann \
	lazybm fixdir

.SUFFIXES: .o .c .ln

.c.o:   ;   $(CC) $(CFLAGS) $(SITE_DEF) $(DEFINES) -DSAFE_KILL -c $*.c
.cpp.o:   ;   $(CC) $(CFLAGS) $(SITE_DEF) $(DEFINES) -DSAFE_KILL -c $*.c
.c.ln:  ;   lint -abhi $*.c


allthing:  $(BINFILE) printboards MakeTar

clean:
	rm -rf *.o *~ core $(BINFILE)
# Modification ends here by Leeward 1997.11

$(BINFILE): % : %.o
	$(CC) $(LDFLAGS) -o $* $^ ../libBBS.a ../libsystem.a

install: all
	$(INSTALL) $(BINFILE) printboards MakeTar $(BBSHOME)/bin

PrintBoards.o:PrintBoards.cpp
	$(CPP) $(CFLAGS) $(SITE_DEF) $(DEFINES) -c PrintBoards.cpp

RecordFile.o:RecordFile.cpp
	$(CPP) $(CFLAGS) $(SITE_DEF) $(DEFINES) -c RecordFile.cpp

Mutex.o:Mutex.cpp
	$(CPP) $(CFLAGS) $(SITE_DEF) $(DEFINES) -c Mutex.cpp

MakeTar.o:MakeTar.cpp
	$(CPP) $(CFLAGS) $(SITE_DEF) $(DEFINES) -c $^ 

printboards:PrintBoards.o RecordFile.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBBSD)

MakeTar:MakeTar.o Mutex.o
	$(CPP) $(LDFLAGS) -o MakeTar MakeTar.o Mutex.o ../libBBS.a ../libsystem.a $(PTHREAD)
