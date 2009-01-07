#!/bin/sh

# quick & dirty fix for CVS. need better solution. 20051004 atppp
mkdir -p admin; touch admin/Makefile.in;

if [ `uname -s` = Darwin ]
then
    LIBTOOLIZE=glibtoolize
else
    LIBTOOLIZE=libtoolize
fi

aclocal; $LIBTOOLIZE -c --force; autoheader; automake -a; autoconf
