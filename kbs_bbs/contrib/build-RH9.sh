#!/bin/sh
# $Id$

# 修改这里的参数定制您的站点
# BBSHOME 表示您的 BBS 所在的目录
BBSHOME=/home/bbs

# BBSSITE 表示您采用的站点定制文件
BBSSITE=fb2k-v2

# WWWROOT 表示您的 WWW 所在的目录
# 注意：这个目录不是 httpd.conf 里面的 DocumentRoot 参数。
WWWROOT=/var/www

# 以下的代码请不要修改，除非您能确定自己在干什么
if [ -f Makefile ]; then
	make distclean
fi

cvs up -d

aclocal; autoheader; automake -a; autoconf

if [ -d bbs2www ]; then
	cd bbs2www

	aclocal; autoheader; automake -a; autoconf

	cd ..
	WWWCONFIG="--with-www=$WWWROOT --with-php=/usr/include/php"
else
	WWWCONFIG=--disable-www
fi

if [ -d sshbbsd ]; then
	cd sshbbsd

	aclocal; autoheader; automake -a; autoconf

	cd ..
	SSHCONFIG=--enable-ssh
else
	SSHCONFIG=--disable-ssh
fi

if [ -d innbbsd ]; then
	cd innbbsd

	aclocal; autoheader; automake -a; autoconf

	cd ..
	INNCONFIG=--enable-innbbsd
else
	INNCONFIG=--disable-innbbsd
fi

./configure --prefix=$BBSHOME --enable-site=$BBSSITE \
         $WWWCONFIG \
         $SSHCONFIG \
         $INNCONFIG \
         --with-mysql

make
