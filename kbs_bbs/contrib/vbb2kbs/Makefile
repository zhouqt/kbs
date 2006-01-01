BBSHOME = /home/bbs
CFLAGS = -I/usr/include/mysql -I../../src -L../../libBBS/.libs -L../../libsystem/.libs -lmysqlclient -lBBS -lsystem  -Wl,--rpath -Wl,$(BBSHOME)/lib -Wall -ggdb
CC = gcc $(CFLAGS)

convert: convert.h convert.c
	$(CC) -o convert convert.c

clean:
	rm convert
