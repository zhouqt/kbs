#!/bin/sh
DAY=`date +%Y-%m-%d`
BACKUPHOME=/backup/bbs/Announce/$DAY
BBSHOME=/home/bbs

mkdir -p $BACKUPHOME
cd $BBSHOME/0Announce/groups
for i in `for i in $(grep Path .Names); do echo ${i##*/}; done`; do
	cd $i
	for j in `for j in $(grep Path .Names); do echo ${j##*/}; done`; do
		echo "Backup 0Announce/$i/$j..."
		tar -zcf $BACKUPHOME/$j.tar.gz $j
	done
	cd ..
done

#find $BACKUPHOME -maxdepth 1 -mtime +30 | xargs rm -rf
