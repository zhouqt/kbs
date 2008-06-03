#!/bin/sh

#### backup mail home boards 0Announce
#### set BBSHOME,BACKUPDIR to correct value before use it!

BACKUPCMD="tar zcf"
LOGFILE="/tmp/backup$$.log"
NOTICECMD="/home/bbs/bin/postfilealt.php"

log() {
  echo `date +"%Y%m%d %H:%M:%S"`:$1 > $LOGFILE
}

logerror() {
  log $2
  $NOTICECMD $LOGFILE SYSOP "backup $1 error" system_dev
  cat $LOGFILE
  rm $LOGFILE
  exit
}

execcmd() {
  log $1 >> $LOGFILE
  $1 2>&1 >> $LOGFILE
}

rotatefile() {
  if [ -f $1 ]; then
    execcmd "mv -f $1 $1.bak"
  fi
}

if [ "x$1" == "x" ] || ([ "$1" != "mail" ] && [ "$1" != "home" ] && [ "$1" != "boards" ] && [ "$1" != "0Announce" ]); then
  logerror "" "usage:$0 mail|home|boards|0Announce"
fi


if [ "x${BBSHOME}" == "x" ] || [ ! -d $BBSHOME ]; then
  logerror $1 "BBSHOME no set or incorrect!"
fi

if [ "x${BACKUPDIR}" == "x" ] || [ ! -d $BACKUPDIR ]; then
  logerror $1 "BACKUPDIR no set or incorrect!"
fi

log "backup $1 begin"
if [ "$1" == "0Announce" ]; then
  TODIR=$BACKUPDIR/0Announce/groups
  SOURCEDIR=$BBSHOME/0Announce/groups
  execcmd "mkdir -p $TODIR"
  ## backup Announce without board's.
  rotatefile $BACKUPDIR/0Announce/all_except_groups.tgz 
  execcmd "$BACKUPCMD $BACKUPDIR/0Announce/all_except_groups.tgz $BBSHOME/0Announce --exclude $BBSHOME/0Announce/groups"
else
  TODIR=$BACKUPDIR/$1
  SOURCEDIR=$BBSHOME/$1
  execcmd "mkdir -p $TODIR"
fi

cd $SOURCEDIR
DIRS=
for i in *; do
  if [ -d $i ]; then
    rotatefile $TODIR/$i.tgz
    execcmd "$BACKUPCMD $TODIR/$i.tgz $i"
    DIRS="$DIRS --exclude $SOURCEDIR/$i"
  fi
done
rotatefile $TODIR/__other.tgz
execcmd "$BACKUPCMD $TODIR/__other.tgz $SOURCEDIR $DIRS"
log "backup $1 end" >> $LOGFILE

cat $LOGFILE
$NOTICECMD $LOGFILE SYSOP "backup $1 success" system_dev
rm $LOGFILE
