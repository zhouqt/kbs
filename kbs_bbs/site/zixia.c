#include "bbs.h"

#ifndef REGISTER_TSINGHUA_WAIT_TIME
#define REGISTER_TSINGHUA_WAIT_TIME (24*60*60)
#endif

const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

const char seccode[SECNUM][5] = {
	"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"
};

const char *permstrings[] = {
        "»ù±¾È¨Á¦",             /* PERM_BASIC */
        "½øÈëÁÄÌìÊÒ",           /* PERM_CHAT */
        "ºô½ĞËûÈËÁÄÌì",         /* PERM_PAGE */
        "·¢±íÎÄÕÂ",             /* PERM_POST */
        "Ê¹ÓÃÕß×ÊÁÏÕıÈ·",       /* PERM_LOGINOK */
        //"½ûÖ¹·¢±íÎÄÕÂ",         /* PERM_DENYPOST */
        "Õ¾Îñ½û",         /* PERM_BMMANAGER */
        "¿ÉÒşÉí",               /* PERM_CLOAK */
        "¿É¼ûÒşÉí",             /* PERM_SEECLOAK */
        "³¤ÆÚÕÊºÅ",         /* PERM_XEMPT */
        "±à¼­ÏµÍ³µµ°¸",         /* PERM_WELCOME */
        "°åÖ÷",                 /* PERM_BOARDS */
        "ÕÊºÅ¹ÜÀíÔ±",           /* PERM_ACCOUNTS */
        NAME_BBS_CHINESE "ÖÇÄÒÍÅ",       /* PERM_CHATCLOAK */
        //"Í¶Æ±¹ÜÀíÔ±",           /* PERM_OVOTE */
        "·â½ûÓéÀÖÈ¨ÏŞ",           /* PERM_DENYRELAX */
        "ÏµÍ³Î¬»¤¹ÜÀíÔ±",       /* PERM_SYSOP */
        "Read/Post ÏŞÖÆ",       /* PERM_POSTMASK */
        "¾«»ªÇø×Ü¹Ü",           /* PERM_ANNOUNCE*/
        "ÌÖÂÛÇø×Ü¹Ü",           /* PERM_OBOARDS*/
        "»î¶¯¿´°æ×Ü¹Ü",         /* PERM_ACBOARD*/
        "²»ÄÜ ZAP(ÌÖÂÛÇø×¨ÓÃ)", /* PERM_NOZAP*/
        "ÁÄÌìÊÒOP(ÔªÀÏÔº×¨ÓÃ)", /* PERM_CHATOP */
        "ÏµÍ³×Ü¹ÜÀíÔ±",         /* PERM_ADMIN */
        "»ú73°æ",          	/* PERM_HORNOR*/
        "¿´ÃØÃÜ¾«»ªÇø",         /* PERM_SECANC*/
        "¿´AKA°æ",           /* PERM_JURY*/
        "¿´Sexy°æ",           /* PERM_SEXY*/
        "×ÔÉ±½øĞĞÖĞ",           /* PERM_SUICIDE?*/
        "¹ÛÒô½ã½ã",           /* PERM_MM*/
        "¿´ÏµÍ³ÌÖÂÛ°æ",           /* PERM_DISS*/
        "·â½ûMail",           /* PERM_DENYMAIL*/
};

/* You might want to put more descriptive strings for SPECIAL1 and SPECIAL2
   depending on how/if you use them. */
const char *user_definestr[] = {
    "»î¶¯¿´°æ",                 /* DEF_ACBOARD */
    "Ê¹ÓÃ²ÊÉ«",                 /* DEF_COLOR */
    "±à¼­Ê±ÏÔÊ¾×´Ì¬À¸",         /* DEF_EDITMSG */
    "·ÖÀàÌÖÂÛÇøÒÔ New ÏÔÊ¾",    /* DEF_NEWPOST */
    "Ñ¡µ¥µÄÑ¶Ï¢À¸",             /* DEF_ENDLINE */
    "ÉÏÕ¾Ê±ÏÔÊ¾ºÃÓÑÃûµ¥",       /* DEF_LOGFRIEND */
    "ÈÃºÃÓÑºô½Ğ",               /* DEF_FRIENDCALL */
    "Ê¹ÓÃ×Ô¼ºµÄÀëÕ¾»­Ãæ",       /* DEF_LOGOUT */
    "½øÕ¾Ê±ÏÔÊ¾±¸ÍüÂ¼",         /* DEF_INNOTE */
    "ÀëÕ¾Ê±ÏÔÊ¾±¸ÍüÂ¼",         /* DEF_OUTNOTE */
    "Ñ¶Ï¢À¸Ä£Ê½£ººô½ĞÆ÷/ÈËÊı",  /* DEF_NOTMSGFRIEND */
    "²Ëµ¥Ä£Ê½Ñ¡Ôñ£ºÒ»°ã/¾«¼ò",  /* DEF_NORMALSCR */
    "ÔÄ¶ÁÎÄÕÂÊÇ·ñÊ¹ÓÃÈÆ¾íÑ¡Ôñ", /* DEF_CIRCLE */
    "ÔÄ¶ÁÎÄÕÂÓÎ±êÍ£ì¶µÚÒ»ÆªÎ´¶Á",       /* DEF_FIRSTNEW */
    "ÆÁÄ»±êÌâÉ«²Ê£ºÒ»°ã/±ä»»",  /* DEF_TITLECOLOR */
    "½ÓÊÜËùÓĞÈËµÄÑ¶Ï¢",         /* DEF_ALLMSG */
    "½ÓÊÜºÃÓÑµÄÑ¶Ï¢",           /* DEF_FRIENDMSG */
    "ÊÕµ½Ñ¶Ï¢·¢³öÉùÒô",         /* DEF_SOUNDMSG */
    "ÀëÕ¾áá¼Ä»ØËùÓĞÑ¶Ï¢",       /* DEF_MAILMSG */
    "·¢ÎÄÕÂÊ±ÊµÊ±ÏÔÊ¾Ñ¶Ï¢",     /*"ËùÓĞºÃÓÑÉÏÕ¾¾ùÍ¨Öª",    DEF_LOGININFORM */
    "²Ëµ¥ÉÏÏÔÊ¾°ïÖúĞÅÏ¢",       /* DEF_SHOWSCREEN */
    "½øÕ¾Ê±ÏÔÊ¾Ê®´óĞÂÎÅ",       /* DEF_SHOWHOT */
    "½øÕ¾Ê±¹Û¿´ÁôÑÔ°æ",         /* DEF_NOTEPAD */
    "ºöÂÔÑ¶Ï¢¹¦ÄÜ¼ü: Enter/Esc",        /* DEF_IGNOREMSG */
    "Ê¹ÓÃ¸ßÁÁ½çÃæ",                   /* DEF_HIGHCOLOR */
    "½øÕ¾Ê±¹Û¿´ÉÏÕ¾ÈËÊıÍ³¼ÆÍ¼", /* DEF_SHOWSTATISTIC Haohmaru 98.09.24 */
    "Î´¶Á±ê¼ÇÊ¹ÓÃ *",           /* DEF_UNREADMARK Luzi 99.01.12 */
    "Ê¹ÓÃGBÂëÔÄ¶Á",             /* DEF_USEGB KCN 99.09.03 */
    "¶Ôºº×Ö½øĞĞÕû×Ö´¦Àí",  /* DEF_CHCHAR 2002.9.1 */
		    "ÏÔÊ¾ÏêÏ¸ÓÃ»§Êı¾İ",  /*DEF_SHOWDETAILUSERDATA 2003.7.31 */
	    "ÏÔÊ¾ÕæÊµÓÃ»§Êı¾İ" /*DEF_REALDETAILUSERDATA 2003.7.31 */
};

const char    *explain[] = {
	"¸« Í· °ï",
	"´ó»°Î÷ÓÎ",
	"´óĞ¡·Ö¶æ",
        "³ÔºÈÍæÀÖ",
	"ßóßóÍáÍá",
	"Ò÷·çÅªÔÂ",
        "Î÷ÓÎÖ®Â·",
        "¶¹¸¯Î÷Ê©",
	"¶«³ÉÎ÷¾Í",
        "´óÊ¥È¡¾­",
         NULL
};
 
const char    *groups[] = {
	"AxFaction",
	"zixia",
	"Factions",
	"Entertain",
	"Watering",
	"Poem",
	"GoWest",
	"DouFuGirl",
  	"PandoraBox",
  	"Reserve",
         NULL
};

const char secname[SECNUM][2][20]={
	"¸« Í· °ï", "[ºÚ°ï/ÏµÍ³]",
	"´ó»°Î÷ÓÎ", "[´ó»°/¼¼Êõ]",
	"´óĞ¡·Ö¶æ", "[µØÅÌ/Ğ£°à]",
	"³ÔºÈÍæÀÖ", "[ÓéÀÖ/ÌåÓı]",
	"ßóßóÍáÍá", "[¸ĞĞÔ/ÁÄÌì]",
	"Ò÷·çÅªÔÂ", "[ĞÄÇé/ÎÄ»¯]",
	"Î÷ÓÎÖ®Â·", "[³ö¹ú/¾Û»á]",
	"¶¹¸¯Î÷Ê©", "[µêÃæ/µ±ÆÌ]",
	"¶«³ÉÎ÷¾Í", "[Ê¡·İ/³ÇÊĞ]",
	"´óÊ¥È¡¾­", "[×¨Òµ/ÉêÇë]",
};

const char *mailbox_prop_str[] =
{
	    "·¢ĞÅÊ±±£´æĞÅ¼şµ½·¢¼şÏä",
	    "É¾³ıĞÅ¼şÊ±²»±£´æµ½À¬»øÏä",
	    "°æÃæ°´ 'v' Ê±½øÈë: ÊÕ¼şÏä(OFF) / ĞÅÏäÖ÷½çÃæ(ON)",
};

struct _shmkey
{
	char key[20];
	int value;
};

static const struct _shmkey shmkeys[]= {
{ "BCACHE_SHMKEY",  3693 },
{ "UCACHE_SHMKEY",  3696 },
{ "UTMP_SHMKEY",    3699 },
{ "ACBOARD_SHMKEY", 9013 },
{ "ISSUE_SHMKEY",   5010 },
{ "GOODBYE_SHMKEY", 5020 },
{ "PASSWDCACHE_SHMKEY", 3697 },
{ "STAT_SHMKEY",    5100 },
{ "CONVTABLE_SHMKEY",    5101 },
{ "MSG_SHMKEY",    5200 },
{    "",   0 }
};

int get_shmkey(char *s)
{
	int n=0;
	while(shmkeys[n].key!=0)
	{
		if(!strcasecmp(shmkeys[n].key, s))
			return shmkeys[n].value;
		n++;
	}
	return 0;
}

int
uleveltochar( char* buf, struct userec *lookupuser )
{
	unsigned lvl;
	char userid[IDLEN+2];
	
	lvl = lookupuser->userlevel;
	strncpy( userid, lookupuser->userid, IDLEN+2 );

	// buf[10], buf ×î¶à 4 ¸öºº×Ö + 1 byte £¨\0½áÎ²£©
	//¸ù¾İ level
	//
	if( !( lvl | PERM_POST ) ) strcpy( buf, "ÑÆ°Í" ); 
      	else if ( ! (lvl & PERM_BASIC) ) strcpy( buf, "Ï¹×Ó" ); 
	else if( lvl < PERM_DEFAULT ) strcpy( buf, "ÊéÉú" );
    	//else if( lvl & PERM_SYSOP ) strcpy(buf,"°ïÖ÷");
      	else if( lvl & PERM_MM && lvl & PERM_CHATCLOAK ) strcpy(buf,"¾§¾§¹ÃÄï");
      	else if( lvl & PERM_MM ) strcpy(buf,"¹ÛÒôÃÀÃ¼");
      	else  if( lvl & PERM_CHATCLOAK ) strcpy(buf,"ÎŞµĞÅ£Ê­");
      	else if  ( lvl & PERM_BOARDS ) strcpy(buf,"¶şµ±¼ÒµÄ");
      	else strcpy(buf, NAME_USER_SHORT);

	//ÖĞÎÄËµÃ÷£¬¸ù¾İ level
    	if( !strcmp(lookupuser->userid,"SYSOP")
		    || !strcmp(lookupuser->userid,"Rama") )
	    strcpy( buf, "Ç¿µÁÍ·" );
    	else if( !strcmp(lookupuser->userid,"netterm") )
	    strcpy( buf, "ÀÏ°ïÖ÷" );
    	else if( !strcmp(lookupuser->userid,"zixia") )
	    strcpy( buf, "Íú²Æ" );
    	else if( !strcmp(lookupuser->userid,"windtear") )
	    	strcpy( buf, "Çå»ª×Ü´ú" );
    	else if( !strcmp(lookupuser->userid,"click") )
	    	strcpy( buf, "Ğ¡°×ÁúÂí" );
    	else if( !strcmp(lookupuser->userid,"wuhu") )
	    	strcpy( buf, "°Ë½ä" );
    	else if( !strcmp(lookupuser->userid,"halen") )
	    strcpy( buf, "Ğ¡Æ¤¿¨Çï" );
    	else if (!strcmp(lookupuser->userid,"cclu") ||
		!strcmp(lookupuser->userid,"SuperMM") ||
		!strcmp(lookupuser->userid,"SilverSnow") ||
		!strcmp(lookupuser->userid, "busygirl") )
	    strcpy( buf, "¹ÛÒô½ã½ã" );
    	else if( !strcmp(lookupuser->userid,"Bison") )
	    strcpy( buf, "ÒùÔô" );
    	else if( !strcmp(lookupuser->userid,"CIE") )
	    strcpy( buf, "¸£²¼Ë¹" );
    	else if( !strcmp(lookupuser->userid,"Roy") )
	    strcpy( buf, "´ó±ı" );
    	else if( !strcmp(lookupuser->userid,"dwd") )
	    strcpy( buf, "¶àÎÄÌìÍõ" );
    	else if( !strcmp(lookupuser->userid,"birby") )
	    strcpy( buf, "³Ö¹úÌìÍõ" );
    	else if( !strcmp(lookupuser->userid,"KCN") )
	    strcpy( buf, "ÉÏµÛ" );
        else if( !strcmp(lookupuser->userid,"bad") )
        strcpy( buf, "ÌÆÉ®" );
    	else if( !strcmp(lookupuser->userid,"cityhunter") 
		    || !strcmp(lookupuser->userid,"soso")
		    || !strcmp(lookupuser->userid,"Czz")
		    || !strcmp(lookupuser->userid,"flyriver") )
	    strcpy( buf, "Å£Ä§Íõ" );
    	else if( !strcmp(lookupuser->userid,"guest") )
	    strcpy( buf, "ÆÏÌÑ" );

    	return 1;
}

#include "modes.h"

char *
ModeType(mode)
int     mode;
{
    switch(mode) {
    case IDLE:      return "^@^zz..ZZ" ;
    case NEW:       return "ĞÂ" NAME_USER_SHORT "×¢²á" ;
    case LOGIN:     return "½øÈë" NAME_BBS_NICK;
    case CSIE_ANNOUNCE:     return "¼³È¡¾«»ª";
    case CSIE_TIN:          return "Ê¹ÓÃTIN";
    case CSIE_GOPHER:       return "Ê¹ÓÃGopher";
    case MMENU:     return "Ö÷²Ëµ¥";
    case ADMIN:     return NAME_SYS_MANAGE;
    case SELECT:    return "Ñ¡ÔñÌÖÂÛÇø";
    case READBRD:   return "ä¯ÀÀÌÖÂÛÇø";
    case READNEW:   return "²ì¿´ĞÂÎÄÕÂ";
    case  READING:  return "Éó²éÎÄÕÂ";
    case  POSTING:  return "ÅúÔÄÎÄÕÂ" ;
    case MAIL:      return "ĞÅ¼şÑ¡µ¥" ;
    case  SMAIL:    return "¼ÄĞÅÖĞ";
    case  RMAIL:    return "¶ÁĞÅÖĞ";
    case TMENU:     return "Ì¸ÌìËµµØÇø";
    case  LUSERS:   return "¿´Ë­ÔÚÏßÉÏ";
    case  FRIEND:   return "ÕÒÏßÉÏºÃÓÑ";
    case  MONITOR:  return "¼à¿´ÖĞ";
    case  QUERY:    return "²éÑ¯" NAME_USER_SHORT;
    case  TALK:     return "ÁÄÌì" ;
    case  PAGE:     return "ºô½Ğ" NAME_USER_SHORT;
    case  CHAT2:    return "ÃÎ»Ã¹ú¶È";
    case  CHAT1:    return CHAT_SERVER "ÖĞ";
    case  CHAT3:    return "¿ìÔÕÍ¤";
    case  CHAT4:    return "ÀÏ´óÁÄÌìÊÒ";
    case  IRCCHAT:  return "»áÌ¸IRC";
    case LAUSERS:   return "Ì½ÊÓ" NAME_USER_SHORT;
    case XMENU:     return "ÏµÍ³×ÊÑ¶";
    case  VOTING:   return "Í¶Æ±";
    case  BBSNET:   return "²¨Èô²¨ÂŞÃÛ";
    case  EDITWELC: return "±à¼­ Welc";
    case EDITUFILE: return "±à¼­µµ°¸";
    case EDITSFILE: return NAME_SYS_MANAGE;
    case  EDITSIG:  return "¿ÌÓ¡";
    case  EDITPLAN: return "Äâ¼Æ»­";
    case ZAP:       return "¶©ÔÄÌÖÂÛÇø";
    case EXCE_MJ:   return "Î§³ÇÕù°Ô";
    case EXCE_BIG2: return "±È´óÓª";
    case EXCE_CHESS:return "³şºÓºº½ç";
    case NOTEPAD:   return "ÁôÑÔ°å";
    case GMENU:     return "¹¤¾ßÏä";
    case FOURM:     return "4m Chat";
    case ULDL:      return "UL/DL" ;
    case MSG:       return NAME_SEND_MSG;
    case USERDEF:   return "×Ô¶©²ÎÊı";
    case EDIT:      return "ĞŞ¸ÄÎÄÕÂ";
    case OFFLINE:   return "×ÔÉ±ÖĞ..";
    case EDITANN:   return "±àĞŞ¾«»ª";
    case CCUGOPHER: return "ËûÕ¾¾«»ª";
    case LOOKMSGS:  return NAME_VIEW_MSG;
    case WFRIEND:   return "Ñ°ÈËÃû²á";
    case LOCKSCREEN:return "ÆÁÄ»Ëø¶¨";
    case GIVEUPNET:
        return "½äÍøÖĞ..";
    case WEBEXPLORE:return "Webä¯ÀÀ";
    case SERVICES:    return "ĞİÏĞÓéÀÖ..";
	case FRIENDTEST:  return "ĞÄÓĞÁéÏ¬";
    case CHICKEN:
	return "ĞÇ¿ÕÕ½¶·¼¦";
	case KILLER:        return "É±ÈËÓÎÏ·";
    default: return "È¥ÁËÄÇÀï!?" ;
    }
}

struct count_arg {
    int www_count;
    int telnet_count;
};

int countuser(struct user_info* uinfo,struct count_arg* arg,int pos)
{
    if (uinfo->mode==WEBEXPLORE)
        arg->www_count++;
    else
        arg->telnet_count++;
    return COUNT;
}

int multilogin_user(struct userec *user, int usernum,int mode)
{
    int logincount;
    int curr_login_num;
    struct count_arg arg;

    bzero(&arg,sizeof(arg));
    logincount = apply_utmpuid((APPLY_UTMP_FUNC)countuser, usernum, &arg);

    if (logincount < 1)
        RemoveMsgCountFile(user->userid);

#ifdef FILTER
    if (!strcmp(user->userid,"menss")&&logincount<2)
        return 0;
#endif
    if (HAS_PERM(user, PERM_MULTILOG))
        return 0;               /* don't check sysops */
    curr_login_num = get_utmp_number();
    /* Leeward: 97.12.22 BMs may open 2 windows at any time */
    /* Bigman: 2000.8.17 ÖÇÄÒÍÅÄÜ¹»¿ª2¸ö´°¿Ú */
    /* stephen: 2001.10.30 ÖÙ²Ã¿ÉÒÔ¿ªÁ½¸ö´°¿Ú */
	/* roy: °æÖ÷&AKA°æÍ¬Ñ§¿ÉÒÔ¿ªÁ½¸ö´°¿Ú */
    if ((HAS_PERM(user,PERM_BOARDS) || HAS_PERM(user,PERM_CHATOP)||
        HAS_PERM(user,PERM_JURY) || HAS_PERM(user,PERM_CHATCLOAK)
		|| HAS_PERM(user, PERM_BMAMANGER)  )
        && logincount< 2)
        return 0;

    /* allow multiple guest user */
    if (!strcmp("guest", user->userid)) {
        if (logincount > MAX_GUEST_NUM) {
            return 2;
        }
        return 0;
    } else if (((curr_login_num < 500) && (logincount >= 2)) /*Ğ¡ÓÚ500¿ÉÒÔË«µÇ*/
               || ((curr_login_num >= 500) && (logincount >= 1)  /*500ÈËÒÔÉÏ*/
                     && !(((arg.telnet_count==0)&&(mode==0))  /* telnet¸öÊıÎªÁã¿ÉÒÔÔÙµÇÒ»¸ötelnet */
                            || (((arg.www_count==0)&&(mode==1)) ))))       /*user login limit */
        return 1;
    return 0;
}

int old_compute_user_value( struct userec *urec)
{
    int         value;

    /* if (urec) has CHATCLOAK permission, don't kick it */
	/* ÔªÀÏºÍÈÙÓşÕÊºÅ ÔÚ²»×ÔÉ±µÄÇé¿öÏÂ£¬ ÉúÃüÁ¦999 Bigman 2001.6.23 */
    /* 
    * zixia 2001-11-20 ËùÓĞµÄÉúÃüÁ¦¶¼Ê¹ÓÃºêÌæ»»£¬
    * ÔÚ smth.h/zixia.h ÖĞ¶¨Òå 
    * */

    if( urec->userlevel & PERM_MM )
	return LIFE_DAY_SYSOP;
    
    if( ((urec->userlevel & PERM_HORNOR)||(urec->userlevel & PERM_CHATCLOAK )) && (!(urec->userlevel & PERM_SUICIDE)))
        return LIFE_DAY_NODIE;

    if ( urec->userlevel & PERM_SYSOP) 
	return LIFE_DAY_SYSOP;
	/* Õ¾ÎñÈËÔ±ÉúÃüÁ¦²»±ä Bigman 2001.6.23 */
	

    value = (time(0) - urec->lastlogin) / 60;    /* min */
    if (0 == value) value = 1; /* Leeward 98.03.30 */

    /* ĞŞ¸Ä: ½«ÓÀ¾ÃÕÊºÅ×ªÎª³¤ÆÚÕÊºÅ, Bigman 2000.8.11 */
    if ((urec->userlevel & PERM_XEMPT) && (!(urec->userlevel & PERM_SUICIDE)) )
    {	if (urec->lastlogin < 988610030)
        return LIFE_DAY_LONG; /* Èç¹ûÃ»ÓĞµÇÂ¼¹ıµÄ */
        else
            return (LIFE_DAY_LONG * 24 * 60 - value)/(60*24);
    }
    /* new user should register in 30 mins */
    if( strcmp( urec->userid, "new" ) == 0 ) {
        return (LIFE_DAY_NEW - value) / 60; /* *->/ modified by dong, 1998.12.3 */
    }

    /* ×ÔÉ±¹¦ÄÜ,Luzi 1998.10.10 */
    if (urec->userlevel & PERM_SUICIDE)
        return (LIFE_DAY_SUICIDE * 24 * 60 - value)/(60*24);
    /**********************/
    if(urec->numlogins <= 3)
        return (LIFE_DAY_SUICIDE * 24 * 60 - value)/(60*24);
    if( !(urec->userlevel & PERM_LOGINOK) )
        return (LIFE_DAY_NEW * 24 * 60 - value)/(60*24);
    /* if (urec->userlevel & PERM_LONGID)
         return (667 * 24 * 60 - value)/(60*24); */
    return (LIFE_DAY_USER * 24 * 60 - value)/(60*24);
}

int compute_user_value(struct userec *urec)
{
    int value;
    int registeryear;
    int basiclife;

    /* if (urec) has CHATCLOAK permission, don't kick it */
    /* ÔªÀÏºÍÈÙÓşÕÊºÅ ÔÚ²»×ÔÉ±µÄÇé¿öÏÂ£¬ ÉúÃüÁ¦999 Bigman 2001.6.23 */
    /* 
       * zixia 2001-11-20 ËùÓĞµÄÉúÃüÁ¦¶¼Ê¹ÓÃºêÌæ»»£¬
       * ÔÚ smth.h/zixia.h ÖĞ¶¨Òå 
       * */
    /* ÌØÊâ´¦ÀíÇëÒÆ¶¯³öcvs ´úÂë */

    if (urec->lastlogin < 1022036050)
        return old_compute_user_value(urec);
    /* Õâ¸öÊÇËÀÈËµÄid,sigh */
    if ((urec->userlevel & PERM_HORNOR) && !(urec->userlevel & PERM_LOGINOK))
        return LIFE_DAY_LONG;


    if (((urec->userlevel & PERM_HORNOR) || (urec->userlevel & PERM_CHATCLOAK)) && (!(urec->userlevel & PERM_SUICIDE)))
        return LIFE_DAY_NODIE;

    if ((urec->userlevel & PERM_ANNOUNCE) && (urec->userlevel & PERM_OBOARDS))
        return LIFE_DAY_SYSOP;
    /* Õ¾ÎñÈËÔ±ÉúÃüÁ¦²»±ä Bigman 2001.6.23 */


    value = (time(0) - urec->lastlogin) / 60;   /* min */
    if (0 == value)
        value = 1;              /* Leeward 98.03.30 */

    /* new user should register in 30 mins */
    if (strcmp(urec->userid, "new") == 0) {
        return (LIFE_DAY_NEW - value) / 60;     /* *->/ modified by dong, 1998.12.3 */
    }

    /* ×ÔÉ±¹¦ÄÜ,Luzi 1998.10.10 */
    if (urec->userlevel & PERM_SUICIDE)
        return (LIFE_DAY_SUICIDE * 24 * 60 - value) / (60 * 24);
    /**********************/
    if (urec->numlogins <= 3)
        return (LIFE_DAY_SUICIDE * 24 * 60 - value) / (60 * 24);
    if (!(urec->userlevel & PERM_LOGINOK))
        return (LIFE_DAY_NEW * 24 * 60 - value) / (60 * 24);
    /* if (urec->userlevel & PERM_LONGID)
       return (667 * 24 * 60 - value)/(60*24); */
    registeryear = (time(0) - urec->firstlogin) / 31536000;

	/* roy: Á½ÄêÒÔÉÏÓÃ»§ÉúÃüÁ¦ÎªLIFE_DAY_LONG */
    if (registeryear < 2)
        basiclife = LIFE_DAY_USER + 1;
    else 
        basiclife = LIFE_DAY_LONG + 1;
    return (basiclife * 24 * 60 - value) / (60 * 24);
}

/**
 * ¾«»ªÇøÏà¹Øº¯Êı¡£
 */
int ann_get_postfilename(char *filename, struct fileheader *fileinfo,
						MENU *pm)
{
	char fname[PATHLEN];
	char *ip;

	strcpy(filename, fileinfo->filename);
	sprintf(fname, "%s/%s", pm->path, filename);
	ip = &filename[strlen(filename) - 1];
	while (dashf(fname)) {
		if (*ip == 'Z')
			ip++, *ip = 'A', *(ip + 1) = '\0';
		else
			(*ip)++;
		sprintf(fname, "%s/%s", pm->path, filename);
	}
}

/**
 * ÎÄÕÂÏà¹Øº¯Êı¡£
 */
time_t get_posttime(const struct fileheader *fileinfo)
{
	return atoi(fileinfo->filename + 2);
}

void set_posttime(struct fileheader *fileinfo)
{
	return;
}

void set_posttime2(struct fileheader *dest, struct fileheader *src)
{
	return;
}

/**
 * °æÃæÏà¹Ø¡£
 */
void build_board_structure(const char *board)
{
	return;
}

void get_mail_limit(struct userec* user,int *sumlimit,int * numlimit)
{
    if ((!(user->userlevel & PERM_SYSOP)) && strcmp(user->userid, "Arbitrator")) {
        if (user->userlevel & PERM_CHATCLOAK) {
            *sumlimit = 4000;
            *numlimit = 4000;
        } else
            /*
             *              * if (lookupuser->userlevel & PERM_BOARDS)
             *                           * set BM, chatop, and jury have bigger mailbox, stephen 2001.10.31 
             *                                        */
        if (user->userlevel & PERM_MANAGER) {
            *sumlimit = 600;
            *numlimit = 600;
        } else if (user->userlevel & PERM_LOGINOK) {
            *sumlimit = 240;
            *numlimit = 300;
        } else {
            *sumlimit = 15;
            *numlimit = 15;
        }
    }
    else {
        *sumlimit = 9999;
        *numlimit = 9999;
        return;
    }
}

/* board permissions control */
int check_read_perm(struct userec *user, const struct boardheader *board)
{
    if (board == NULL)
        return 0;
    if (board->level & PERM_POSTMASK || HAS_PERM(user, board->level) || (board->level & PERM_NOZAP)) {
        if (board->flag & BOARD_CLUB_READ) {    /*¾ãÀÖ²¿*/
            if (HAS_PERM(user,PERM_OBOARDS)&&HAS_PERM(user, PERM_SYSOP))
                return 1;
            if (board->clubnum <= 0 || board->clubnum >= MAXCLUB)
                return 0;
            if (user->club_read_rights[(board->clubnum - 1) >> 5] & (1 << ((board->clubnum - 1) & 0x1f)))
                return 1;
            else
                return 0;
        }
        return 1;
    }
    return 0;
}

int check_see_perm(struct userec* user,const struct boardheader* board)
{
    if (board == NULL)
        return 0;
    if (board->level & PERM_POSTMASK
    	|| ((user==NULL)&&(board->level==0))
    	|| ((user!=NULL)&& HAS_PERM(user, board->level) )
    	|| (board->level & PERM_NOZAP))
	{
        if (board->flag & BOARD_CLUB_HIDE)     /*Òş²Ø¾ãÀÖ²¿*/
		{
			if (user==NULL) return 0;
			   if (HAS_PERM(user, PERM_OBOARDS))
					return 1;
			   return check_read_perm(user,board);
		}
        return 1;
    }
    return 0;
}

//×Ô¶¯Í¨¹ı×¢²áµÄº¯Êı  binxun
int auto_register(char* userid,char* email,int msize)
{
	struct userdata ud;
	struct userec* uc;
	char* item,*temp;
	char fdata[7][STRLEN];
	char genbuf[STRLEN];
	char buf[STRLEN];
	char fname[STRLEN];
	int unum;
	FILE* fout;
	int n;
	struct userec deliveruser;
	static const char *finfo[] = { "ÕÊºÅÎ»ÖÃ", "ÉêÇë´úºÅ", "ÕæÊµĞÕÃû", "·şÎñµ¥Î»",
        "Ä¿Ç°×¡Ö·", "Á¬Âçµç»°", "Éú    ÈÕ", NULL
    };
  	static const char *field[] = { "usernum", "userid", "realname", "career",
    	"addr", "phone", "birth", NULL
	};

	bzero(&deliveruser,sizeof(struct userec));
	strcpy(deliveruser.userid,"deliver");
	deliveruser.userlevel = -1;
	strcpy(deliveruser.username,"×Ô¶¯·¢ĞÅÏµÍ³");



	bzero(fdata,7*STRLEN);

	if((unum = getuser(userid,&uc)) == 0)return -1;//faild
	if(read_userdata(userid,&ud) < 0)return -1;

	strncpy(genbuf,email,STRLEN - 16);
	item =strtok(genbuf,"#");
	if(item)strncpy(ud.realname,item,NAMELEN);
	item = strtok(NULL,"#");  //Ñ§ºÅ
	item = strtok(NULL,"#");
	if(item)strncpy(ud.address,item,STRLEN);

	email[strlen(email) - 3] = '@';
	strncpy(ud.realemail,email,STRLEN-16); //email length must be less STRLEN-16


	sprintf(fdata[0],"%d",unum);
	strncpy(fdata[2],ud.realname,NAMELEN);
	strncpy(fdata[4],ud.address,STRLEN);
	strncpy(fdata[5],ud.email,STRLEN);
	strncpy(fdata[1],userid,IDLEN);

	sprintf(buf,"tmp/email/%s",userid);
	if ((fout = fopen(buf,"w")) != NULL)
	{
		fprintf(fout,"%s\n",email);
		fclose(fout);
	}

	if(write_userdata(userid,&ud) < 0)return -1;
	mail_file("deliver","etc/s_fill",userid,"¹§Ï²Äã,ÄãÒÑ¾­Íê³É×¢²á.",0,0);
	//sprintf(genbuf,"deliver ÈÃ %s ×Ô¶¯Í¨¹ıÉí·İÈ·ÈÏ.",uinfo.userid);

	sprintf(fname, "tmp/security.%d", getpid());
	if ((fout = fopen(fname, "w")) != NULL)
	{
		fprintf(fout, "ÏµÍ³°²È«¼ÇÂ¼ÏµÍ³\n[32mÔ­Òò£º%s×Ô¶¯Í¨¹ı×¢²á[m\n", userid);
                fprintf(fout, "ÒÔÏÂÊÇÍ¨¹ıÕß¸öÈË×ÊÁÏ");
                fprintf(fout, "\n\nÄúµÄ´úºÅ     : %s\n", ud.userid);
                fprintf(fout, "ÄúµÄêÇ³Æ     : %s\n", uc->username);
                fprintf(fout, "ÕæÊµĞÕÃû     : %s\n", ud.realname);
                fprintf(fout, "µç×ÓÓÊ¼şĞÅÏä : %s\n", ud.email);
                fprintf(fout, "ÕæÊµ E-mail  : %s\n", ud.realemail);
                fprintf(fout, "·şÎñµ¥Î»     : %s\n", "");
                fprintf(fout, "Ä¿Ç°×¡Ö·     : %s\n", ud.address);
                fprintf(fout, "Á¬Âçµç»°     : %s\n", "");
                fprintf(fout, "×¢²áÈÕÆÚ     : %s", ctime(&uc->firstlogin));
                fprintf(fout, "×î½ü¹âÁÙÈÕÆÚ : %s", ctime(&uc->lastlogin));
                fprintf(fout, "×î½ü¹âÁÙ»úÆ÷ : %s\n", uc->lasthost);
                fprintf(fout, "ÉÏÕ¾´ÎÊı     : %d ´Î\n", uc->numlogins);
                fprintf(fout, "ÎÄÕÂÊıÄ¿     : %d(Board)\n", uc->numposts);
                fprintf(fout, "Éú    ÈÕ     : %s\n", "");

                fclose(fout);
                //post_file(currentuser, "", fname, "Registry", str, 0, 2);

		sprintf(genbuf,"%s ×Ô¶¯Í¨¹ı×¢²á",ud.userid);
		post_file(&deliveruser,"",fname,"Registry",genbuf,0,1);
	/*if (( fout = fopen(logfile,"a")) != NULL)
	{
		fclose(fout);
	}*/
	}

	sethomefile(buf, userid, "/register");
	if ((fout = fopen(buf, "w")) != NULL) {
		for (n = 0; field[n] != NULL; n++)
			fprintf(fout, "%s     : %s\n", finfo[n], fdata[n]);
		fprintf(fout, "ÄúµÄêÇ³Æ     : %s\n", uc->username);
		fprintf(fout, "µç×ÓÓÊ¼şĞÅÏä : %s\n", ud.email);
		fprintf(fout, "ÕæÊµ E-mail  : %s\n", ud.realemail);
		fprintf(fout, "×¢²áÈÕÆÚ     : %s\n", ctime(&uc->firstlogin));
		fprintf(fout, "×¢²áÊ±µÄ»úÆ÷ : %s\n", uc->lasthost);
		fprintf(fout, "Approved: %s\n", userid);
		fclose(fout);
	}

	return 0;
}

