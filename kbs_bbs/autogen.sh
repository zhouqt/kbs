#!/bin/sh

# quick & dirty fix for CVS. need better solution. 20051004 atppp
mkdir -p admin; touch admin/Makefile.in;
mkdir -p innbbsd; touch innbbsd/Makefile.in; touch innbbsd/linkinnd.sh.in;

aclocal; libtoolize -c --force; autoheader; automake -a; autoconf
