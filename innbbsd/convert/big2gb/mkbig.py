#/usr/bin/env python

import mkunitab

mkunitab.make('CP950.TXT', 'big', [[0xa1, 0xff], [0x40, 0xff]], 0x25a1, 0xa1bc)

