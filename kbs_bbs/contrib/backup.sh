#!/bin/sh
DAY=`date +%Y-%m-%d`
BACKUPHOME=/backup/bbs/Announce/$DAY
BBSHOME=/home/bbs

if [ ! -d "$BACKUPHOME" ]; then
	mkdir -p $BACKUPHOME
	cd $BBSHOME/0Announce/groups
	for i in `for i in $(grep Path .Names); do echo ${i##*/}; done`; do
		if [ -d "$i" ]; then
			cd $i
			for j in `for j in $(grep Path .Names); do echo ${j##*/}; done`; do
				if [ -d "$j" ]; then
					echo "Backup 0Announce/$i/$j..."
					tar -zcf $BACKUPHOME/$j.tar.gz $j
				fi
			done
			cd ..
		fi
	done

	find ${BACKUPHOME%/*} -maxdepth 1 -mtime +30 | xargs rm -rf
fi

cd $BACKUPHOME
ftp -n <<end
open $1 $2
quote user $3
quote pass $4
mkdir $DAY
cd $DAY
bin
prompt
mput *
quit
end
