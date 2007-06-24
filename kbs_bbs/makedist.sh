#!/bin/sh
cpfile() {
if [ ! -f $1/$2 ]; then
  /bin/cp  $2 $1/$2
fi
}

md() {
if [ ! -d $1/$2 ]; then
  mkdir $1/$2
fi
}

echo -n "Warning: This will OVERWRITE WHOLE BBS DATA, confirm [N]?"
read ans
ans=${ans:-N}
case $ans in
    [Yy]) echo "Installing new bbs to $1" ;;
    *) echo "Abort ..." ; exit ;;
esac
cd bbshome
for i in `find . -type d`; do
md $1 $i
done
for i in `find . -type f`; do
cpfile $1 $i
done
md $1/home
md $1/mail
for i in A B C D E F G H I J K L M N O P Q R S T U V W X Y Z; do
md $1/home $i
md $1/mail $i
done
md $1/vote
md $1/boards
md $1/boards/_attach
md $1/cache
md $1/etc/posts
md $1/game
md $1/game/pipgame
md $1/game/worker
md $1/tmp
md $1/tmp/forward
md $1/tmp/bbs-gsend
md $1/tmp/email
md $1/tmp/clip
md $1/tmp/bbs-internet-gw
md $1/tmp/Xpost
md $1/var
md $1/xml
