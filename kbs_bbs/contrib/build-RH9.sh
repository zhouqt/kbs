#!/bin/sh

make distclean

cvs up -d

aclocal; automake -a; autoconf

cd bbs2www

aclocal; automake -a; autoconf

cd ..

cd sshbbsd

aclocal; automake -a; autoconf

cd ..

./configure --prefix=/usr/local/bbs --enable-site=devel \
         --with-www=/var/www --with-php=/usr/include/php --enable-ssh \
         --with-mysql

make
