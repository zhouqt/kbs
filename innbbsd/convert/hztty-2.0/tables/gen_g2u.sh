#!/bin/sh
#
# This converter uses Ross Paterson's utf7 package.
# (ftp://cnd.org/pub/software/unix/convert/utf7/)

GB_TO_UTF8=/home/ygz/unicode/utf7/gbtou8
UTF8_TO_GB=/home/ygz/unicode/utf7/u8togb
UTF8_TO_UNICODE=/home/ygz/unicode/utf7/u8touni

echo "slow, be patient"

echo "gb to unicode ..."

echo "" | awk '
BEGIN {					# output all GB
	for ( q = 161; q <= 169; q++ ) {	# 0xa1 -- 0xa9
		for ( w = 161; w <= 254; w++ ) { printf "%c%c", q, w; }
	}
	for ( q = 176; q <= 247; q++ ) {	# 0xb0 -- 0xf7
		for ( w = 161; w <= 254; w++ ) { printf "%c%c", q, w; }
	}
}' | $GB_TO_UTF8 > mapGBtoUNI.utf8
$UTF8_TO_UNICODE mapGBtoUNI.utf8 mapGBtoUNI.u
rm -f mapGBtoUNI.utf8

echo "unicode to gb ..."

echo "" | awk '	
BEGIN {					# output all Unicode UTF-8
	for ( i = 194 ; i <= 223; i++ )		# ASCII is UNDEF for GB
		printf ("%c%c%c", 239, 191, 189);
						# start from 0x0080
	for ( i = 194 ; i <= 223; i++ )		# 11000010 - 11011111
	 for ( j = 128 ; j <= 191; j++ ) 	#  10000000 - 10111111
		printf ("%c%c", i, j);
						# start from 0x0800
						# 11100000
	 for ( j = 160 ; j <= 191; j++ ) 	#  10100000 - 10111111
	  for ( k = 128 ; k <= 191; k++ ) 	#   10000000 - 10111111
		printf ("%c%c%c", 224, j, k);
	for ( i = 225 ; i <= 239; i++ )		# 11100001 - 11101111
	 for ( j = 128 ; j <= 191; j++ ) 	#  10000000 - 10111111
	  for ( k = 128 ; k <= 191; k++ ) 	#   10000000 - 10111111
		printf ("%c%c%c", i, j, k);
}' > mapUNItoGB.utf8
$UTF8_TO_GB < mapUNItoGB.utf8 > mapUNItoGB.gb
rm -f mapUNItoGB.utf8

make	# generate the gen_ctab program

./gen_ctab mapGBtoUNI.u  GtoU >  g2u_tables.c
./gen_ctab mapUNItoGB.gb UtoG >> g2u_tables.c
echo "A new \"g2u_tables.c\" is generated.  Replace the old one with this."
