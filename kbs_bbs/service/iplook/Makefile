#
#
#  What a shit Makefile!
#
#

# this is the target name used for bin2obj to generate objects
# elf32-little is for linux/freebsd, etc

ARCH_TARGET = elf32-little

all: libiplook.so test
	
libiplook.so: look.c ipinfo.o
	gcc -g -shared -o libiplook.so -O6 look.c ipinfo.o

ipinfo.o: ipinfo_tree string_pool string_index ipinfo_version ipinfo_count bin2obj
	./bin2obj ipinfo.o ipinfo_tree string_pool string_index ipinfo_version ipinfo_count
	
bin2obj: bin2obj.cpp 
	g++ -o bin2obj bin2obj.cpp -DARCH_TARGET=\"$(ARCH_TARGET)\" -lbfd

ipinfo_tree string_pool string_index : ipmask.txt build
	./build < ipmask.txt

build: build.cpp
	g++ -o build build.cpp

ipmask.txt: convert QQwry.txt
	./convert < QQwry.txt > ipmask.txt

convert: convert.cpp
	g++ -o convert convert.cpp

looktest: looktest.c ipinfo.o
	gcc -o looktest -O2 looktest.c ipinfo.o 

test: test.c iplook.h
	gcc -g -o test test.c -liplook -L.

decode.o: decode.asm
	nasm -f elf decode.asm

wryext: wryext.c decode.o
	gcc -o wryext wryext.c decode.o

QQwry.txt ipinfo_version ipinfo_count: QQWry.Dat wryext
	./wryext QQWry.Dat > QQwry.txt

updateip:
	wget http://96hk.y365.com/soft/qqwry.rar
	unrar e qqwry.rar QQWry.Dat
	
clean:
	rm -rf convert ipmask.txt build bin2obj ipinfo_tree string_index string_pool libiplook.so *.o  test looktest wryext ipinfo_version ipinfo_count
