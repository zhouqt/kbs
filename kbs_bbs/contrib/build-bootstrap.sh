#!/bin/sh
# $Id$

# 超级懒人建站脚本 by flyriver

# fetch_smthbbs_source
fetch_smthbbs_source() {
  CVSROOT=":pserver:bbsdev@cvs.smth.org:/home/cvs"
  if [ ! -f $HOME/.cvspass ]; then
    cvs -d $CVSROOT login
  else
    RESULT=`grep "$CVSROOT" $HOME/.cvspass`
    if [ "x$RESULT" = "x" ]; then
      cvs -d $CVSROOT login
    fi
  fi
  if [ $? -ne 0 ]; then
    exit 1
  fi
  cvs -d $CVSROOT co smth_bbs || exit 1
  cd smth_bbs
  cvs -d $CVSROOT co bbshome || exit 1
  cvs -d $CVSROOT co sshbbsd || exit 1
  cvs -d $CVSROOT co innbbsd || exit 1
  cd ..
}

fetch_smthbbs_source

BUILDLOG=smthbuild.log

# 检查是否已经安装
# check_installed software_name
check_installed() {
  RPMNAME=$1
  echo -n "Checking $RPMNAME ... "
  RESULT=`rpm -q $RPMNAME | grep "^$RPMNAME"`
  if [ "x$RESULT" = "x" ]; then
    echo "Not installed."
    return 1;
  else
    echo "Installed."
    return 0;
  fi
}

# 下载软件
# fetch_software dir file
fetch_software() {
  DIRNAME="$1"
  FILENAME="$2"
  FULLURL="$1$2"
  echo -n "Fetching software $FILENAME ... "
  wget -q $FULLURL
  if [ $? -ne 0 ]; then
    echo "Failed."
    exit 1
  else
    echo "Success."
  fi
}

# 安装软件
# install_software dir file
install_software() {
  DIRNAME="$1"
  FILENAME="$2"
  if [ ! -f $FILENAME ]; then
    fetch_software "$DIRNAME" "$FILENAME"
  fi
  echo -n "Installing software $FILENAME ... "
  RESULT=`rpm -U $FILENAME 2>&1`
  if [ $? -ne 0 ]; then
    echo "Failed."
    echo $RESULT
    exit 1
  else
    echo "Success."
  fi
}

# 编译软件
# compile_software dir file builddir cfgopt
compile_software() {
  DIRNAME="$1"
  FILENAME="$2"
  BUILDDIR="$3"
  CFGOPT="$4"
  CWD=`pwd`
  if [ ! -f $FILENAME ]; then
    fetch_software "$DIRNAME" "$FILENAME"
  fi
  echo -n "Extracting software $FILENAME ... "
  RESULT=`tar zxf $FILENAME 2>&1`
  if [ $? -ne 0 ]; then
    echo "Failed."
    echo $RESULT
    exit 1
  else
    echo "Success."
  fi
  echo -n "Compiling and installing software $FILENAME ... "
  cd $BUILDDIR
  ./configure $CFGOPT > $BUILDLOG 2>& 1
  if [ $? -ne 0 ]; then
    echo "Configure failed."
    exit 1
  fi
  make >> $BUILDLOG 2>& 1
  if [ $? -ne 0 ]; then
    echo "Compiling failed."
    exit 1
  fi
  make install >> $BUILDLOG 2>& 1
  if [ $? -ne 0 ]; then
    echo "Installing failed."
    exit 1
  fi
  echo "Success."
  cd "$CWD"
}

DLDIR=ftp://dev.smth.org/software/
DLFILE=automake-1.6.1-1.noarch.rpm
check_installed "automake"
if [ $? -eq 0 ]; then
  echo -n "Checking automake version ... "
  AMVER=`automake --version | awk 'NR == 1 {print $4;}'`
  echo -n $AMVER
  RESULT=`echo $AMVER | grep "^1.6"`
  if [ "x$AMVER" = "x" -o "x$RESULT" = "x" ]; then
    echo "  Not OK"
    install_software $DLDIR $DLFILE
  else
    echo "  OK"
  fi
else
  install_software $DLDIR $DLFILE
fi

DLDIR=ftp://dev.smth.org/software/
DLFILE=autoconf-2.57-3.noarch.rpm
check_installed "autoconf"
if [ $? -eq 0 ]; then
  echo -n "Checking autoconf version ... "
  ACFVER=`autoconf --version | awk 'NR == 1 {print $4;}'`
  echo -n $ACFVER
  RESULT=`echo $ACFVER | grep "^2.57"`
  if [ "x$ACFVER" = "x" -o "x$RESULT" = "x" ]; then
    echo "  Not OK"
    install_software $DLDIR $DLFILE
  else
    echo "  OK"
  fi
else
  install_software $DLDIR $DLFILE
fi

DLDIR=ftp://ftp.tsinghua.edu.cn/mirror/FreeBSD/distfiles/
DLFILE=libtool-1.4.3.tar.gz
BUILDDIR=libtool-1.4.3
CFGOPT="--prefix=/usr"
check_installed "libtool"
if [ $? -eq 0 ]; then
  echo -n "Checking libtool version ... "
  LTVER=`libtool --version | awk 'NR == 1 {print $4;}'`
  echo -n $LTVER
  RESULT=`echo $LTVER | grep "^1.4.3"`
  if [ "x$LTVER" = "x" -o "x$RESULT" = "x" ]; then
    echo "  Not OK"
    compile_software $DLDIR $DLFILE $BUILDDIR $CFGOPT
  else
    echo "  OK"
  fi
else
  compile_software $DLDIR $DLFILE $BUILDDIR $CFGOPT
fi

DLDIR=ftp://dev.smth.org/software/
DLFILE=gmp-4.1.2-2.i386.rpm
DEVFILE=gmp-devel-4.1.2-2.i386.rpm
check_installed "gmp"
if [ $? -ne 0 ]; then
  install_software $DLDIR $DLFILE
  install_software $DLDIR $DEVFILE
fi

DLDIR=ftp://dev.smth.org/software/
DLFILE=libesmtp-1.0-2.i386.rpm
DEVFILE=libesmtp-devel-1.0-2.i386.rpm
check_installed "libesmtp"
if [ $? -ne 0 ]; then
  install_software $DLDIR $DLFILE
  install_software $DLDIR $DEVFILE
fi

DLDIR=ftp://ftp.tsinghua.edu.cn/mirror/apache.org/dist/httpd/
DLFILE=apache_1.3.29.tar.gz
BUILDDIR=apache_1.3.29
CFGOPT="--prefix=/usr/local/www --enable-module=so"
compile_software $DLDIR $DLFILE $BUILDDIR $CFGOPT

DLDIR=ftp://ftp.tsinghua.edu.cn/mirror/php.net/distributions/
DLFILE=php-4.3.4.tar.gz
BUILDDIR=php-4.3.4
CFGOPT="--with-apxs=/usr/local/www/bin/apxs"
compile_software $DLDIR $DLFILE $BUILDDIR $CFGOPT

# 检查及设置本地 bbs 用户
BBSUID=
BBSGID=

getbbsug() {
  BBSUID=`grep "^bbs:" /etc/passwd | cut -d ':' -f 3`
  BBSGID=`grep "^bbs:" /etc/group | cut -d ':' -f 3`
  if [ "x$BBSUID" = "x" -o "x$BBSGID" = "x" ]; then
    return 1;
  else
    return 0;
  fi
}

# make_bbs_ug builddir bbssite
make_bbs_ug() {
  BUILDDIR="$1"
  BBSSITE="$2"
  CWD=`pwd`
  cd "$BUILDDIR/site"
  BBSUID=`grep "^#define BBSUID" $BBSSITE.h | awk '{print $3;}'`
  BBSGID=`grep "^#define BBSGID" $BBSSITE.h | awk '{print $3;}'`
  OLDUID=$BBSUID
  OLDGID=$BBSGID

  getbbsug
  if [ $? -ne 0 ]; then
    /usr/sbin/groupadd -g $OLDGID bbs
    /usr/sbin/adduser -u $OLDUID -g $OLDGID bbs
  else
    if [ "$BBSUID" != "$OLDUID" -o "$BBSGID" != "$OLDGID" ]; then
      PATTERN="s/^#define BBSUID.*$OLDUID/#define BBSUID $BBSUID/g"
      sed -e "$PATTERN" $BBSSITE.h > $BBSSITE.bak
      mv $BBSSITE.bak $BBSSITE.h
      PATTERN="s/^#define BBSGID.*$OLDGID/#define BBSGID $BBSGID/g"
      sed -e "$PATTERN" $BBSSITE.h > $BBSSITE.bak
      mv $BBSSITE.bak $BBSSITE.h
    fi
  fi
  cd "$CWD"
}

# install_smthbbs builddir bbshome bbssite cfgopt
install_smthbbs() {
  BUILDDIR="$1"
  BBSHOME="$2"
  BBSSITE="$3"
  CFGOPT="--prefix=$BBSHOME --enable-site=$BBSSITE $4"
  CWD=`pwd`

  echo -n "Compiling and installing smthbbs ... "
  cd $BUILDDIR

  aclocal; autoheader; automake -a; autoconf

  cd bbs2www
  aclocal; autoheader; automake -a; autoconf
  cd ..

  cd sshbbsd
  aclocal; autoheader; automake -a; autoconf
  cd ..

  cd innbbsd
  aclocal; autoheader; automake -a; autoconf
  cd ..

  ./configure $CFGOPT > $BUILDLOG 2>& 1
  if [ $? -ne 0 ]; then
    echo "Configure failed."
    exit 1
  fi
  make >> $BUILDLOG 2>& 1
  if [ $? -ne 0 ]; then
    echo "Compiling failed."
    exit 1
  fi
  make install >> $BUILDLOG 2>& 1
  if [ $? -ne 0 ]; then
    echo "Installing failed."
    exit 1
  fi
  make install-home >> $BUILDLOG 2>& 1
  if [ $? -ne 0 ]; then
    echo "Installing data failed."
    exit 1
  fi
  echo "Success."
  chown -R bbs:bbs $BBSHOME
  cd "$CWD"
}

BUILDDIR=smth_bbs
BBSHOME=/usr/local/bbs
BBSSITE=fb2k
make_bbs_ug $BUILDDIR $BBSSITE

BUILDDIR=smth_bbs
BBSHOME=/usr/local/bbs
BBSSITE=fb2k
CFGOPT="--with-www=/usr/local/www"
CFGOPT="$CFGOPT --with-php=/usr/local/include/php"
CFGOPT="$CFGOPT --with-php=/usr/local/include/php"
CFGOPT="$CFGOPT --with-mysql"
CFGOPT="$CFGOPT --enable-ssh"
CFGOPT="$CFGOPT --enable-ssl"
install_smthbbs $BUILDDIR $BBSHOME $BBSSITE $CFGOPT
