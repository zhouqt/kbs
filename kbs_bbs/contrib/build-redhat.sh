#!/bin/sh
# $Id$
#
# 快速编译脚本 for RedHat 9, Fedora Core 1/2/3/4/5/6, Fedora 7/8
#
# 修改这里的参数定制您的站点
# BBSHOME 表示您的 BBS 所在的目录
BBSHOME=/usr/local/bbs

# BBSSITE 表示您采用的站点定制文件
BBSSITE=fb2k-v2

# 以下的代码请不要修改，除非您能确定自己在干什么
if [ -f Makefile ]; then
  make distclean
fi

if [ -d .svn ]; then
  svn up
  ./autogen.sh
fi

if [ -d sshbbsd/.svn ]; then
  cd sshbbsd
  ./autogen.sh
  cd ..
fi

./configure --prefix=$BBSHOME --enable-site=$BBSSITE \
     --with-php --enable-ssh --enable-innbbsd --with-mysql

make
