#!/bin/sh
#
# This converter uses Ross Paterson's utf7 package.
# (ftp://cnd.org/pub/software/unix/convert/utf7/)

BIG_TO_UTF8=/home/ygz/unicode/utf7/b5tou8
UTF8_TO_BIG=/home/ygz/unicode/utf7/u8tob5
UTF8_TO_UNICODE=/home/ygz/unicode/utf7/u8touni

echo "slow, be patient"

echo "big5 to unicode ..."

echo "" | awk '
BEGIN {					# output all BIG5
	for ( q = 161; q <= 249; q++ ) {	# 0xa1 -- 0xf9
		for ( w =  64; w <= 126; w++ ) { printf "%c%c", q, w; }
		for ( w = 161; w <= 254; w++ ) { printf "%c%c", q, w; }
	}
}' | $BIG_TO_UTF8 > mapBIGtoUNI.utf8
$UTF8_TO_UNICODE mapBIGtoUNI.utf8 mapBIGtoUNI.u
rm -f mapBIGtoUNI.utf8

echo "unicode to big5 ..."

echo "" | awk '	
BEGIN {					# output all Unicode UTF-8
	for ( i = 194 ; i <= 223; i++ )		# ASCII is UNDEF for BIG5
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
}' > mapUNItoBIG.utf8
$UTF8_TO_BIG < mapUNItoBIG.utf8 > mapUNItoBIG.b5
rm -f mapUNItoBIG.utf8

make	# generate the gen_ctab program

./gen_ctab mapBIGtoUNI.u  BtoU >  b2u_tables.c
./gen_ctab mapUNItoBIG.b5 UtoB >> b2u_tables.c
echo "A new \"b2u_tables.c\" is generated.  Replace the old one with this."
