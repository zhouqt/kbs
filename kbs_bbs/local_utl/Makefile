# Modified by Leeward 1997.11 1998.04-05

# The macro "LEEWARD_X_RECORD" need defining for *.c under local_utl */
#CC       = cc -DAIX -DSYSV -DLEEWARD_X_RECORD
CC       = cc -g -DLINUX -DSYSV -DLEEWARD_X_RECORD -I../include -I..
CPP       = gcc -g -DLINUX -DSYSV -DLEEWARD_X_RECORD -I../include -I..
include Makefile.spec

BINFILE= poststat static usage bfinger Xclean Xhtml Bhtml bfinger refriend chlevel listonline syncann printboards MakeTar 

.SUFFIXES: .o .c .cpp .ln

.c.o:   ;   $(CC) $(CFLAGS) $(SITE_DEF) $(DEFINES) -c $*.c
.c.ln:  ;   lint -abhi $*.c


all:  $(BINFILE)

Xclean:
	$(CC) -o Xclean clean.c

Xhtml: Xhtml.c
	$(CC) -o Xhtml Xhtml.c

Bhtml:
	$(CC) -o Bhtml Bhtml.c

poststat: poststat.c
	$(CC) -o poststat poststat.c

static: static.c
	$(CC) -o static static.c
usage: usage.c record.c 
	$(CC) -o usage usage.c record.c 

# Leeward: Below "chmod 6755"  is needed for WWW CGI
bfinger: bfinger.c modetype.c
	$(CC) -o bfinger bfinger.c modetype.c
	cp -f bfinger ../../bin
	chmod 6755 ../../bin/bfinger

clean:
	rm -rf *.o *~ core $(BINFILE)
# Modification ends here by Leeward 1997.11

refriend: refriend.c record.c
	$(CC) -o refriend refriend.c record.c
chlevel: chlevel.c record.c
	$(CC) -o chlevel chlevel.c record.c

install: all
	cp -f $(BINFILE) ../../bin

listonline:listonline.o ../libBBS.a
	gcc -o listonline listonline.o ../libBBS.a

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

printboards:PrintBoards.o RecordFile.o
	$(CPP) -o printboards PrintBoards.o RecordFile.o

MakeTar.o:MakeTar.cpp
	$(CPP) $(CFLAGS) $(SITE_DEF) $(DEFINES) -c MakeTar.cpp

MakeTar:MakeTar.o Mutex.o
	$(CPP) -o MakeTar MakeTar.o Mutex.o -lpthread
