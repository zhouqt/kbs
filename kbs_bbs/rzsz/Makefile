#
#   ripped from original automake file
#
HEADERS = zglobal.h zmodem.h
LIBZMODEM = libzmodem.a

SRCS = lsz.c zreadline.c crctab.c rbsb.c zm.c canit.c

DEFS = -DNFGVMIN

OBJS =  lsz.o zreadline.o crctab.o rbsb.o zm.o canit.o
CC = gcc
AR = ar
CFLAGS = -g -O2
COMPILE = $(CC) $(DEFS) $(INCLUDES) $(CFLAGS)
CCLD = $(CC)
LINK = $(CCLD) $(CFLAGS) $(LDFLAGS) -o $@
RANLIB = ranlib
all: $(LIBZMODEM)
.SUFFIXES: .c .o 

.c.o:
	$(COMPILE) -c $<


libzmodem.a: $(OBJS)
	-rm -f libzmodem.a
	$(AR) cru libzmodem.a $(OBJS)
	$(RANLIB) libzmodem.a

clean:
	rm -rf *.a *.o

