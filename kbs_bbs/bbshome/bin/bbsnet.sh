#!/bin/sh
#
#	define the standout() and standend() escape sequences...
#
SO="[7m"
SE="[0m"
stty pass8

#biff n

while true
do
clear
echo "---------------------------------------------"
echo "          ${SO} ¥æ¤j¸ê¤u¨t BBS ªA°È¶µ¥Ø ${SE}"
echo "---------------------------------------------"
echo "
   ${SO}[0]${SE} ¥»¯¸¤½§GÄæ

   ${SO} ¹Ï®Ñ¬d¸ß¨t²Î ${SE}
   [1] ¥æ³q¤j¾Ç		[5] °ê¥ß¤¤¥¡¹Ï®ÑÀ] (°ª¤â¥Î¤§, ¦]¬°«Ü½ÆÂø)
   [2] ²MµØ¤j¾Ç  	[6] ¥x¥_¥«¥ß¹Ï®ÑÀ]
   [3] ¥xÆW¤j¾Ç         [7] ¤¤¥¡¬ã¨s°|              
   [4] ¦¨¥\¤j¾Ç              

   ${SO}[97]${SE} ³]©w 1Discuss ©Ò¥Î¤§½s¿è¾¹
   ${SO}[98]${SE} login ¤H¼Æ²Î­p¹Ïªí¾ú¥vÀÉ
   ${SO}[99]${SE} ¤µ¤é login ¤H¼Æ²Î­p¹Ïªí
    ${SO}[S]${SE} ±N 1Discuss ¤º¤§°Q½×°Ï±Æ§Ç

   ${SO}[Q]${SE} Exit.    
"
#   ${SO}[99]${SE} ¶i¯¸¦¸¼Æ±Æ¦æº] 

echo -n "½Ð¿ï¾Ü [Q]: " 

        if read CHOICE
           then
	    clear
            time_stamp=`sdate`
            case "${CHOICE}"
              in
                '')
                  break
                  ;;
                'Q')
                  break
                  ;;
		'q')
		  break
		  ;;
                0)
		  LC_CTYPE=iso_8859_1
		  export LC_CTYPE
		  echo "$time_stamp faqview $USER" >> bbslog
		  cd 0Announce
		  faqview '    ¥æ³q¤j¾Ç¸ê°T¤uµ{¾Ç¨t BBS ¤½§GÄæ (perl ª©)'
		  cd ..
                  ;;
                1)
                  echo "$time_stamp bbsnet $USER ¥æ¤j¹Ï®ÑÀ]" >> bbslog
                  echo "${SO} ³s©¹¥æ¤j¹Ï®ÑÀ] ${SE}"
                  rlogin lib1.nctu.edu.tw -l library
                  #expect /expect/nctu_lib
                  ;;
                2)
                  echo "$time_stamp bbsnet $USER ²M¤j¹Ï®ÑÀ]" >> bbslog
                  echo "${SO} ³s©¹²M¤j¹Ï®ÑÀ] ${SE}"
                  echo "½Ð¥Î ${SO}search${SE} login"
                  telnet 140.114.72.2
                  #expect /expect/nthu_lib
                  ;;
                3)
                  echo "$time_stamp bbsnet $USER ¥x¤j¹Ï®ÑÀ]" >> bbslog
                  echo "${SO} ³s©¹¥x¤j¹Ï®ÑÀ] ${SE}"
                  echo "½Ð¥Î ${SO}library${SE} login"
                  #rlogin asleep.ntu.edu.tw -l reader
                  telnet 140.112.196.20 
                  ;;
                4)
                  echo "$time_stamp bbsnet $USER ¦¨¤j¹Ï®ÑÀ]" >> bbslog
                  echo "${SO} ³s©¹¦¨¤j¹Ï®ÑÀ] ${SE}"
                  echo "½Ð¥Î ${SO}OPAC${SE} login"
                  telnet 140.116.207.1
                  #expect /expect/ncku_lib
                  ;;
                5)
                  echo "$time_stamp bbsnet $USER ¤¤¥¡¹Ï®ÑÀ]" >> bbslog
                  echo "${SO} ³s©¹°ê¥ß¤¤¥¡¹Ï®ÑÀ] ${SE}"
                  echo "\
¦pªG­n°h¥X¤¤¥¡¹Ï®ÑÀ]ªº¨t²Î, ½Ð¦b¡uÀË¯ÁÁä¡v´£¥Ü²Å¸¹¤UÁä¤J ${SO}.X${SE} 
µM«á¦b ¡ucommand¡v ´£¥Ü²Å¸¹¤UÁä¤J ${SO} logoff ${SE}"
                  echo -n "½Ð«ö ENTER Ä~Äò"
                  read junk
                  #telnet 192.83.186.1
                  expect -f /expect/clib
                  ;;
                6)
                  echo "$time_stamp bbsnet $USER ¥x¥_¥«¥ß¹Ï®ÑÀ]" >> bbslog
                  echo "${SO} ³s©¹¥x¥_¥«¥ß¹Ï®ÑÀ] ${SE}"
                  echo "½Ð¥Î ${SO}opacnet${SE} login"
                  #telnet 192.83.187.1
                  expect -f /expect/tmlib
                  ;;
                7)
                  echo "$time_stamp bbsnet $USER ¤¤¬ã°|¹Ï®ÑÀ]" >> bbslog
                  echo "${SO} ³s©¹¤¤¬ã°|¹Ï®ÑÀ] ${SE}"
                  rlogin las.sinica.edu.tw -l chinese
                  ;;
		97) echo "          >>>   ½s¿è¾¹³]©wµe­±   <<<     " 
                    echo "============================================="
                    echo "     (1) ${SO}ve${SE}   ---- BBS ¤º©Ò¥Îªº½s¿è¾¹"
		    echo "     (2) ${SO}cvi${SE}  ---- vi"
                    echo "     (3) ${SO}cjoe${SE} ---- joe"
                    echo "============================================="
                    echo -n "½Ð¿ï¾Ü 1 - 3 ªº¼Æ¥Ø (½Ð­@¤ßµ¥­Ô) ==> " 
                    if read EDIT 
	            then
	                qEDIT=`cat /home/$USER/editor`
                        case ${EDIT} in
                        1) qEDIT="/bin/ve" ;;
                        2) qEDIT="/bin/cvi" ;;
                        3) qEDIT="/bin/cjoe" ;;
                        *) echo  "¥u¯à¿ï¾Ü 1 - 3 ªº¼Æ¥Ø"  ;;
                        esac

                        echo  "¥Ø«e¨Ï¥Î¤§½s¿è¾¹¬° ${SO} $qEDIT ${SE}"
                    fi

		    echo $qEDIT > /home/$USER/editor
                    echo -n "½Ð«ö <enter> Ä~Äò" 
                    read junk
                    echo "$time_stamp bbsnet $USER set_editor" >> bbslog
		    ;;
                
                98) echo -n "¤ë¥÷ (1..12) ==>"
		    read mon
		    echo -n "¤é´Á (1..30) ==>"
		    read day
		    rmore stat/login.$mon.$day 
                    echo "$time_stamp bbsnet $USER ²Î­pªí" >> bbslog
                    echo -n "½Ð«ö ${SO}ENTER${SE} Ä~Äò "
                    read junk  
                  ;;

                99) rmore stat/login 
                    echo "$time_stamp bbsnet $USER ²Î­pªí" >> bbslog
                    echo -n "½Ð«ö ${SO}ENTER${SE} Ä~Äò¬d¸ß¬Q¤é²Î­pªí "
                    read junk  
		    clear
		    rmore stat/login.yesterday
                    echo -n "½Ð«ö ${SO}ENTER${SE} Ä~Äò "
                    read junk  
                  ;;
		s|S) 
		    if [ -f home/$USER/.newsrc ] ; then
			cp home/$USER/.newsrc tmp/$USER
			sort tmp/$USER > home/$USER/.newsrc
			rm tmp/$USER
		    fi
		    ;;
                *)
          	  echo ""
		  echo "${SO} ¬Ý¤£À´±z¿ïªº ${CHOICE}, ¦A¸Õ¤@¦¸§a .${SE}"
		  read junk
		  ;;
            esac
          else
            exit
        fi
done

clear

