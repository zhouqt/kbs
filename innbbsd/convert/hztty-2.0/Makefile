##
##
## By default, hztty includes all conversion tables, if you want to
## exclude some of the conversions, comment out the corresponding variables.

# between GB & BIG5	(add about 50K)
DEF_GB_BIG5= -DHZTTY_GB_BIG5
OBJ_GB_BIG5= b2g.o b2g_tables.o

# between UNICODE & GB	(add about 150K)
DEF_UNICODE_GB= -DHZTTY_UNICODE_GB
OBJ_UNICODE_GB= g2u.o g2u_tables.o

# between UNICODE & BIG5	(add about 170K)
DEF_UNICODE_BIG5= -DHZTTY_UNICODE_BIG5
OBJ_UNICODE_BIG5= b2u.o b2u_tables.o

# you might change the following

CC=gcc
CPP=$(CC) -E
CDEBUGFLAGS= -O
MAKE=make
RM=rm -f

####################################################################
# you probably don't need to change anything below

MODULES_DEF=	${DEF_GB_BIG5} ${DEF_UNICODE_GB} ${DEF_UNICODE_BIG5}
OBJS =  main.o tty.o io.o log.o hz2gb.o unicode.o hack.o	\
	${OBJ_GB_BIG5} ${OBJ_UNICODE_GB} ${OBJ_UNICODE_BIG5}

DEFINES= 
CFLAGS=${CDEBUGFLAGS} ${DEFINES} ${MODULES_DEF} -I.

PROG = hztty
LIBS=

all: linux

help:
	@echo "Choose one of the make command according to your system type:"
	@echo "	DEC Alpha (OSF/1):		make osf"
	@echo "	DEC (Ultrix):			make ultrix"
	@echo "	HP (UX):			make hpux"
	@echo "	HP/Apollo (domain):		make apollo"
	@echo "	IBM RS6000 (AIX):		make ibm-rs"
	@echo "	Macintosh II (A/UX 2.0):	make mac2"
	@echo "	MIPS (???):			make mips"
	@echo "	Motorola (R32Vx/R3Vx):		make m88k-r3"
	@echo "	Motorola (R40Vx):		make m88k-r4"
	@echo "	PC (Linux):			make linux"
	@echo "	Sequent (BSD):			make sequent-bsd"
	@echo "	Sequent (System V R4):		make sequent-sysv4"
	@echo "	SGI (IRIX 3.x):			make sgi-irix3"
	@echo "	SGI (IRIX 4.x):			make sgi-irix4"
	@echo "	SGI (IRIX 5.x):			make sgi-irix5"
	@echo "	Sun (SunOS 4.x or Solaris 2):	make sun"
	@echo "	Symmetry:			make symmetry"
	@echo ""
	@echo "If none of above is for you, try one of these:"
	@echo "	BSD Unix:			make bsd"
	@echo "	Systerm V Unix:			make sys"


sun:
	miniconf.sh "$(CPP)"
	${MAKE} ${MFLAGS} ${PROG}

sun-os5: svr4

sgi-irix3:
	miniconf.sh $(CPP)
	${MAKE} ${MFLAGS} \
		DEFINES='-DSYSV -DIRIX=3 -D__STDC__ -DUNISTD' \
		${PROG}

sgi-irix4:
	miniconf.sh $(CPP)
	${MAKE} ${MFLAGS} \
		DEFINES='-DSYSV -DIRIX=4 -D__STDC__ -DUNISTD' \
		${PROG}

sgi-irix5:
	miniconf.sh $(CPP)
	${MAKE} ${MFLAGS} DEFINES='-DSVR4 -DIRIX=5' ${PROG}

osf:	posix

ultrix:
	miniconf.sh $(CPP)
	${MAKE} ${MFLAGS} \
		DEFINES='-YBSD' \
		${PROG}

apollo:	bsd

hpux:	sys

ibm-rs:	sys

mips:
	miniconf.sh $(CPP)
	${MAKE} ${MFLAGS} \
		DEFINES='-DSYSV -DSIG_RETURNS_INT' \
		${PROG}

sequent-bsd:
	miniconf.sh $(CPP)
	${MAKE} ${MFLAGS} DEFINES='-DBSD' ${PROG} LIBS='-lseq'

sequent-sysv4:
	miniconf.sh $(CPP)
	${MAKE} ${MFLAGS} DEFINES='-DSVR4' ${PROG} LIBS='-lseq'

symmetry:
	miniconf.sh $(CPP)
	${MAKE} ${MFLAGS} \
		DEFINES='-DSIG_RETURNS_INT' \
		${PROG}

mac2:
	miniconf.sh $(CPP)
	${MAKE} ${MFLAGS} \
		DEFINES='-DSYSV -DmacII' LIBS='-lposix' \
		${PROG}

m88k-r3: sys

m88k-r4: svr4

linux:
	./miniconf.sh $(CPP)
	${MAKE} ${MFLAGS} DEFINES='-DPOSIX -DSYSV -DUSE_SYSV_UTMP' ${PROG}


bsd:
	miniconf.sh $(CPP)
	${MAKE} ${MFLAGS} DEFINES='-DBSD' ${PROG}

posix:
	miniconf.sh $(CPP)
	${MAKE} ${MFLAGS} DEFINES='-DPOSIX' ${PROG}

sys:
	miniconf.sh $(CPP)
	${MAKE} ${MFLAGS} DEFINES='-DSYSV' ${PROG}

svr4:
	miniconf.sh $(CPP)
	${MAKE} ${MFLAGS} DEFINES='-DSVR4' ${PROG}


${PROG}: miniconf.h ${OBJS}
	$(CC) $(CDEBUGFLAGS) -o ${PROG} ${OBJS} ${LIBS}

miniconf.h:
	${MAKE} clean
	miniconf.sh $(CPP)

clean:
	${RM} *.o ${PROG} miniconf.h core *~

# DO NOT DELETE THIS LINE -- make depend depends on it.

main.o: config.h io.h
tty.o: config.h 
io.o: config.h io.h
log.o: config.h io.h
hz2gb.o: config.h io.h
unicode.o: config.h io.h
b2g.o: config.h io.h
g2u.o: config.h io.h
b2u.o: config.h io.h
