#!/bin/sh

BBSHOME=/usr/local/bbs
BBSSITE=devel
WWWROOT=/var/www

make distclean

cvs up -d

aclocal; autoheader; automake -a; autoconf

cd bbs2www

aclocal; autoheader; automake -a; autoconf

cd ..

cd sshbbsd

aclocal; autoheader; automake -a; autoconf

cd ..

./configure --prefix=$BBSHOME --enable-site=$BBSSITE \
         --with-www=$WWWROOT --with-php=/usr/include/php --enable-ssh \
         --with-mysql

make
