# The macro "LEEWARD_X_RECORD" need defining for *.c under mail2bbs */
ifeq (../Makefile.spec ,$(wildcard ../Makefile.spec))
include	../Makefile.spec
endif
INSTALL=install
MAIL2BBS=mail2bbs
BBS_SRC=..
#RECORD_OBJ=$(BBS_SRC)/record.o
RECORD_SRC=$(BBS_SRC)/record.c
#CFLAGS=-I$(BBS_SRC) -DBBSHOME='"$(BBSHOME)"' -DBBSSITE_H='"$(BBSSITE).h"' -DINVISIBLE
CFLAGS+=-I$(BBS_SRC)
SITE_DEF = -DBBSHOME='"$(BBSHOME)"' -DBBSSITE_H='"$(BBSSITE).h"' -DINVISIBLE

INSTALLDIR=bin

all:
	@echo use 'make mail2bbs' to make mail2bbs
	@echo use 'make install_mail2bbs' to install the prog

$(MAIL2BBS):        mail2bbs.c 
	$(CC) $(CFLAGS) $(SITE_DEF) $(DEFINES) -Xlinker -Bstatic -o $(MAIL2BBS) mail2bbs.c -DLEEWARD_X_RECORD ../libBBS.a ../libsystem.a

install_mail2bbs: $(MAIL2BBS)
	@echo Please run this option as root.
	test -d $(INSTALLDIR) || mkdir -p $(INSTALLDIR)
	$(INSTALL) -g bbs -m 4750 -o bbs $(MAIL2BBS) $(INSTALLDIR)
	test -d $(BBSHOME)/$(INSTALLDIR) || mkdir -p $(BBSHOME)/$(INSTALLDIR)
	$(INSTALL) -g bbs -m 4750 -o bbs $(MAIL2BBS) $(BBSHOME)/$(INSTALLDIR)


showuser: showuser.c
	$(CC) -o showuser showuser.c 

resize: resize.c
	$(CC) -o resize resize.c

bfinger: bfinger.c modetype.c
	$(CC) -o bfinger bfinger.c modetype.c
