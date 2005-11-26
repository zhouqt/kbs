#!/bin/bash

# $Id$
# 自动生成 kbs-cygwin 二进制打包的脚本

CYGWINBASE=/opt/cygwin
CYGWINDIST=$CYGWINBASE/i686-pc-cygwin
BBSHOME=/home/kbs/bbs
PATH=$CYGWINBASE/bin:$PATH
CC=i686-pc-cygwin-gcc
CFLAGS=-O3
DLLTOOL=i686-pc-cygwin-dlltool
IFS=
export BBSHOME PATH CC CFLAGS DLLTOOL IFS

./configure --prefix=$BBSHOME --enable-site=fb2k-v2 --disable-www \
--with-mysql=$CYGWINDIST --enable-ssh --enable-innbbsd --without-pthread \
--enable-customizing \
--with-zlib=$CYGWINDIST \
--with-libesmtp=$CYGWINDIST \
--build=i686-pc-linux-gnu --host=i686-pc-cygwin || exit 1

rm -rf $BBSHOME/*
rm -rf $BBSHOME/.* > /dev/null 2>&1

make || exit 2
make install || exit 3
echo y | make install-home || exit 4
make clean

build_dll() {
  BASE_NAME_="$1"
  OBJ_FILES_="$2"
  LD_FLAGS_="$3"
  DLL_NAME_="cyg$BASE_NAME_"
  IMP_NAME_="lib$BASE_NAME_.a"
  CMD_LINE="$CC -shared -o $DLL_NAME_ \
        -Wl,--out-implib=$IMP_NAME_ \
        -Wl,--export-all-symbols \
        -Wl,--enable-auto-import \
        -Wl,--whole-archive  \
        -Wl,--no-whole-archive \
  $OBJ_FILES_ \
  $LD_FLAGS_"
  echo $CMD_LINE
  eval $CMD_LINE

  return $?
}

build_service_dll() {
  BASE_NAME_="$1"
  OBJ_FILES_="$2"
  LD_FLAGS_="$3"
  DLL_NAME_="$BASE_NAME_"
  IMP_NAME_="$BASE_NAME_.a"
  CMD_LINE="$CC -shared -o $DLL_NAME_ \
        -Wl,--out-implib=$IMP_NAME_ \
        -Wl,--export-all-symbols \
        -Wl,--enable-auto-import \
        -Wl,--whole-archive  \
        -Wl,--no-whole-archive \
  $OBJ_FILES_ \
  $LD_FLAGS_"
  echo $CMD_LINE
  eval $CMD_LINE

  return $?
}

install_file() {
  FILE_NAME="$1"
  INST_DIR="$2"
  cp $FILE_NAME $INST_DIR
}

install_service() {
  FILE_NAME="$1"
  SERVICE_DEST="$2"
  INST_DIR="service"
  LINK_FILE="$BBSHOME/$INST_DIR/$FILE_NAME.dll"

  IS_SSH=`echo $FILE_NAME | grep "^ssh"`
  if [ "x$IS_SSH" = "x" ]; then
    LINK_DEST="$BBSHOME/$SERVICE_DEST/lib$FILE_NAME.so"
  else
    BASE_NAME=`echo $FILE_NAME | sed -e 's/^ssh//g'`
    LINK_DEST="$BBSHOME/$SERVICE_DEST/lib$BASE_NAME.ssh.so"
  fi

  cp $FILE_NAME.dll $LINK_FILE
  ln -s $LINK_FILE $LINK_DEST
}

cd libsystem

make

DLL_NAME="system.dll"
OBJ_FILES="f_cat.o f_cp.o f_rm.o f_mv.o f_touch.o f_ln.o f_lock.o bm_strcmp.o \
          uuencode.o memmem.o flock.o base64.o"
DLL_LD_FLAGS="-L$CYGWINDIST/lib \
             -lcygipc -L$CYGWINDIST/lib -lz -lesmtp"
build_dll $DLL_NAME $OBJ_FILES $DLL_LD_FLAGS || exit 5

install_file cygsystem.dll $BBSHOME/bin
install_file libsystem.dll.a $CYGWINDIST/lib

cd ..

cd libBBS

make

DLL_NAME="BBS.dll"
OBJ_FILES="record.o log.o ucache.o stuff.o md5.o crypt.o pass.o bcache.o \
           utmp.o lvars.o site.o boards.o bbs_sendmail.o libmsg.o article.o \
           convcode.o sysconf.o libann.o filter.o mgrep.o default.o output.o \
           libtmpl.o libpc.o xml.o super_filter.o"
DLL_LD_FLAGS="-L$CYGWINDIST/lib \
              -lcygipc -L$CYGWINDIST/lib -lz -lesmtp -lsystem \
              -L$CYGWINDIST/lib/mysql -lmysqlclient"
build_dll $DLL_NAME $OBJ_FILES $DLL_LD_FLAGS || exit 6

install_file cygBBS.dll $BBSHOME/bin
install_file libBBS.dll.a $CYGWINDIST/lib

cd ..

cd rzsz

make

cd ..

cd src

make

$DLLTOOL --export-all --output-def bbsd.def \
bbs.o record.o delete.o stuff.o mail.o register.o xyz.o talk.o \
comm_lists.o namecomplete.o chat.o maintain.o edit.o more.o pass.o help.o \
bcache.o boards.o screen.o userinfo.o vote.o announce.o sendmsg.o boards_t.o \
bm.o list.o goodbye.o bbsd_single.o lvars.o select.o libann.o \
newmain_single.o newio.o newterm.o convcode.o bbs_sendmail.o crypt.o ucache.o \
utmp.o log.o md5.o libmsg.o article.o sysconf.o site.o filter.o mgrep.o \
definekey.o default.o calltime.o super_filter.o libtmpl.o msg_manager.o \
addr_manage.o libpc.o personal_corp.o newread.o

$DLLTOOL --dllname bbsd.exe --def bbsd.def --output-lib libbbsd.a

$DLLTOOL --dllname bbsd.exe --output-exp bbsd.exp --def bbsd.def


$CC -DBBSMAIN -DINVISIBLE -DSAFE_KILL -D_DETAIL_UINFO_ -DSHOW_IDLE_TIME \
-DCYGWIN -I/usr/local/include -o bbsd.exe bbsd.exp \
bbs.o record.o delete.o stuff.o mail.o register.o xyz.o talk.o \
comm_lists.o namecomplete.o chat.o maintain.o edit.o more.o pass.o help.o \
bcache.o boards.o screen.o userinfo.o vote.o announce.o sendmsg.o boards_t.o \
bm.o list.o goodbye.o bbsd_single.o lvars.o select.o libann.o \
newmain_single.o newio.o newterm.o convcode.o bbs_sendmail.o crypt.o ucache.o \
utmp.o log.o md5.o libmsg.o article.o sysconf.o site.o filter.o mgrep.o \
definekey.o default.o calltime.o super_filter.o libtmpl.o msg_manager.o \
addr_manage.o libpc.o personal_corp.o newread.o \
../rzsz/.libs/libzmodem.a \
-L$CYGWINDIST/lib -lsystem \
-lcygipc -L$CYGWINDIST/lib -lz -lesmtp \
-L$CYGWINDIST/lib/mysql -lmysqlclient || exit 7

install_file bbsd.exe $BBSHOME/bin
install_file libbbsd.a $CYGWINDIST/lib

cd ..

cd sshbbsd

make
$DLLTOOL --export-all --output-def sshbbsd.def \
sshd.o auth-passwd.o log-server.o canohost.o servconf.o tildexpand.o \
serverloop.o rsa.o randoms.o buffer.o emulate.o packet.o compress.o \
xmalloc.o bufaux.o authfile.o crc32.o rsaglue.o cipher.o des.o match.o \
mpaux.o userfile.o signals.o blowfish.o deattack.o  addr_manage.o \
announce.o article.o bbs.o bbs_sendmail.o bbsd_single.o bcache.o bm.o \
boards.o boards_t.o calltime.o chat.o comm_lists.o convcode.o crypt.o \
default.o definekey.o delete.o edit.o filter.o goodbye.o help.o libann.o \
libmsg.o libpc.o libtmpl.o list.o log.o lvars.o mail.o maintain.o md5.o \
mgrep.o more.o msg_manager.o namecomplete.o newhelp.o newio.o \
newmain_single.o newread.o newterm.o output.o pass.o personal_corp.o \
record.o register.o screen.o select.o sendmsg.o site.o stuff.o \
super_filter.o sysconf.o talk.o ucache.o userinfo.o utmp.o vote.o xml.o \
xyz.o  idea.o

$DLLTOOL --dllname sshbbsd.exe --def sshbbsd.def --output-lib libsshbbsd.a

$DLLTOOL --dllname sshbbsd.exe --output-exp sshbbsd.exp --def sshbbsd.def

$CC -DBBSMAIN -DINVISIBLE -DSAFE_KILL -D_DETAIL_UINFO_ -DSHOW_IDLE_TIME -g \
-DCYGWIN -I/usr/local/include -o sshbbsd.exe sshbbsd.exp \
sshd.o auth-passwd.o log-server.o canohost.o servconf.o tildexpand.o \
serverloop.o rsa.o randoms.o buffer.o emulate.o packet.o compress.o \
xmalloc.o bufaux.o authfile.o crc32.o rsaglue.o cipher.o des.o match.o \
mpaux.o userfile.o signals.o blowfish.o deattack.o  addr_manage.o \
announce.o article.o bbs.o bbs_sendmail.o bbsd_single.o bcache.o bm.o \
boards.o boards_t.o calltime.o chat.o comm_lists.o convcode.o crypt.o \
default.o definekey.o delete.o edit.o filter.o goodbye.o help.o libann.o \
libmsg.o libpc.o libtmpl.o list.o log.o lvars.o mail.o maintain.o md5.o \
mgrep.o more.o msg_manager.o namecomplete.o newhelp.o newio.o \
newmain_single.o newread.o newterm.o output.o pass.o personal_corp.o \
record.o register.o screen.o select.o sendmsg.o site.o stuff.o \
super_filter.o sysconf.o talk.o ucache.o userinfo.o utmp.o vote.o xml.o \
xyz.o  idea.o \
../rzsz/.libs/libzmodem.a \
-L$CYGWINDIST/lib -lsystem \
-lgmp -lcygipc -L$CYGWINDIST/lib -lz  -lesmtp \
-L$CYGWINDIST/lib/mysql -lmysqlclient || exit 8

install_file sshbbsd.exe $BBSHOME/bin
install_file libsshbbsd.a $CYGWINDIST/lib

cd ..

cd daemon

make

make install

cd ..

cd libsystem

make clean

cd ..

cd libBBS

make clean

cd ..

cd local_utl

make

make install

cd ..

cd mail2bbs

make

make install

cd ..

cd innbbsd

make

make install

cd ..

cd service

DLL_LD_FLAGS="-L$CYGWINDIST/lib -L$CYGWINDIST/lib/mysql -lmysqlclient \
              -lcygipc -L$CYGWINDIST/lib -lbbsd -lz -lesmtp"
SSHDLL_LD_FLAGS="-L$CYGWINDIST/lib -L$CYGWINDIST/lib/mysql -lmysqlclient \
              -lcygipc -L$CYGWINDIST/lib -lsshbbsd -lz -lesmtp"

cd pip

make

OBJ_FILES="pip_file.o pip_menu.o pip_practice.o pip_basic.o pip_for_fb.o \
           pip_other.o pip_special.o pip_ending.o pip_job.o pip_pic.o \
		   pip_store.o pip_fight.o pip_main.o pip_play.o pip_system.o"
build_service_dll "pip.dll" $OBJ_FILES $DLL_LD_FLAGS || exit 10
install_service "pip" "game/so" || exit 10
build_service_dll "sshpip.dll" $OBJ_FILES $SSHDLL_LD_FLAGS || exit 11
install_service "sshpip" "game/so" || exit 11

cd ..

cd worker

make

OBJ_FILES="maingame.o screens.o gamedata.o gamedraw.o gameinit.o gameplay.o \
           gametty.o"
WORKER_LD_FLAGS="$DLL_LD_FLAGS -lsystem"
SSHWORKER_LD_FLAGS="$SSHDLL_LD_FLAGS -lsystem"
build_service_dll "worker.dll" $OBJ_FILES $WORKER_LD_FLAGS || exit 12
install_service "worker" "game/so" || exit 12
build_service_dll "sshworker.dll" $OBJ_FILES $SSHWORKER_LD_FLAGS || exit 13
install_service "sshworker" "game/so" || exit 13

cd ..

make

OBJ_FILES="friendtest.o"
build_service_dll "friendtest.dll" $OBJ_FILES $DLL_LD_FLAGS || exit 14
install_service "friendtest" "service" || exit 14
build_service_dll "sshfriendtest.dll" $OBJ_FILES $SSHDLL_LD_FLAGS || exit 15
install_service "sshfriendtest" "service" || exit 15

OBJ_FILES="bbsnet.o tcplib.o"
build_service_dll "bbsnet.dll" $OBJ_FILES $DLL_LD_FLAGS || exit 16
install_service "bbsnet" "service" || exit 16
build_service_dll "sshbbsnet.dll" $OBJ_FILES $SSHDLL_LD_FLAGS || exit 17
install_service "sshbbsnet" "service" || exit 17

OBJ_FILES="calc.o"
build_service_dll "calc.dll" $OBJ_FILES $DLL_LD_FLAGS || exit 18
install_service "calc" "service" || exit 18
build_service_dll "sshcalc.dll" $OBJ_FILES $SSHDLL_LD_FLAGS || exit 19
install_service "sshcalc" "service" || exit 19

OBJ_FILES="calendar.o"
build_service_dll "calendar.dll" $OBJ_FILES $DLL_LD_FLAGS || exit 20
install_service "calendar" "service" || exit 20
build_service_dll "sshcalendar.dll" $OBJ_FILES $SSHDLL_LD_FLAGS || exit 21
install_service "sshcalendar" "service" || exit 21

OBJ_FILES="killer.o"
build_service_dll "killer.dll" $OBJ_FILES $DLL_LD_FLAGS || exit 22
install_service "killer" "service" || exit 22
build_service_dll "sshkiller.dll" $OBJ_FILES $SSHDLL_LD_FLAGS || exit 23
install_service "sshkiller" "service" || exit 23

OBJ_FILES="tt.o"
build_service_dll "tt.dll" $OBJ_FILES $DLL_LD_FLAGS || exit 24
install_service "tt" "service" || exit 24
build_service_dll "sshtt.dll" $OBJ_FILES $SSHDLL_LD_FLAGS || exit 25
install_service "sshtt" "service" || exit 25

OBJ_FILES="tetris.o"
build_service_dll "tetris.dll" $OBJ_FILES $DLL_LD_FLAGS || exit 26
install_service "tetris" "service" || exit 26
build_service_dll "sshtetris.dll" $OBJ_FILES $SSHDLL_LD_FLAGS || exit 27
install_service "sshtetris" "service" || exit 27

OBJ_FILES="dict.o"
build_service_dll "dict.dll" $OBJ_FILES $DLL_LD_FLAGS || exit 28
install_service "dict" "service" || exit 28
build_service_dll "sshdict.dll" $OBJ_FILES $SSHDLL_LD_FLAGS || exit 29
install_service "sshdict" "service" || exit 29

OBJ_FILES="quiz.o"
build_service_dll "quiz.dll" $OBJ_FILES $DLL_LD_FLAGS || exit 30
install_service "quiz" "service" || exit 30
build_service_dll "sshquiz.dll" $OBJ_FILES $SSHDLL_LD_FLAGS || exit 31
install_service "sshquiz" "service" || exit 31

OBJ_FILES="iquery.o"
build_service_dll "iquery.dll" $OBJ_FILES $DLL_LD_FLAGS || exit 32
install_service "iquery" "service" || exit 32
build_service_dll "sshiquery.dll" $OBJ_FILES $SSHDLL_LD_FLAGS || exit 33
install_service "sshiquery" "service" || exit 33

OBJ_FILES="killer2.o"
build_service_dll "killer2.dll" $OBJ_FILES $DLL_LD_FLAGS || exit 34
install_service "killer2" "service" || exit 34
build_service_dll "sshkiller2.dll" $OBJ_FILES $SSHDLL_LD_FLAGS || exit 35
install_service "sshkiller2" "service" || exit 35

OBJ_FILES="ansieditor.o"
build_service_dll "ansieditor.dll" $OBJ_FILES $DLL_LD_FLAGS || exit 36
install_service "ansieditor" "service" || exit 36
build_service_dll "sshansieditor.dll" $OBJ_FILES $SSHDLL_LD_FLAGS || exit 37
install_service "sshansieditor" "service" || exit 37

OBJ_FILES="winmine.o"
build_service_dll "winmine.dll" $OBJ_FILES $DLL_LD_FLAGS || exit 38
install_service "winmine" "service" || exit 38
build_service_dll "sshwinmine.dll" $OBJ_FILES $SSHDLL_LD_FLAGS || exit 39
install_service "sshwinmine" "service" || exit 39

OBJ_FILES="pandora.o tcplib.o"
build_service_dll "pandora.dll" $OBJ_FILES $DLL_LD_FLAGS || exit 40
install_service "pandora" "service" || exit 40
build_service_dll "sshpandora.dll" $OBJ_FILES $SSHDLL_LD_FLAGS || exit 40
install_service "sshpandora" "service" || exit 40

cd ..

