#!/bin/sh

BBSHOME=/usr/local/bbs
BBSSITE=devel
WWWROOT=/var/www

make distclean

cvs up -d

autoheader; aclocal; automake -a; autoconf

cd bbs2www

autoheader; aclocal; automake -a; autoconf

cd ..

cd sshbbsd

autoheader; aclocal; automake -a; autoconf

cd ..

./configure --prefix=$BBSHOME --enable-site=$BBSSITE \
         --with-www=$WWWROOT --with-php=/usr/include/php --enable-ssh \
         --with-mysql

make
