#BBSHOME = /home/bbstest
CGIPATH = /home/www/cgi-bin
#HTMPATH = /home/httpd/html/bbs

#ifeq (Makefile.spec ,$(wildcard Makefile.spec))
#include	../Makefile.spec
#endif

#CC 	= gcc
#CFLAGS = -O3 -g -I..
#LDFLAGS = -s -L. -lbbslib
#LDFLAGS = -L. -lbbslib
DEFINES = $(OS_DEF) $(SITE_DEF)

#BBSLIBOBJS = crypt.o boardrc.o bbslib.o

CGI	= bbsusr bbstop10 bbssec bbsdoc bbscon bbsbrdadd bbsboa bbsall bbsanc bbs0an \
	  bbslogout bbsleft bbslogin bbsqry bbsnot bbsfind \
	  bbsfadd bbsfdel bbsfall bbsfriend bbsfoot bbsform bbspwd bbsplan bbsinfo \
	  bbsmybrd bbssig bbspst bbsgcon bbsgdoc bbsdel bbsdelmail bbsmailcon bbsmail \
	  bbsdelmsg bbssnd bbsalluser bbsnotepad bbsmsg bbssendmsg bbsreg \
	  bbsmailmsg bbssndmail bbsnewmail bbspstmail bbsgetmsg bbschat bbssel bbscloak \
	  bbsmdoc bbsnick chatlogin bbsjs bbstfind bbsadl bbstcon bbstdoc \
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

bbs0an: bbs0an.o
	$(CC) -o $@ bbs0an.o $(LDFLAGS)

bbsadl: bbsadl.o
	$(CC) -o $@ bbsadl.o $(LDFLAGS)

bbsall: bbsall.o
	$(CC) -o $@ bbsall.o $(LDFLAGS)

bbsalluser: bbsalluser.o
	$(CC) -o $@ bbsalluser.o $(LDFLAGS)

bbsanc: bbsanc.o
	$(CC) -o $@ bbsanc.o $(LDFLAGS)

bbsbfind: bbsbfind.o
	$(CC) -o $@ bbsbfind.o $(LDFLAGS)

bbsboa: bbsboa.o
	$(CC) -o $@ bbsboa.o $(LDFLAGS)

bbsbrdadd: bbsbrdadd.o
	$(CC) -o $@ bbsbrdadd.o $(LDFLAGS)

bbsccc: bbsccc.o
	$(CC) -o $@ bbsccc.o $(LDFLAGS)

bbschat: bbschat.o
	$(CC) -o $@ bbschat.o $(LDFLAGS)

bbsclear: bbsclear.o
	$(CC) -o $@ bbsclear.o $(LDFLAGS)

bbscloak: bbscloak.o
	$(CC) -o $@ bbscloak.o $(LDFLAGS)

bbscon: bbscon.o
	$(CC) -o $@ bbscon.o $(LDFLAGS)

bbsdel: bbsdel.o
	$(CC) -o $@ bbsdel.o $(LDFLAGS)

bbsdelmail: bbsdelmail.o
	$(CC) -o $@ bbsdelmail.o $(LDFLAGS)

bbsdelmsg: bbsdelmsg.o
	$(CC) -o $@ bbsdelmsg.o $(LDFLAGS)

bbsdenyadd: bbsdenyadd.o
	$(CC) -o $@ bbsdenyadd.o $(LDFLAGS)

bbsdenyall: bbsdenyall.o
	$(CC) -o $@ bbsdenyall.o $(LDFLAGS)

bbsdenydel: bbsdenydel.o
	$(CC) -o $@ bbsdenydel.o $(LDFLAGS)

bbsdoc: bbsdoc.o
	$(CC) -o $@ bbsdoc.o $(LDFLAGS)

bbsdoreg: bbsdoreg.o
	$(CC) -o $@ bbsdoreg.o $(LDFLAGS)

bbsedit: bbsedit.o
	$(CC) -o $@ bbsedit.o $(LDFLAGS)

bbsfadd: bbsfadd.o
	$(CC) -o $@ bbsfadd.o $(LDFLAGS)

bbsfall: bbsfall.o
	$(CC) -o $@ bbsfall.o $(LDFLAGS)

bbsfdel: bbsfdel.o
	$(CC) -o $@ bbsfdel.o $(LDFLAGS)

bbsfind: bbsfind.o
	$(CC) -o $@ bbsfind.o $(LDFLAGS)

bbsfoot: bbsfoot.o
	$(CC) -o $@ bbsfoot.o $(LDFLAGS)

bbsform: bbsform.o
	$(CC) -o $@ bbsform.o $(LDFLAGS)

bbsfriend: bbsfriend.o
	$(CC) -o $@ bbsfriend.o $(LDFLAGS)

bbsfwd: bbsfwd.o
	$(CC) -o $@ bbsfwd.o $(LDFLAGS)

bbsgcon: bbsgcon.o
	$(CC) -o $@ bbsgcon.o $(LDFLAGS)

bbsgdoc: bbsgdoc.o
	$(CC) -o $@ bbsgdoc.o $(LDFLAGS)

bbsgetmsg: bbsgetmsg.o
	$(CC) -o $@ bbsgetmsg.o $(LDFLAGS)

bbsinfo: bbsinfo.o
	$(CC) -o $@ bbsinfo.o $(LDFLAGS)

bbsjs: bbsjs.o
	$(CC) -o $@ bbsjs.o $(LDFLAGS)

bbsleft: bbsleft.o
	$(CC) -o $@ bbsleft.o $(LDFLAGS)

bbslogin: bbslogin.o
	$(CC) -o $@ bbslogin.o $(LDFLAGS)

bbslogout: bbslogout.o
	$(CC) -o $@ bbslogout.o $(LDFLAGS)

bbsmail: bbsmail.o
	$(CC) -o $@ bbsmail.o $(LDFLAGS)

bbsmailcon: bbsmailcon.o
	$(CC) -o $@ bbsmailcon.o $(LDFLAGS)

bbsmailmsg: bbsmailmsg.o
	$(CC) -o $@ bbsmailmsg.o $(LDFLAGS)

bbsman: bbsman.o
	$(CC) -o $@ bbsman.o $(LDFLAGS)

bbsmdoc: bbsmdoc.o
	$(CC) -o $@ bbsmdoc.o $(LDFLAGS)

bbsmnote: bbsmnote.o
	$(CC) -o $@ bbsmnote.o $(LDFLAGS)

bbsmsg: bbsmsg.o
	$(CC) -o $@ bbsmsg.o $(LDFLAGS)

bbsmybrd: bbsmybrd.o
	$(CC) -o $@ bbsmybrd.o $(LDFLAGS)

bbsmywww: bbsmywww.o
	$(CC) -o $@ bbsmywww.o $(LDFLAGS)

bbsnewmail: bbsnewmail.o
	$(CC) -o $@ bbsnewmail.o $(LDFLAGS)

bbsnick: bbsnick.o
	$(CC) -o $@ bbsnick.o $(LDFLAGS)

bbsnot: bbsnot.o
	$(CC) -o $@ bbsnot.o $(LDFLAGS)

bbsnotepad: bbsnotepad.o
	$(CC) -o $@ bbsnotepad.o $(LDFLAGS)

bbsparm: bbsparm.o
	$(CC) -o $@ bbsparm.o $(LDFLAGS)

bbsplan: bbsplan.o
	$(CC) -o $@ bbsplan.o $(LDFLAGS)

bbspst: bbspst.o
	$(CC) -o $@ bbspst.o $(LDFLAGS)

bbspstmail: bbspstmail.o
	$(CC) -o $@ bbspstmail.o $(LDFLAGS)

bbspwd: bbspwd.o
	$(CC) -o $@ bbspwd.o $(LDFLAGS)

bbsqry: bbsqry.o
	$(CC) -o $@ bbsqry.o $(LDFLAGS)

bbsreg: bbsreg.o
	$(CC) -o $@ bbsreg.o $(LDFLAGS)

bbssec: bbssec.o
	$(CC) -o $@ bbssec.o $(LDFLAGS)

bbssel: bbssel.o
	$(CC) -o $@ bbssel.o $(LDFLAGS)

bbssendmsg: bbssendmsg.o
	$(CC) -o $@ bbssendmsg.o $(LDFLAGS)

bbssig: bbssig.o
	$(CC) -o $@ bbssig.o $(LDFLAGS)

bbssnd: bbssnd.o
	$(CC) -o $@ bbssnd.o $(LDFLAGS)

bbssndmail: bbssndmail.o
	$(CC) -o $@ bbssndmail.o $(LDFLAGS)

bbsstat: bbsstat.o
	$(CC) -o $@ bbsstat.o $(LDFLAGS)

bbstcon: bbstcon.o
	$(CC) -o $@ bbstcon.o $(LDFLAGS)

bbstdoc: bbstdoc.o
	$(CC) -o $@ bbstdoc.o $(LDFLAGS)

bbstfind: bbstfind.o
	$(CC) -o $@ bbstfind.o $(LDFLAGS)

bbstop10: bbstop10.o
	$(CC) -o $@ bbstop10.o $(LDFLAGS)

bbstopb10: bbstopb10.o
	$(CC) -o $@ bbstopb10.o $(LDFLAGS)

bbstty: bbstty.o
	$(CC) -o $@ bbstty.o $(LDFLAGS)

bbsudir: bbsudir.o
	$(CC) -o $@ bbsudir.o $(LDFLAGS)

bbsufind: bbsufind.o
	$(CC) -o $@ bbsufind.o $(LDFLAGS)

bbsusr: bbsusr.o
	$(CC) -o $@ bbsusr.o $(LDFLAGS)

chatlogin: chatlogin.o
	$(CC) -o $@ chatlogin.o $(LDFLAGS)

install:
	rm -rf $(CGIPATH)/bbslogin
	cp -f $(CGI) $(CGIPATH)
#	chmod +s $(CGIPATH)/*
#	cp html/* $(HTMPATH)

clean:
	rm -f *.o core *.core libbbslib.a $(CGI)
