#!/bin/sh
awk 'BEGIN { definenum=0;infilter=0; } \
  $0~/#ifdef[ \t]*SMTH/ { if (infilter==0) {infilter=1;definenum=0;output=0;} } \
  { if (infilter==1) {  \
      if (match($0,"#endif")) {\
         definenum--; \
         if (definenum==0) {\
         infilter=0; \
         output=0; }\
      } else {\
        if (match($0,"#if")) {\
          definenum++; \
          output=0; \
        } else {\
          if (match($0,"#else")) {\
           if (definenum==1) \
             output=1; \
           else  output=0; \
          } else {\
            if (output==1) print $0; \
          } \
        }\
      }\
    } else \
     print $0; \
  }' $1 > /tmp/filtersource
mv -f /tmp/filtersource $1
