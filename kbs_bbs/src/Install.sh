#! /bin/sh

BBS_HOME=/home/bbs
INSTALL="/usr/bin/install -c"
TARGET=/home/bbs/bin

echo "This script will install the whole BBS to ${BBS_HOME}..."
echo -n "Press <Enter> to continue ..."
read ans

if [ -d ${BBS_HOME} ] ; then
        echo -n "Warning: ${BBS_HOME} already exists, overwrite whole bbs [N]?"
        read ans
        ans=${ans:-N}
        case $ans in
            [Yy]) echo "Installing new bbs to ${BBS_HOME}" ;;
            *) echo "Abort ..." ; exit ;;
        esac
else
        echo "Making dir ${BBS_HOME}"
        mkdir ${BBS_HOME}
        chown -R bbs ${BBS_HOME}
        chgrp -R bbs ${BBS_HOME}
fi

echo "Setup bbs directory tree ....."
( cd bbshome ; tar cf - * ) | ( cd ${BBS_HOME} ; tar xf - )

chown -R bbs ${BBS_HOME}
chgrp -R bbs ${BBS_HOME}

${INSTALL} ${TARGET} -m770  -gbbs -obbs    bbs
${INSTALL} ${TARGET} -m770  -gbbs -obbs    bbs.chatd

cat > ${BBS_HOME}/etc/sysconf.ini << EOF
# comment

BBSHOME         = "/home/bbs"
BBSID           = "SMTH"
BBSNAME         = "SMTH BBS"
BBSDOMAIN       = "smth.edu.cn"

#SHOW_IDLE_TIME         = 1
KEEP_DELETED_HEADER     = 0

#BBSNTALKD      = 1
#NTALK          = "/bin/ctalk.sh"
#REJECTCALL     = "/bin/rejectcall.sh"
#GETPAGER       = "getpager"

BCACHE_SHMKEY   = 7813
UCACHE_SHMKEY   = 7912
UTMP_SHMKEY     = 3785
ACBOARD_SHMKEY  = 9013
ISSUE_SHMKEY    = 5002
GOODBYE_SHMKEY  = 5003

IDENTFILE       = "etc/preach"
EMAILFILE       = "etc/mailcheck"
#NEWREGFILE     = "etc/newregister"

PERM_BASIC      = 0x00001
PERM_CHAT       = 0x00002
PERM_PAGE       = 0x00004
PERM_POST       = 0x00008
PERM_LOGINOK    = 0x00010
PERM_DENYPOST   = 0x00020
PERM_CLOAK      = 0x00040
PERM_SEECLOAK   = 0x00080
PERM_XEMPT      = 0x00100
PERM_WELCOME    = 0x00200
PERM_BOARDS     = 0x00400
PERM_ACCOUNTS   = 0x00800
PERM_CHATCLOAK  = 0x01000
PERM_OVOTE      = 0x02000
PERM_SYSOP      = 0x04000
PERM_POSTMASK   = 0x08000
PERM_ANNOUNCE   = 0x10000
PERM_OBOARDS    = 0x20000
PERM_ACBOARD    = 0x40000
UNUSE1          = 0x80000
UNUSE2          = 0x100000
UNUSE3          = 0x200000
UNUSE4          = 0x400000
UNUSE5          = 0x800000
UNUSE6          = 0x1000000
UNUSE7          = 0x2000000
UNUSE8          = 0x4000000
UNUSE9          = 0x8000000
UNUSE10         = 0x10000000
UNUSE11         = 0x20000000

PERM_ADMENU    = PERM_ACCOUNTS , PERM_OVOTE , PERM_SYSOP,PERM_OBOARDS,PERM_WELCOME,PERM_ANNOUNCE
AUTOSET_PERM    = PERM_CHAT, PERM_PAGE, PERM_POST, PERM_LOGINOK

#include "etc/menu.ini"
EOF

cat > ${BBS_HOME}/etc/menu.ini << EOF
#---------------------------------------------------------------------
%S_MAIN






[34m"" -[1;34m _ ..[0m[36m-- -"[1;36m ''- - -.[36m _. ._. ._.[0m[1;34m. _ _. _[34m.- --" [36m'- --._ _ _.[1;36m '. ._ [34m _ .-.-[0m[34m.."[m
                                                                        
                                                        O                      
                 ,                                    o         ,/            
              .//                                   o         ///         
          ,.///;,   ,;/                                   ,,////.   ,/     [1;32m  )[m
         o;;;;;:::;///                              '.  o:::::::;;///    [1;32m ( ([m
        >;;...::::;\\\                                <::::::::;;\\\     [1;32m(  )[m
          ''\\\\\'" ''\                                 ''::::::' \\     [1;32m )([0;32m\[1m([m  
             '\\                                            '\\           [1;32m()[0;32m/[1;32m))[m
                                                               '\         [1;32m\([0;32m|[1m/[m
                                                                          [1;32m()[m[32m|[1m([m
                                                                           [1;32m([m[32m|[1m)[m
                                                                           [1;32m\[m[32m|[1m/[m
%

#---------------------------------------------------------------------
%menu TOPMENU
title        0, 0, "D¿ï³æ"
screen       3, 0, S_MAIN
!M_EGROUP    10, 28, 0,         "EGroup",    "E) ¤ÀÃþQ½×°Ï"
!M_MAIL      0, 0, 0,           "Mail",      "M) BzHºà
!M_TALK      0, 0, 0,           "Talk",      "T) WN¾ô    Talk  "
!M_INFO      0, 0, PERM_BASIC,  "Info",      "I) u¨ãc   Xyz-1  "
!M_SYSINFO   0, 0, 0,           "Config",    "C) t²Î¸êT Xyz-2  "
@LeaveBBS    0, 0, 0,           "GoodBye",   "G) »¡«ô«ô          "
@Announce    0, 0, 0,           "0Announce", "0) ºëµØ¤½GÄæ      "
!M_TIN       0, 0, 0,           "1TIN"    ,  "1) ¤å³¹J¾ã   TIN  "
#@ExecGopher  0, 0, 0,           "2Gopher",  "2) ¸ê®Æd¸ß Gopher "
!M_ExceMJ    0, 0, PERM_POST,   "Services",  "S) ºô¸ô¬ÛÃöA°ÈP Game"
#@SetHelp    0, 0, 0,           "Help",      "H) »²Ue­±]w    "
!M_ADMIN     0, 0, PERM_ADMENU,"Admin",      "A) t²ÎºÞz\¯àªí  "
%
#---------------------------------------------------------------------
%S_TIN



                                                                        
                                                                        
                                                                       
                                                                            
                            [1;33mÅýA¨É¨ü¸ê®ÆÂ×Iªº@¬É[m
                          zwwwwwwwwwww{
                          x¢«                  ¢ªx
                          x                      x
                          x                      x
                          x                      x
                          x                      x
                          x                      x
                          x¢©                  ¢¨x
                          |wwwwwwwwwww}
                                                                        
                                                                
                                                                
%

#---------------------------------------------------------------------
%menu M_TIN
title        0, 0, "Tin ¿ï³æ"
screen       3, 0, S_TIN
@ExecTin     14, 28, PERM_POST, "1TIN",       "1) ¤å³¹J¾ã   TIN  "
@EGroups     0, 0, 0,           "TINinBBS",   "T) §â Tin h¨ì BBS"
!..          0, 0, 0,           "Exit",       "E) ^¨ìD¿ï³æ"
%


#------------------------------------------------------------------
%S_EGROUP






                     zwwwwwwwwwwwwwwwww{
                     x                                  x
                     x                                  x
                     x                                  x
                     x                                  x
                     x                                  x
                     x                                  x
                     x                                  x
                     x                                  x
                     x                                  x
                     x                                  x
                     x                                  x
                     x                                  x
                     x                                  x
%
#------------------------------------------------------------------
%menu M_EGROUP
title        0, 0, "¤ÀÃþQ½×°Ï¿ï³æ"
screen       3, 0, S_EGROUP
@EGroups     10, 25, 0, "0BBS",      "0) BBS t²Î -- [¯¸¤º]"
@EGroups     0, 0, 0,   "1CCU",      "1) ¤¤¥¿j¾Ç -- [¥»®Õ]"
@EGroups     0, 0, 0,   "2Campus",   "2) ®Õ¶é¸êT -- [®Õ¶é] [¸êT]"
@EGroups     0, 0, 0,   "3Computer", "3) q¸£§ÞN -- [q¸£] [t²Î]"
@EGroups     0, 0, 0,   "4Rec",      "4) ¥ð¶¢T¼Ö -- [¥ð¶¢] [­µ¼Ö]"
@EGroups     0, 0, 0,   "5Art",      "5) ¤åÃÀ¾ÇN -- [¤åÃÀ] [ªÀ¥æ]"
#@EGroups    0, 0, 0,   "6Science",  "6) ¾ÇN¬ì¾Ç -- [¾Ç¬ì] [y¨¥]"
@EGroups     0, 0, 0,   "6Sports",   "6) Åé|°·¨­ -- [B°Ê] [Â¾´Î]"
@EGroups     0, 0, 0,   "7Talk",     "7) ½Í¤Ñ²áa -- [½Í¤Ñ] [sD]"
@EGroups     0, 0, 0,   "TINinBBS",  "T) §â Tin h¨ì BBS"
@PostArticle 0, 0, PERM_POST,   "Post",      "P) ¤å»¨´§µ§        "
@BoardsAll   0, 0, 0,   "Boards",    "B) ©Ò¦³Q½×°Ï"
@BoardsNew   0, 0, 0,   "New",       "N) \Åªs¤å³¹  New"
!..          0, 0, 0,   "Exit",      "E) ^¨ìD¿ï³æ"
%
EGROUP0 = "0"
EGROUP1 = "C"
EGROUP2 = "IE"
EGROUP3 = "NR"
EGROUP4 = "am"
EGROUP5 = "enij"
EGROUP6 = "rs"
EGROUP7 = "xf"
EGROUPT = "*"
#------------------------------------------------------------------
%S_MAIL






                          zwwwwwwwwwww{
                          x¢«                  ¢ªx
                          x                      x
                          x                      x
                          x                      x
                          x                      x
                          x                      x
                          x                      x
                          x                      x
                          x                      x
                          x                      x
                          x                      x
                          x¢©                  ¢¨x
                          |wwwwwwwwwww}
%

#------------------------------------------------------------------
%menu M_MAIL
title            0, 0, "BzHºà¿ï³æ"
screen           3, 0, S_MAIL
@ReadNewMail     11, 29, 0,             "NewMail", "N) Äý\sHºà"
@ReadMail        0, 0, 0,               "ReadMail","R) Äý\¥þ³¡Hºà"
@SendMail        0, 0, PERM_POST,       "SendMail","S) ¥áp¯È±ø"
@GroupSend       0, 0, PERM_POST,       "Gsend",   "G) HHµ¹@sH"
@SetFriends      0, 0, PERM_BASIC,      "Override","O)z]wn¤ÍW³æ"
@OverrideSend    0, 0, PERM_POST,       "Osend",   "O)|HHµ¹n¤ÍW³æ"
@SetMailList     0, 0, PERM_POST,       "Makelist","M)z]wHHW³æ   "
@ListSend        0, 0, PERM_POST,       "Lsend",   "L)|Hµ¹]wªºW³æ "
@SendNetMail     0, 0, PERM_POST,       "Internet","I) ­¸F¶Ç®Ñ"
!..              0, 0, 0,               "Exit",    "E) ^¨ìD¿ï³æ"
%
#------------------------------------------------------------------
%S_TALK






  ,-*~,-*~''~*-,._.,-*~''~*-,._.,-*~''~*-,._.,-*~''~*-,._.,-*~''~*-,'~*-,._.,
                                                
                                                
                                                
                                                
                                                
                                        
                                                
                                                
                                                
                                                
                                                
                                                        
  ,-*~,-*~''~*-,._.,-*~''~*-,._.,-*~''~*-,._.,-*~''~*-,._.,-*~''~*-,'~*-,._.,
%

#------------------------------------------------------------------
%menu M_TALK
title        0, 0, "WN¾ô¿ï³æ"
screen       3, 0,   S_TALK
@ShowFriends 10, 27,    PERM_BASIC, "Friends",  "F) ]±´n¤Í   Friend"
@ShowLogins  0, 0,      0,          "Users",    "U) ÀôU|¤è    Users"
@QueryUser   0, 0,      PERM_BASIC, "Query",    "Q) d¸ßºô¤Í    Query"
@Talk        0, 0,      PERM_POST,  "Talk",     "T) N¾ô²Óy     Talk"
@SetPager    0, 0,      PERM_BASIC, "Pager",    "P) Q@­ÓHRR Page"
@SendMsg     0, 0,      PERM_POST,  "SendMsg",  "S) eT®§µ¹OH"
@SetFriends  0, 0,      PERM_BASIC, "Override", "O) ]wn¤ÍW³æ"
@EnterChat   0, 0,      PERM_BASIC, "1Chat",    "1) ]¿ßl«È´Ì"
#@EnterChat   0, 0,      PERM_BASIC, "2Chat",    "2) ]¿ßl«È´Ì"
#@EnterChat   0, 0,      PERM_BASIC, "3Relay",   "3) §ÖvF    "
#@EnterChat   0, 0,      PERM_BASIC, "4Chat",    "4) ¦Ñj²á¤Ñ«Ç"
@ExecIrc     0, 0,      PERM_POST,  "IRC",      "I) P»Ú|½Í      IRC"
#@ListLogins 0, 0,      PERM_CHAT,  "List",     "L) W¯¸¨Ï¥ÎªÌÂ²ªí"
@Monitor     0, 0,      PERM_CHAT,  "Monitor",  "M) ±´µø¥Á±¡"
@RealLogins  0, 0,      PERM_SYSOP, "Nameofreal","N) W¯¸¨Ï¥ÎªÌu¹êmW"
!..          0, 0, 0,               "Exit",     "E) ^¨ìD¿ï³æ"
%

#------------------------------------------------------------------
%S_INFO






                          ùÝùùùùùùùùùùùùùùùùùùùùùùùùùß
                          ùø                        ùø
                          ùø                        ùø
                          ùø                        ùø
                          ùø                        ùø
                          ùø                        ùø
                          ùø                        ùø
                          ùø                        ùø
                          ùø                        ùø                    ,--,/
                          ùø                        ùòw¢¡    ~w¢¡_ ___/ /\|
                          ùø                        ùø  ¢¢ww¢£ ,:( )__,_)  ~
                          ùø                        ùø         //  //   L==;
                          ùãùùùùùùùùùùùùùùùùùùùùùùùùùå         '   \     | ^
 ,_.-*'"'*-._,_.-*'"'*-._,_.-*'"'*-._,_.-*'"'*-._,_.-*'"'*-._,_.-*'"'*-._,_.-*
%

#------------------------------------------------------------------
%menu M_INFO
title        0, 0, "u¨ãc¿ï³æ"
screen       3, 0, S_INFO
@FillForm    11, 29, 0,             "FillForm",  "F) µùU¸Ô²Ó­ÓH¸ê®Æ"
@SetInfo     0, 0,      PERM_BASIC, "Info",      "I) ]w­ÓH¸ê®Æ"
@UserDefine  0, 0,      PERM_BASIC, "Userdefine","U) ­ÓH°Ñ¼Æ]w"
#@OffLine     0, 0,      PERM_BASIC, "OffLine",   "O) ¦Û±þ~~~~~"
#@EditSig    0, 0,      PERM_POST,  "Signature", "S) ¨èL(Ã±WÀÉ) Sig."
#@EditPlan   0, 0,      PERM_POST,  "QueryEdit", "Q) ÀÀ­ÓH»¡©úÀÉ Plan"
@EditUFiles  0, 0,      PERM_POST,  "WriteFiles","W) s­×­ÓHÀÉ®×"
@SetCloak    0, 0,      PERM_SYSOP, "Cloak",     "C) Áô¨­N"
@SetHelp     0, 0,      PERM_BASIC, "Help",      "H) »²Ue­±]w"
!..          0, 0, 0,               "Exit",      "E) ^¨ìD¿ï³æ"
%

#------------------------------------------------------------------
%S_SYSINFO






         ¡´¡½¡½¡½¡½¡½¡½¡½¡½¡½¡½¡½¡½¡½¡½¡½¡½¡½¡½¡½¡½¡½¡½¡½¡½¡½¡½¡½¡½¡´
         ¡½                                                        ¡½
         ¡½                                                        ¡½
         ¡½                                                        ¡½
         ¡½                                                        ¡½
         ¡½                                                        ¡½
         ¡½                                                        ¡½
         ¡½                                                        ¡½
         ¡½                                                        ¡½
         ¡½                                                        ¡½
         ¡½                                                        ¡½
         ¡½                                                        ¡½
         ¡´¡½¡½¡½¡½¡½¡½¡½¡½¡½¡½¡½¡½¡½¡½¡½¡½¡½¡½¡½¡½¡½¡½¡½¡½¡½¡½¡½¡½¡´
                                                                        
%
#------------------------------------------------------------------
%menu M_SYSINFO
title        0, 0, "t²Î¸êT¿ï³æ"
screen       3, 0, S_SYSINFO
@ShowLicense 10, 28, 0,        "License",   "L) ¨Ï¥Î°õ·Ó      "
@ShowVersion 0, 0, 0,          "Copyright", "C) ´¼zv¸êT    "
@Notepad     0,0,0,            "Notepad",   "N) ¬Ý¬Ýd¨¥O"    
@ShowDate    0, 0, 0,          "Date",      "D) ¥Øe®É¨è     Date "
@DoVote      0, 0, PERM_BASIC, "Vote",      "V) ¤½¥Á§ë²¼     Vote "
@VoteResult  0, 0, PERM_BASIC, "Results",   "R) ¿ï±¡³ø¾É          "
@MailAll     0, 0, PERM_SYSOP, "MailAll",   "M) HHµ¹©Ò¦³H      "
@ExecBBSNet  0, 0, PERM_POST,  "BBSNet",    "B) ¬ï±ô»Èe(¦Ñ°¨ÃÑ~)"  
@ShowWelcome 0, 0, 0,          "Welcome",   "W) i¯¸e­±      "
@SpecialUser 0, 0, PERM_POST,  "Users",     "U) X®æ¤½¥ÁCªí  "
#@ExecViewer  0, 0, 0,         "0Announce", "0) ¥»¯¸¤½GÄæ (ÂÂª©t²Î)"
!..          0, 0, 0,          "Exit",      "E) ^¨ìD¿ï³æ"
%

#---------------------------------------------------------------------------
%S_ExceMJ






                     zwwwwwwwwwwwwwwwww{
                     x                                  x
                     x                                  x
                     x                                  x
                     x                                  x
                     x                                  x
                     x                                  x
                     x                                  x
                     x                                  x
                     x                                  x
                     x                                  x
                     x                                  x
                     x                                  x
 ,_.-*'"'*-._,_.-*'"'*-._,_.-*'"'*-._,_.-*'"'*-._,_.-*'"'*-._,_.-*'"'*-._,_.-*
%
#------------------------------------------------------------------
%menu M_ExceMJ
title        0, 0, "ºô¸ôGamej«÷L  "
screen       3, 0, S_ExceMJ
@WWW        12,29, 0,           "WWW",         "W) ¤år¼Ò¦¡ªº WWW"
@ExecGopher  0, 0, 0,           "Gopher",      "G) ¸ê®Æd¸ß Gopher "
@ExecMJ      0, 0, PERM_CLOAK, "Mujang",       "M) ºô¸ô³ÂN"
@ExecBIG2    0, 0,  0,          "Big2",        "B) ¦ÑG³Ìj  "
@ExecCHESS   0, 0,  0,          "Chess",       "C) M¾ÔH´Ñ  "
!..          0, 0,  0,          "Exit",        "E) ^¨ìD¿ï³æ"
%

#------------------------------------------------------------------
%S_ADMIN






                                                                        
                                                                             
                                                                             
                                                                        
                                                                        
                                                                        
                                                                        
                                                                        
                                                                        
                                                                        
                                                                        
                                                                        
                                                                        
                                                                        
%

#------------------------------------------------------------------
%menu M_ADMIN
title        0, 0, "t²Îºû@¿ï³æ"
screen       3, 0, S_ADMIN
@CheckForm   9, 28,PERM_SYSOP,  "Register",    "R) ]w¨Ï¥ÎªÌµùU¸ê®Æ"
@ModifyInfo  0, 0, PERM_SYSOP,  "Info",        "I) ­×§ï¨Ï¥ÎªÌ¸ê®Æ"
@ModifyLevel 0, 0, PERM_SYSOP,  "Level",       "L) §ó§ï¨Ï¥ÎªÌªºv­­"
@KickUser    0, 0, PERM_SYSOP,  "Kick",        "K) N¨Ï¥ÎªÌ½ðX¥»t²Î"
@DelUser     0, 0, PERM_ACCOUNTS,"DeleteUser", "D) ¬å±¼¨Ï¥ÎªÌb¸¹"
@OpenVote    0, 0, PERM_OVOTE,  "Vote",        "V) |¦æt²Î§ë²¼"
@NewBoard    0, 0, PERM_OBOARDS,"NewBoard",    "N) }±Ò@­ÓsªºQ½×°Ï"
@EditBoard   0, 0, PERM_ANNOUNCE,"ChangeBoard","C) ­×§ïQ½×°Ï»¡©úP]w
@DelBoard    0, 0, PERM_OBOARDS,"BoardDelete","B) ¬å±¼@­ÓL¥ÎªºQ½×°Ï"
@SetTrace    0, 0, PERM_SYSOP,  "Trace",       "T) ]wO_O¿ý°£¿ù¸êT"
@CleanMail   0, 0, PERM_SYSOP,  "MailClean",   "M) M°£©Ò¦³ÅªLªºpHH¥ó"
@Announceall 0, 0, PERM_SYSOP,  "Announceall", "A) ¹ï©Ò¦³Hs¼½"
@EditSFiles  0, 0, PERM_WELCOME,"Files",       "F) s¿èt²ÎÀÉ®×"
!..          0, 0, 0,           "Exit",        "E) ^¨ìD¿ï³æ"
%

#------------------------------------------------------------------


EOF

echo "Install is over...."
