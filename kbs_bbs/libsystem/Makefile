OBJ =	f_cat.o f_cp.o f_rm.o f_mv.o

all:allthing

#ifeq (Makefile.spec ,$(wildcard Makefile.spec))
include	../Makefile.spec
#endif

.c.o:   ;   $(CC) $(CFLAGS) -c $*.c

allthing:	../libsystem.a

../libsystem.a:	$(OBJ)
	ar rv $@ $?
	$(RANLIB) $@
