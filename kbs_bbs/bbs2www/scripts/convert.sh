#!/bin/sh

for cfile in *.c
do
	echo -n "Converting $cfile ..."
	mv $cfile $cfile.BAK
	sed -e 's/BBSLIB\.inc/bbslib\.h/g' $cfile.BAK > $cfile
	rm -f $cfile.BAK
	echo "Done."
done

