#!/usr/bin/perl

# script used to build links 


$dir = shift or $dir = "..";
@clist = `ls $dir`;
foreach $cfile (@clist) {
   chop($cfile);
   if ($cfile =~ /(.+)\.[cC]$/) {
      unlink "$1.c"; 
      symlink ("$dir/$1.c", "$1.c");
   }
}
symlink ("$dir/Makefile.spec", "Makefile.spec");

