#!/bin/bash
gcc -g -o newindex newindex.c
gcc -g -o newqueryd newqueryd.c -lm
mkdir /home/bbs
cp -f newindex /home/bbs
cp -f newqueryd /home/bbs
cp -f word.txt /home/bbs
cd /home/bbs
mkdir /home/bbs/bk
./newindex
killall newqueryd
cp -f bk/* .
rm -f _all_.index
./newqueryd &
