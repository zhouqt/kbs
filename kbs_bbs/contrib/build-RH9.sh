#!/bin/sh

make distclean

cvs up -d

autoheader; aclocal; automake -a; autoconf

cd bbs2www

autoheader; aclocal; automake -a; autoconf

cd ..

cd sshbbsd

autoheader; aclocal; automake -a; autoconf

cd ..

./configure --prefix=/usr/local/bbs --enable-site=devel \
         --with-www=/var/www --with-php=/usr/include/php --enable-ssh \
         --with-mysql

make
