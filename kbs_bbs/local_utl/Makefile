# Modified by Leeward 1997.11 1998.04-05
# The home directory for the user 'bbs'
BBSHOME=/home/bbs

# The uid for the user 'bbs'
BBSUID=9999
#BBSUID=502


# The macro "LEEWARD_X_RECORD" need defining for *.c under local_utl */
#CC       = cc -DAIX -DSYSV -DLEEWARD_X_RECORD
#CC       = cc -g -DLINUX -DSYSV -DLEEWARD_X_RECORD -I../include -I..
#CPP       = gcc -g -DLINUX -DSYSV -DLEEWARD_X_RECORD -I../include -I..
OS_DEF   = -DLINUX -DSYSV 
CSIE_DEF = -DSHOW_IDLE_TIME -DBBSGID=502 -D_DETAIL_UINFO_ 

include ../Makefile.spec

CPP=$(CC)

SITE_DEF = -DBBSHOME='"$(BBSHOME)"' -DBBSUID=$(BBSUID) -DINVISIBLE
DEFINES	=  $(CSIE_DEF) $(OS_DEF)
CFLAGS += -I..

BINFILE= poststat static usage Xclean Xhtml Bhtml refriend chlevel listonline syncann printboards MakeTar 

.SUFFIXES: .o .c .ln

.c.o:   ;   $(CC) $(CFLAGS) $(SITE_DEF) $(DEFINES) -DSAFE_KILL -DBBSMAIN -c $*.c
.c.ln:  ;   lint -abhi $*.c


all:  $(BINFILE)

Xclean: clean.o
	$(CC) -o $@ $^

Xhtml: Xhtml.o
	$(CC) -o $@ $^

Bhtml: Bhtml.o
	$(CC) -o $@ $^

poststat: poststat.o
	$(CC) -o $@ $^

static: static.o
	$(CC) -o $@ $^

usage:  usage.o ../libBBS.a
	$(CC) -o $@ $^

clean:
	rm -rf *.o *~ core $(BINFILE)
# Modification ends here by Leeward 1997.11

refriend: refriend.o ../libBBS.a
	$(CC) -o $@ $^

chlevel: chlevel.o ../libBBS.a
	$(CC) -o $@ $^

install: all
	cp -f $(BINFILE) ../../bin

listonline:listonline.o ../libBBS.a
	$(CC) -o $@ $^

#bcache.o:bcache.c
#	gcc -c bcache.c

syncann:syncann.cpp
	gcc -o syncann syncann.cpp

PrintBoards.o:PrintBoards.cpp
	$(CPP) $(CFLAGS) $(SITE_DEF) $(DEFINES) -c PrintBoards.cpp

RecordFile.o:RecordFile.cpp
	$(CPP) $(CFLAGS) $(SITE_DEF) $(DEFINES) -c RecordFile.cpp

Mutex.o:Mutex.cpp
	$(CPP) $(CFLAGS) $(SITE_DEF) $(DEFINES) -c Mutex.cpp

MakeTar.o:MakeTar.cpp
	$(CPP) $(CFLAGS) $(SITE_DEF) $(DEFINES) -c $^

printboards:PrintBoards.o RecordFile.o
	$(CC) -o $@ $^ -lbsd

MakeTar:MakeTar.o Mutex.o
	$(CPP) -o MakeTar MakeTar.o Mutex.o -lpthread
