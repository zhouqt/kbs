#!/bin/sh
#
# The BIGTOGB/GB2BIG must convert between Big5 (ET) and GB.

GBTOBIG=g2b
BIGTOGB=b2g

echo "slow, be patient"

echo "" | awk '
BEGIN {					# output all GB
	for ( q = 161; q <= 169; q++ ) {	# 0xa1 -- 0xa9
		for ( w = 161; w <= 254; w++ ) { printf "%c%c\n", q, w; }
	}
	for ( q = 176; q <= 247; q++ ) {	# 0xb0 -- 0xf7
		for ( w = 161; w <= 254; w++ ) { printf "%c%c\n", q, w; }
	}
}' | $GBTOBIG > mapGBtoBIG.b5

echo "" | awk '
BEGIN {					# output all Big5
	for ( q = 161; q <= 249; q++ ) {	# 0xa1 -- 0xf9
		for ( w =  64; w <= 126; w++ ) { printf "%c%c\n", q, w; }
		for ( w = 161; w <= 254; w++ ) { printf "%c%c\n", q, w; }
	}
}' | $BIGTOGB > mapBIGtoGB.gb

make	# generate the gen_ctab program

./gen_ctab -l mapGBtoBIG.b5 GtoB >  b2g_tables.c
./gen_ctab -l mapBIGtoGB.gb BtoG >> b2g_tables.c
echo "A new \"b2g_tables.c\" is generated.  Replace the old one with this."
