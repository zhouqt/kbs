#!/bin/sh
DAY=`date +%Y-%m-%d`
SOURCE=/backup/Announce/$DAY
EXTRACT=/backup/temp
BBSHOME=/home/bbs
DEST=$BBSHOME/announce/$DAY
TEMP=$BBSHOME/temp

if [ ! -d "$DEST" ]; then
	mkdir -p $DEST
	cd $SOURCE
	for i in *.tar.gz; do
		cd $EXTRACT
		gzip -dc $SOURCE/$i | tar -xf -
		$BBSHOME/bin/Xhtml -o $DEST/ -w $TEMP/ $EXTRACT/${i%%.*}
		rm -rf $EXTRACT/${i%%.*}
		rm -f $DEST/${i%%.*}.bbs.tgz
	done
fi

find ${DEST%/*} -mtime +30 | xargs rm -rf
