#!/bin/sh
DAY=`date +%Y-%m-%d`
SOURCE=/backup/Announce/$DAY
BBSHOME=/home/bbs
DEST=$BBSHOME/announce/$DAY
TEMP=$BBSHOME/temp

if [ ! -d "$DEST" ]; then
	mkdir -p $DEST
	cd $TEMP
	for i in $SOURCE/*.tar.gz; do
		gzip -dc $i | tar -xf -
		$BBSHOME/bin/Xhtml -o $DEST/ -w $TEMP/ $SOURCE/${i%%.*}
		rm -rf $TEMP/${i%%.*}
		rm -f $DEST/${i%%.*}.bbs.tgz
	done
fi

find ${DEST%/*} -maxdepth 1 -mtime +30 | xargs rm -rf
