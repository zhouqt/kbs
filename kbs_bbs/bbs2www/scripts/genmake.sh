#!/bin/sh

for cfile in *.c
do
	basefile=`echo $cfile | sed -e "s/\.c//g"`
	echo "$basefile: $basefile.o"
	echo "	\$(CC) -o \$@ $basefile.o \$(LDFLAGS)"
	echo
done

