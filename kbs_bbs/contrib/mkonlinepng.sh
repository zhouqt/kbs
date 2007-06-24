#!/bin/bash
OUTPUTDIR="/home0/www/htdocs/bbsstat"
OUTPUTDIRSED="\/home0\/www\/htdocs\/bbsstat"
LOGDIR="bonlinelog"

cd /home/bbs

nowyear=`date '+%Y'`
nowmonth=`date '+%m'`
nowday=`date '+%d'`
#declare -i intmonth=$nowmonth
#declare -i intday=$nowday
#shortmonth=`echo $intmonth`
#shortday=`echo $intday`
shortday=`expr 0 + $nowday`
shortmonth=`expr 0 + $nowmonth`

if test ! -d "$OUTPUTDIR/$nowyear/$nowmonth"
then
	`mkdir -p $OUTPUTDIR/$nowyear/$nowmonth`
fi

cp -f $LOGDIR/$nowyear/$shortmonth/${shortday}_useronline dayonline

nowfile="$nowyear/$nowmonth/${nowday}_useronline.png"
nowfilesed="$nowyear\/$nowmonth\/${nowday}_useronline.png"

sed -e "s/OUTPUTFILE/$OUTPUTDIRSED\/$nowfilesed/g" day.plt > todayonline.plt
gnuplot todayonline.plt
rm -f todayonline.plt
cp -f $OUTPUTDIR/$nowfile $OUTPUTDIR/todayonline.png
