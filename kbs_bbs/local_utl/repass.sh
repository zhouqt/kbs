#! /bin/sh
gcc -o repass repass.c
gcc -o rehome rehome.c rehome.o
cp /home/bbs/.PASSWDS /home/bbs/.PASSWDS.old
repass
cp /home/bbs/.PASSWDS.tmp /home/bbs/.PASSWDS
chown bbs.bbs /home/bbs/.PASSWDS
rehome
