# The macro "LEEWARD_X_RECORD" need defining for *.c under mail2bbs */
ifeq (../Makefile.spec ,$(wildcard ../Makefile.spec))
include	../Makefile.spec
endif
CC=gcc
MAIL2BBS=mail2bbs
BBS_SRC=/home/system/bbs/src/zixia_bbs
#RECORD_OBJ=$(BBS_SRC)/record.o
RECORD_SRC=$(BBS_SRC)/record.c
CFLAGS=-I$(BBS_SRC) -DBBSHOME='"$(BBSHOME)"' -DBBSSITE_H='"$(BBSSITE).h"' -DINVISIBLE

INSTALLDIR=/home/system/bbs/bin
SITE_DEF = 

all:
	@echo use 'make mail2bbs' to make mail2bbs
	@echo use 'make install_mail2bbs' to install the prog

$(MAIL2BBS):        mail2bbs.c $(RECORD_OBJ)
	$(CC) $(CFLAGS) -Xlinker -Bstatic -o $(MAIL2BBS) mail2bbs.c $(RECORD_SRC) -DAIX -DSYSV -DLEEWARD_X_RECORD

install_mail2bbs: $(MAIL2BBS)
	@echo Please run this option as root.
	test -d $(INSTALLDIR) || mkdir -p $(INSTALLDIR)
	install -g bbs -m 4750 -o bbs $(MAIL2BBS) $(INSTALLDIR)
	test -d $(BBSHOME)/$(INSTALLDIR) || mkdir -p $(BBSHOME)/$(INSTALLDIR)
	install -g bbs -m 4750 -o bbs $(MAIL2BBS) $(BBSHOME)/$(INSTALLDIR)


showuser: showuser.c
	$(CC) -o showuser showuser.c 

resize: resize.c
	$(CC) -o resize resize.c

bfinger: bfinger.c modetype.c
	$(CC) -o bfinger bfinger.c modetype.c
