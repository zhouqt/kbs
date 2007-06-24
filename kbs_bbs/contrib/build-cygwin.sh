#!/bin/sh

# 注意: 这个脚本仅适合于 smthbbs-1.2.x stable 版本

CYGWINBASE=/opt/cygwin
CYGWINDIST=$CYGWINBASE/i686-pc-cygwin
BBSHOME=/usr/local/bbs
PATH=$CYGWINBASE/bin:$PATH
CC=i686-pc-cygwin-gcc
CFLAGS=-O3
DLLTOOL=i686-pc-cygwin-dlltool
export PATH CC CFLAGS DLLTOOL

./configure --prefix=$BBSHOME --enable-site=fb2k-v2 --disable-www \
--without-mysql --enable-ssh --enable-innbbsd --without-pthread \
--enable-customizing \
--with-zlib=$CYGWINDIST \
--with-libesmtp=$CYGWINDIST \
--build=i686-pc-linux-gnu --host=i686-pc-cygwin

cd libsystem

make

$CC -shared -o cygsystem.dll \
        -Wl,--out-implib=libsystem.dll.a \
        -Wl,--export-all-symbols \
        -Wl,--enable-auto-import \
        -Wl,--whole-archive  \
        -Wl,--no-whole-archive \
f_cat.o f_cp.o f_rm.o f_mv.o f_touch.o f_ln.o f_lock.o bm_strcmp.o \
uuencode.o memmem.o flock.o base64.o \
-L$CYGWINDIST/lib \
-lcygipc -L$CYGWINDIST/lib -lz  -lesmtp

cp cygsystem.dll $CYGWINDIST/bin
cp libsystem.dll.a $CYGWINDIST/lib

cd ..

cd libBBS

make

$CC -shared -o cygBBS.dll \
        -Wl,--out-implib=libBBS.dll.a \
        -Wl,--export-all-symbols \
        -Wl,--enable-auto-import \
        -Wl,--whole-archive  \
        -Wl,--no-whole-archive \
record.o log.o ucache.o stuff.o md5.o crypt.o pass.o bcache.o utmp.o \
lvars.o site.o boards.o bbs_sendmail.o libmsg.o article.o \
convcode.o sysconf.o libann.o filter.o mgrep.o default.o output.o \
libtmpl.o libpc.o xml.o \
-L$CYGWINDIST/lib \
-lcygipc -L$CYGWINDIST/lib -lz  -lesmtp  -lsystem

cp cygBBS.dll $CYGWINDIST/bin
cp libBBS.dll.a $CYGWINDIST/lib

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


$CC -DBBSMAIN \
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
-lcygipc -L$CYGWINDIST/lib -lz  -lesmtp

cp libbbsd.a $CYGWINDIST/lib

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

$CC -DBBSMAIN -g \
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
-lgmp -lcygipc -L$CYGWINDIST/lib -lz  -lesmtp

cp libsshbbsd.a $CYGWINDIST/lib

cd ..

cd daemon

make

cd ..

cd libsystem

make clean

cd ..

cd libBBS

make clean

cd ..

cd local_utl

make

cd ..

cd mail2bbs

make

cd ..

cd innbbsd

make

cd ..

cd service

cd pip

make

$CC -shared -o pip.dll \
        -Wl,--out-implib=pip.dll.a \
        -Wl,--export-all-symbols \
        -Wl,--enable-auto-import \
        -Wl,--whole-archive  \
        -Wl,--no-whole-archive \
pip_file.o pip_menu.o pip_practice.o pip_basic.o pip_for_fb.o pip_other.o \
pip_special.o pip_ending.o pip_job.o pip_pic.o pip_store.o pip_fight.o \
pip_main.o pip_play.o pip_system.o \
-L$CYGWINDIST/lib \
-lcygipc -L$CYGWINDIST/lib -lbbsd -lz  -lesmtp

$CC -shared -o sshpip.dll \
        -Wl,--out-implib=sshpip.dll.a \
        -Wl,--export-all-symbols \
        -Wl,--enable-auto-import \
        -Wl,--whole-archive  \
        -Wl,--no-whole-archive \
pip_file.o pip_menu.o pip_practice.o pip_basic.o pip_for_fb.o pip_other.o \
pip_special.o pip_ending.o pip_job.o pip_pic.o pip_store.o pip_fight.o \
pip_main.o pip_play.o pip_system.o \
-L$CYGWINDIST/lib \
-lcygipc -L$CYGWINDIST/lib -lsshbbsd -lz  -lesmtp

cd ..

cd worker

make

$CC -shared -o worker.dll \
        -Wl,--out-implib=worker.dll.a \
        -Wl,--export-all-symbols \
        -Wl,--enable-auto-import \
        -Wl,--whole-archive  \
        -Wl,--no-whole-archive \
maingame.o screens.o gamedata.o gamedraw.o gameinit.o gameplay.o gametty.o \
-L$CYGWINDIST/lib \
-lcygipc -L$CYGWINDIST/lib -lbbsd -lsystem -lz  -lesmtp

$CC -shared -o sshworker.dll \
        -Wl,--out-implib=sshworker.dll.a \
        -Wl,--export-all-symbols \
        -Wl,--enable-auto-import \
        -Wl,--whole-archive  \
        -Wl,--no-whole-archive \
maingame.o screens.o gamedata.o gamedraw.o gameinit.o gameplay.o gametty.o \
-L$CYGWINDIST/lib \
-lcygipc -L$CYGWINDIST/lib -lsshbbsd -lsystem -lz  -lesmtp

cd ..

make

$CC -shared -o friendtest.dll \
        -Wl,--out-implib=friendtest.dll.a \
        -Wl,--export-all-symbols \
        -Wl,--enable-auto-import \
        -Wl,--whole-archive  \
        -Wl,--no-whole-archive \
friendtest.o \
-L$CYGWINDIST/lib \
-lcygipc -L$CYGWINDIST/lib -lbbsd -lz  -lesmtp

$CC -shared -o sshfriendtest.dll \
        -Wl,--out-implib=sshfriendtest.dll.a \
        -Wl,--export-all-symbols \
        -Wl,--enable-auto-import \
        -Wl,--whole-archive  \
        -Wl,--no-whole-archive \
friendtest.o \
-L$CYGWINDIST/lib \
-lcygipc -L$CYGWINDIST/lib -lsshbbsd -lz  -lesmtp

$CC -shared -o bbsnet.dll \
        -Wl,--out-implib=bbsnet.dll.a \
        -Wl,--export-all-symbols \
        -Wl,--enable-auto-import \
        -Wl,--whole-archive  \
        -Wl,--no-whole-archive \
bbsnet.o tcplib.o \
-L$CYGWINDIST/lib \
-lcygipc -L$CYGWINDIST/lib -lbbsd -lz  -lesmtp

$CC -shared -o sshbbsnet.dll \
        -Wl,--out-implib=sshbbsnet.dll.a \
        -Wl,--export-all-symbols \
        -Wl,--enable-auto-import \
        -Wl,--whole-archive  \
        -Wl,--no-whole-archive \
bbsnet.o tcplib.o \
-L$CYGWINDIST/lib \
-lcygipc -L$CYGWINDIST/lib -lsshbbsd -lz  -lesmtp

$CC -shared -o calc.dll \
        -Wl,--out-implib=calc.dll.a \
        -Wl,--export-all-symbols \
        -Wl,--enable-auto-import \
        -Wl,--whole-archive  \
        -Wl,--no-whole-archive \
calc.o \
-L$CYGWINDIST/lib \
-lcygipc -L$CYGWINDIST/lib -lbbsd -lz  -lesmtp

$CC -shared -o sshcalc.dll \
        -Wl,--out-implib=sshcalc.dll.a \
        -Wl,--export-all-symbols \
        -Wl,--enable-auto-import \
        -Wl,--whole-archive  \
        -Wl,--no-whole-archive \
calc.o \
-L$CYGWINDIST/lib \
-lcygipc -L$CYGWINDIST/lib -lsshbbsd -lz  -lesmtp

$CC -shared -o calendar.dll \
        -Wl,--out-implib=calendar.dll.a \
        -Wl,--export-all-symbols \
        -Wl,--enable-auto-import \
        -Wl,--whole-archive  \
        -Wl,--no-whole-archive \
calendar.o \
-L$CYGWINDIST/lib \
-lcygipc -L$CYGWINDIST/lib -lbbsd -lz  -lesmtp

$CC -shared -o sshcalendar.dll \
        -Wl,--out-implib=sshcalendar.dll.a \
        -Wl,--export-all-symbols \
        -Wl,--enable-auto-import \
        -Wl,--whole-archive  \
        -Wl,--no-whole-archive \
calendar.o \
-L$CYGWINDIST/lib \
-lcygipc -L$CYGWINDIST/lib -lsshbbsd -lz  -lesmtp

$CC -shared -o killer.dll \
        -Wl,--out-implib=killer.dll.a \
        -Wl,--export-all-symbols \
        -Wl,--enable-auto-import \
        -Wl,--whole-archive  \
        -Wl,--no-whole-archive \
killer.o \
-L$CYGWINDIST/lib \
-lcygipc -L$CYGWINDIST/lib -lbbsd -lz  -lesmtp

$CC -shared -o sshkiller.dll \
        -Wl,--out-implib=sshkiller.dll.a \
        -Wl,--export-all-symbols \
        -Wl,--enable-auto-import \
        -Wl,--whole-archive  \
        -Wl,--no-whole-archive \
killer.o \
-L$CYGWINDIST/lib \
-lcygipc -L$CYGWINDIST/lib -lsshbbsd -lz  -lesmtp

$CC -shared -o tt.dll \
        -Wl,--out-implib=tt.dll.a \
        -Wl,--export-all-symbols \
        -Wl,--enable-auto-import \
        -Wl,--whole-archive  \
        -Wl,--no-whole-archive \
tt.o \
-L$CYGWINDIST/lib \
-lcygipc -L$CYGWINDIST/lib -lbbsd -lz  -lesmtp

$CC -shared -o sshtt.dll \
        -Wl,--out-implib=sshtt.dll.a \
        -Wl,--export-all-symbols \
        -Wl,--enable-auto-import \
        -Wl,--whole-archive  \
        -Wl,--no-whole-archive \
tt.o \
-L$CYGWINDIST/lib \
-lcygipc -L$CYGWINDIST/lib -lsshbbsd -lz  -lesmtp

$CC -shared -o tetris.dll \
        -Wl,--out-implib=tetris.dll.a \
        -Wl,--export-all-symbols \
        -Wl,--enable-auto-import \
        -Wl,--whole-archive  \
        -Wl,--no-whole-archive \
tetris.o \
-L$CYGWINDIST/lib \
-lcygipc -L$CYGWINDIST/lib -lbbsd -lz  -lesmtp

$CC -shared -o sshtetris.dll \
        -Wl,--out-implib=sshtetris.dll.a \
        -Wl,--export-all-symbols \
        -Wl,--enable-auto-import \
        -Wl,--whole-archive  \
        -Wl,--no-whole-archive \
tetris.o \
-L$CYGWINDIST/lib \
-lcygipc -L$CYGWINDIST/lib -lsshbbsd -lz  -lesmtp

cd ..

