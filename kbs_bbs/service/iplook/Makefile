#
#
#  What a shit Makefile!
#
#

all: libiplook.so test
	
libiplook.so: look.c ipinfo.c
	gcc -shared -o libiplook.so -O6 look.c ipinfo.c
	
ipinfo.c: ipinfo.dat bincvt
	./bincvt 

bincvt: bincvt.c
	gcc -o bincvt bincvt.c

ipinfo.dat: ipmask.txt build
	./build < ipmask.txt

build: build.cpp
	g++ -o build build.cpp

ipmask.txt: convert QQwry.txt
	./convert < QQwry.txt > ipmask.txt

convert: convert.cpp
	g++ -o convert convert.cpp

test: test.c iplook.h
	gcc -o test test.c -liplook -L.

clean:
	rm -rf convert ipmask.txt build bincvt ipinfo.dat ipinfo.c libiplook.so *.o  test
