OBJ =	f_cat.o f_cp.o f_rm.o f_mv.o f_touch.o f_ln.o
RANLIB=ranlib

all:allthing

#ifeq (Makefile.spec ,$(wildcard Makefile.spec))
include	../Makefile.spec
#endif

.c.o:   ;   $(CC) $(CFLAGS) -I.. -c $*.c

allthing:	../libsystem.a

dep:
	rm -f .depend
	echo $(patsubst %.o,%.c,$(OBJ))|xargs gcc -I.. -M >> .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif

../libsystem.a:	$(OBJ)
	ar rv $@ $?
	$(RANLIB) $@

clean:
	rm -f *.o ../libsystem.a
