#!/bin/bash
OUTPUTDIR="/home0/www/htdocs/bbsstat"
OUTPUTDIRSED="\/home0\/www\/htdocs\/bbsstat"

cd /home/bbs

nowmonth=`date '+%Y/%m/'`
if test ! -d "$OUTPUTDIR/$nowmonth"
then
	`mkdir -p $OUTPUTDIR/$nowmonth`
fi

nowfile=`date '+%Y/%m/%d_useronline.png'`
nowfilesed=`date '+%Y\/%m\/%d_useronline.png'`

sed -e "s/OUTPUTFILE/$OUTPUTDIRSED\/$nowfilesed/g" day.plt > todayonline.plt
gnuplot todayonline.plt
rm -f todayonline.plt
cp -f $OUTPUTDIR/$nowfile $OUTPUTDIR/todayonline.png
