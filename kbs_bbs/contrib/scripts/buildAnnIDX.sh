#!/bin/bash
#
# Written by period, 2000-11-22
# update 0Announce index of each board
# if given no param, check boards according to date
# otherwise check the given board
#
LANG="zh_CN"
LC_CTYPE="zh_CN"
LC_NUMERIC="zh_CN"
LC_TIME="zh_CN"
export LANG LC_CTYPE LC_NUMERIC LC_TIME

annbase="0Announce/groups"
if [ "x$1" = "x" ]; then
    dayofweek=`date +"%w"`
else
    dayofweek=$1
fi
case "$dayofweek" in
    1)
        annptrn="$annbase/campus.faq/* $annbase/comp.faq/*"
        ;;
    2)
        annptrn="$annbase/literal.faq/[a-dA-Df-zF-Z]*"
        ;;
    3)
        annptrn="$annbase/literal.faq/[eE]*"
        ;;
    4)
        annptrn="$annbase/rec.faq/*"
        ;;
    5)
        annptrn="$annbase/s*.faq/*"
        ;;
    6)
        annptrn="$annbase/[t-zT-Z]*.faq/*"
        ;;
    7|0)
        annptrn="$annbase/_NOT_EXIST_/"
        echo > $annbase/buildidx.log
        ;;
    *)
        annptrn="$annbase/$dayofweek"
        ;;
esac
echo "===Start at `date`===" >> $annbase/buildidx.log
date >> "$annbase/announce.index/announce.newupdate"
for annchkdir in $annptrn; do
    if [ -d "$annchkdir" ]; then
        echo "$annchkdir" >> $annbase/buildidx.log
        annidxdir="$annchkdir/announce.index"
        [ -d "$annidxdir" ] || ( mkdir -p "$annidxdir"; \
		echo "__rebuild__ $annidxdir" >> "$annbase/buildidx.log" )
        [ -d "$annidxdir" ] && ( cp -f "$annbase/announce.index/.Names" "$annidxdir"; \
		cp -f "$annbase/announce.index/Note" "$annidxdir" )
        chknames=`grep "^Path=~/announce.index$" "$annchkdir/.Names"|wc -l`
        if [ $chknames = 0 ]; then
            chknames=`grep "^Numb=" "$annchkdir/.Names"|wc -l`
            chknumb=`echo "$chknames + 1"|bc`
            echo "Name=¾«»ªÇøÄ¿Â¼½á¹¹ÁÐ±í                    (BM: BMS)" >> "$annchkdir/.Names"
            echo "Path=~/announce.index" >> "$annchkdir/.Names"
            echo "Numb=$chknumb" >> "$annchkdir/.Names"
            echo "#" >> "$annchkdir/.Names"
        fi
        date >> "$annidxdir/announce.error"
	echo "[1;42;37m  ÐòºÅ          ¾«»ªÇøÖ÷Ìâ                                            ¸üÐÂÈÕÆÚ [m" > "$annidxdir/announce.structure"
	echo "-------------------------------------------------------------------------------" >> "$annidxdir/announce.structure"
        bin/idx0Ann "$annchkdir" -n -l "$annidxdir/announce.error" | \
            tee "$annidxdir/announce.list" | \
            grep "\[\[1;32mÄ¿Â¼\[37m\]" >> "$annidxdir/announce.structure"
        newul=`cat "$annidxdir/announce.list" | \
            grep -v "                \[37m$" | \
            tee "$annidxdir/announce.newupdate" | wc -l | bc`
        if [ $newul -lt 4 ]; then
            brdname=`basename $annchkdir`
            echo "$brdname°æÒÑ¾­ºÜ¾ÃÃ»ÓÐ¸üÐÂ¹ý¾«»ªÇøÁË£¨´Ë´¦ ºÜ¾Ã£½7Ìì£©£¬ÇëÕÒ°æÖ÷Ñ¯ÎÊ" \
                >> "$annbase/announce.index/announce.newupdate"
        fi
    fi
done
echo "===Stop  at `date`===" >> $annbase/buildidx.log
