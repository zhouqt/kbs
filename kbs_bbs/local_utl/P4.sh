#! /bin/sh
make repass
make rehome
make refriend
cp /home/bbs/.PASSWDS /home/bbs/.PASSWDS.old
repass
cp .PASSWDS.tmp /home/bbs/.PASSWDS
chown bbs.bbs /home/bbs/.PASSWDS
refriend
rehome
