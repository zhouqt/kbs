#!/bin/sh
awk 'BEGIN { definenum=0;infilter=0; } \
  $0~/#ifdef[ \t]*FILTER/ { if (infilter==0) {infilter=1;definenum=0;} } \
  $0~/#ifdef[ \t]*SMTH/ { if (infilter==0) {infilter=1;definenum=0;} } \
  { if (infilter==1) {  \
      if (match($0,"#endif")) {\
         definenum--; \
         if (definenum==0) \
         infilter=0; \
      } \
      if (match($0,"#if")) {\
         definenum++; \
      } \
    } else \
     print $0; \
  }' $1 > /tmp/filtersource
mv -f /tmp/filtersource $1
