#BBSHOME = /home/bbstest
CGIPATH = /home/www/cgi-bin/bbs
#HTMPATH = /home/httpd/html/bbs

#ifeq (Makefile.spec ,$(wildcard Makefile.spec))
#include	../Makefile.spec
#endif

#CC 	= gcc
#CFLAGS = -O3 -g -I..
#LDFLAGS = -s -L. -lbbslib
#LDFLAGS = -L. -lbbslib
DEFINES = $(OS_DEF) $(SITE_DEF) -I../include 

#BBSLIBOBJS = crypt.o boardrc.o bbslib.o

CGI	= bbsusr bbstop10 bbssec bbsdoc bbscon bbsbrdadd bbsboa bbsall bbsanc bbs0an \
	  bbslogout bbsleft bbslogin bbsqry bbsnot bbsfind \
	  bbsfadd bbsfdel bbsfall bbsfriend bbsfoot bbsform bbspwd bbsplan bbsinfo \
	  bbsmybrd bbssig bbspst bbsgcon bbsgdoc bbsdel bbsdelmail bbsmailcon bbsmail \
	  bbsdelmsg bbssnd bbsalluser bbsnotepad bbsmsg bbssendmsg bbsreg \
	  bbsmailmsg bbssndmail bbsnewmail bbspstmail bbsgetmsg bbschat bbssel bbscloak \
	  bbsmdoc bbsnick bbsjs bbstfind bbsadl bbstcon bbstdoc \
	  bbsdoreg bbsmywww bbsccc bbsufind bbsclear bbsstat bbsedit bbsman bbsparm \
	  bbsfwd bbsmnote bbsdenyall bbsdenydel bbsdenyadd \
	  bbstopb10 bbsbfind bbstty\

#smth happy: bbslib $(CGI)
#smth := BBSLIBOBJS = crypt.o boardrc.o bbslib.o $(BBSSITE).o smthlib.o md5.o
#happy := BBSLIOBJS = crypt.o boardrc.o bbslib.o
#BBSLIBOBJS = crypt.o boardrc.o bbslib.o $(BBSSITE).o smthlib.o md5.o utmp.o

#DIRS = lib src

.c.o:   ;   $(CC) -DBBSHOME='"$(BBSHOME)"' -DBBSSITE_H='"$(BBSSITE).h"' $(CFLAGS) $(DEFINES) -c $*.c

#MAKEFLAG = $(CFLAGS) $(DEFINES)

all: $(CGI)

$(CGI): % : %.o
	$(CC) $^ -o $* $(LDFLAGS) -DBBSHOME='"$(BBSHOME)"' -DBBSSITE_H='"$(BBSSITE).h"' $(CFLAGS) $(DEFINES) 


install:
	rm -rf $(CGIPATH)/bbslogin
	cp -f $(CGI) $(CGIPATH)
#	chmod +s $(CGIPATH)/*
#	cp html/* $(HTMPATH)

clean:
	rm -f *.o core *.core libbbslib.a $(CGI)
