#BBSHOME = /home/bbstest
#CGIPATH = /home/www/cgi-bin/bbs
#HTMPATH = /home/httpd/html/bbs

ifeq (../../Makefile.spec ,$(wildcard ../../Makefile.spec))
include	../../Makefile.spec
endif

#CC 	= gcc
#CFLAGS = -O3 -g -I..
#LDFLAGS = -s -L. -lbbslib
#LDFLAGS = -L. -lbbslib
DEFINES = $(OS_DEF) $(SITE_DEF) -I../include 

#BBSLIBOBJS = crypt.o boardrc.o bbslib.o

CGI	= bbsusr bbstop10 bbssec bbsdoc bbscon bbsbrdadd bbsboa bbsall bbsanc bbs0an \
	  bbslogout bbsleft bbsqry bbsnot bbsfind \
	  bbsfadd bbsfdel bbsfall bbsfriend bbsfoot bbsform bbspwd bbsplan bbsinfo \
	  bbsmybrd bbssig bbspst bbsgcon bbsgdoc bbsdel bbsdelmail bbsmailcon bbsmail \
	  bbsdelmsg bbssnd bbsalluser bbsnotepad bbsmsg bbssendmsg bbsreg \
	  bbsmailmsg bbssndmail bbsnewmail bbspstmail bbsgetmsg bbschat bbssel bbscloak \
	  bbsmdoc bbsnick bbsjs bbstfind bbstcon bbstdoc \
	  bbsdoreg bbsmywww bbsccc bbsufind bbsclear bbsedit bbsman bbsparm \
	  bbsfwd bbsmnote bbsdenyall bbsdenydel bbsdenyadd \
	  bbstopb10 bbsbfind
#bbslogin 
#smth happy: bbslib $(CGI)
#smth := BBSLIBOBJS = crypt.o boardrc.o bbslib.o $(BBSSITE).o smthlib.o md5.o
#happy := BBSLIOBJS = crypt.o boardrc.o bbslib.o
#BBSLIBOBJS = crypt.o boardrc.o bbslib.o $(BBSSITE).o smthlib.o md5.o utmp.o

#DIRS = lib src

.c.o:   ;   $(CC) -DBBSHOME='"$(BBSHOME)"' -DBBSSITE_H='"$(BBSSITE).h"' $(CFLAGS) $(DEFINES) -c $*.c

#MAKEFLAG = $(CFLAGS) $(DEFINES)

all: $(CGI)

$(CGI): % : %.o
	$(LD) $^ -o $* -lbbslib $(LDFLAGS) $(EXECLDFLAGS) -DBBSHOME='"$(BBSHOME)"' -DBBSSITE_H='"$(BBSSITE).h"' $(DEFINES) 

install: $(patsubst %,$(CGIPATH)/%,$(CGI))
	$(INSTALL) -d $(CGIPATH)
#	chmod +s $(CGIPATH)/*
#	cp html/* $(HTMPATH)

$(patsubst %,$(CGIPATH)/%,$(CGI)): $(CGIPATH)/%: %
	$(INSTALL) -m04771 $^ $(CGIPATH)

clean:
	rm -f *.o core *.core libbbslib.a $(CGI)
