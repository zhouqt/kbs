#!/bin/sh

BBSHOME=/home/bbs
FHEDIT=${BBSHOME}/bin/fhedit
TMPFILE=${BBSHOME}/tmp/combine.`date +%s`

BOARD=$1
COMBINE=$2
COMBINETO=$3

SIZE=`${FHEDIT} --size --board=${BOARD}`
${FHEDIT} --list --simple --board=${BOARD} --number=1 --count=${SIZE} --groupid=${COMBINE} > ${TMPFILE}
for gid in `cat ${TMPFILE}` ; do
  ${FHEDIT} --modify --board=${BOARD} --number=${gid} --groupid=${COMBINETO}
done
rm -f ${TMPFILE}

