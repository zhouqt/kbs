
#smth happy: bbslib $(CGI)
#smth := BBSLIBOBJS = crypt.o boardrc.o bbslib.o $(BBSSITE).o smthlib.o md5.o
#happy := BBSLIOBJS = crypt.o boardrc.o bbslib.o
AR = ar
BBSLIBOBJS = bbslib.o 
BBSLIBRARY = libbbslib.a
ifeq (../../Makefile.spec ,$(wildcard ../../Makefile.spec))
include	../../Makefile.spec
endif
DEFINES = $(OS_DEF) $(SITE_DEF)
.c.o:   ;   $(CC) -DBBSHOME='"$(BBSHOME)"' -DBBSSITE_H='"$(BBSSITE).h"' -I../include -I../.. $(CFLAGS) $(DEFINES) -c $*.c

all: $(BBSLIBRARY)

$(BBSLIBRARY): $(BBSLIBOBJS)
	$(AR) cru libbbslib.a $(BBSLIBOBJS)
	ranlib libbbslib.a

install:
#	rm -rf $(CGIPATH)/bbslogin
#	cp -f $(CGI) $(CGIPATH)
#	chmod +s $(CGIPATH)/*
#	cp html/* $(HTMPATH)

clean:
	rm -f *.o core *.core libbbslib.a
