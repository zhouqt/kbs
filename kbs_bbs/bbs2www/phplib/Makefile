CC      = gcc
LD      = ld
INSTALL	= install
CFLAGS  = -O3 -DCOMPILE_DL=1 -tpp6
TARGET  = phpbbslib.so
SRC     = phpbbslib.c
OBJ     = phpbbslib.o
PHPSUFFIX = php

INC     = -I../include -I$(PHPBASE) -I$(PHPBASE)/main -I$(PHPBASE)/Zend -I$(PHPBASE)/TSRM -DSMTH
all: $(TARGET)
#smth happy: bbslib $(CGI)
#smth := BBSLIBOBJS = crypt.o boardrc.o bbslib.o $(BBSSITE).o smthlib.o md5.o
#happy := BBSLIOBJS = crypt.o boardrc.o bbslib.o
ifeq (../../Makefile.spec ,$(wildcard ../../Makefile.spec))
include	../../Makefile.spec
endif
DEFINES = $(OS_DEF) $(SITE_DEF)
.c.o:   ;   $(CC) -DCOMPILE_DL=1 -DBBSHOME='"$(BBSHOME)"' -DBBSSITE_H='"$(BBSSITE).h"' -I../include -I../.. $(CFLAGS) $(DEFINES) $(INC) -c $*.c

# complier and linker
PHPBASE = /home/share/php-4.2.0

$(TARGET): $(OBJ) ../../libBBS.a ../../libsystem.a
	$(LD) -shared $(LDFLAGS) -o $(TARGET) $(OBJ) -lBBS ../../libsystem.a ../lib/libbbslib.a -lesmtp

install: $(TARGET)
	@mkdir -p $(PHPBBSLIBDIR)
	$(INSTALL) $(TARGET) $(PHPBBSLIBDIR)
	$(INSTALL) funcs.php $(PHPBBSLIBDIR)/funcs.$(PHPSUFFIX)

clean:
	rm -f *.o *.so core
