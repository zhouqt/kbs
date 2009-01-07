#!/bin/sh
if [ `uname -s` = Darwin ]
then
    LIBTOOLIZE=glibtoolize
else
    LIBTOOLIZE=libtoolize
fi

aclocal; $LIBTOOLIZE -c --force; autoheader; automake -a; autoconf
