#include "bbs.h"

const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

const char seccode[SECNUM][5] = {
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9" };
const char *groups[] = {
    "GROUP_0", 
    "GROUP_1", 
    "GROUP_2", 
    "GROUP_3",
    "GROUP_4",
    "GROUP_5",
    "GROUP_6",
    "GROUP_7",
    "GROUP_8",
    "GROUP_9",
    NULL
};
const char secname[SECNUM][2][20] = {
    {"±¾Õ¾ÏµÍ³", "[Õ¾ÄÚ]"},
    {"Ê¯ÓÍÑ§Ôº", "[±¾Ğ£]"},
    {"ĞÖµÜÔºĞ£", "[¸ßĞ£/×éÖ¯]"},
    {"µçÄÔ¼¼Êõ", "[µçÄÔ/ÍøÂç]"},
    {"ĞİÏĞÓéÀÖ", "[ĞİÏĞ/ÓéÀÖ]"},
    {"Ñ§Êõ¿ÆÑ§", "[ÎÄÀí/¿ÆÑ§]"},
    {"ÎÄÒÕÉç½»", "[ÎÄÑ§/ÒÕÊõ]"},
    {"ÌåÓıÔË¶¯", "[ÔË¶¯/½¡¿µ]"},
    {"Ì¸ÌìÁÄµØ", "[Ì¸Ìì/¸ĞĞÔ]"},
    {"¼¼Êõ×ªĞÅ", "[ÍøÂç/±à³Ì]"},
};

const char *explain[] = {
    "±¾Õ¾ÏµÍ³",
    "Ê¯ÓÍÑ§Ôº",
    "ĞÖµÜÔºĞ£",
    "µçÄÔ¼¼Êõ",
    "ĞİÏĞÓéÀÖ",
    "Ñ§Êõ¿ÆÑ§",
    "ÎÄÒÕÉç½»",
    "ÌåÓı½¡Éí",
    "ÖªĞÔ¸ĞĞÔ",
    "¼¼Êõ×ªĞÅ",
    NULL
};

const char *permstrings[] = {
    "»ù±¾È¨Á¦",                 /* PERM_BASIC */
    "½øÈëÁÄÌìÊÒ",               /* PERM_CHAT */
    "ºô½ĞËûÈËÁÄÌì",             /* PERM_PAGE */
    "·¢±íÎÄÕÂ",                 /* PERM_POST */
    "Ê¹ÓÃÕß×ÊÁÏÕıÈ·",           /* PERM_LOGINOK */
    "ÊµÏ°Õ¾Îñ",                 /* PERM_BMMANAGER */
    "ÒşÉí",                   /* PERM_CLOAK */
    "¿´Í¸ÒşÉí",                 /* PERM_SEECLOAK */
    "ÓÀ¾ÃÕÊºÅ",                 /* PERM_XEMPT */
    "±à¼­ÏµÍ³µµ°¸",             /* PERM_WELCOME */
    "°åÖ÷",                     /* PERM_BOARDS */
    "ÕÊºÅ¹ÜÀíÔ±",               /* PERM_ACCOUNTS */
    "±¾Õ¾ÖÇÄÒÍÅ",           /* PERM_CHATCLOAK */
    "·â½ûÓéÀÖÈ¨ÏŞ",             /* PERM_DENYRELAX */
    "ÏµÍ³Î¬»¤¹ÜÀíÔ±",           /* PERM_SYSOP */
    "Read/Post ÏŞÖÆ",           /* PERM_POSTMASK */
    "¾«»ªÇø×Ü¹Ü",               /* PERM_ANNOUNCE */
    "ÌÖÂÛÇø×Ü¹Ü",               /* PERM_OBOARDS */
    "»î¶¯¿´°æ×Ü¹Ü",             /* PERM_ACBOARD */
    "²»ÄÜ ZAP(ÌÖÂÛÇø×¨ÓÃ)",     /* PERM_NOZAP */
    "ÁÄÌìÊÒOP(ÔªÀÏÔº×¨ÓÃ)",     /* PERM_CHATOP */
    "ÏµÍ³×Ü¹ÜÀíÔ±",             /* PERM_ADMIN */
    "ÈÙÓşÕÊºÅ",                 /* PERM_HONOR */
    "ÌØÊâÈ¨ÏŞ 5",               /* PERM_UNUSE? */
    "ÖÙ²ÃÎ¯Ô±",                 /* PERM_JURY */
    "ÌØÊâÈ¨ÏŞ 7",               /* PERM_UNUSE? */
    "×ÔÉ±½øĞĞÖĞ",               /*PERM_SUICIDE */
    "¼¯Ìå×¨ÓÃÕÊºÅ",             /* PERM_COLLECTIVE */
    "¿´ÏµÍ³ÌÖÂÛ°æ",             /* PERM_UNUSE? */
    "·â½ûMail",                 /* PERM_DENYMAIL */

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
    "ÀëÕ¾ºó¼Ä»ØËùÓĞÑ¶Ï¢",       /* DEF_MAILMSG */
    "·¢ÎÄÕÂÊ±ÊµÊ±ÏÔÊ¾Ñ¶Ï¢",     /*"ËùÓĞºÃÓÑÉÏÕ¾¾ùÍ¨Öª",    DEF_LOGININFORM */
    "²Ëµ¥ÉÏÏÔÊ¾°ïÖúĞÅÏ¢",       /* DEF_SHOWSCREEN */
    "½øÕ¾Ê±ÏÔÊ¾Ê®´óĞÂÎÅ",       /* DEF_SHOWHOT */
    "½øÕ¾Ê±¹Û¿´ÁôÑÔ°æ",         /* DEF_NOTEPAD */
    "ºöÂÔÑ¶Ï¢¹¦ÄÜ¼ü: Enter/Esc",        /* DEF_IGNOREMSG */
    "Ê¹ÓÃ¸ßÁÁ½çÃæ",             /* DEF_HIGHCOLOR */
    "¹Û¿´ÈËÊıÍ³¼ÆºÍ×£¸£°ñ",     /* DEF_SHOWSTATISTIC Haohmaru 98.09.24 */
    "Î´¶Á±ê¼ÇÊ¹ÓÃ *",           /* DEF_UNREADMARK Luzi 99.01.12 */
    "Ê¹ÓÃGBÂëÔÄ¶Á",             /* DEF_USEGB KCN 99.09.03 */
    "¶Ôºº×Ö½øĞĞÕû×Ö´¦Àí",  /* DEF_SPLITSCREEN 2002.9.1 */
    "ÏÔÊ¾ÏêÏ¸ÓÃ»§Êı¾İ",  /*DEF_SHOWDETAILUSERDATA 2003.7.31 */
    "ÏÔÊ¾ÕæÊµÓÃ»§Êı¾İ" /*DEF_REALDETAILUSERDATA 2003.7.31 */
};

const char *mailbox_prop_str[] = {
    "·¢ĞÅÊ±±£´æĞÅ¼şµ½·¢¼şÏä",
    "É¾³ıĞÅ¼şÊ±²»±£´æµ½À¬»øÏä",
    "°æÃæ°´ 'v' Ê±½øÈë: ÊÕ¼şÏä(OFF) / ĞÅÏäÖ÷½çÃæ(ON)",
};

int uleveltochar(char *buf, struct userec *lookupuser)
{                               /* È¡ÓÃ»§È¨ÏŞÖĞÎÄËµÃ÷ Bigman 2001.6.24 */
    unsigned lvl;
    char userid[IDLEN + 2];

#ifdef HAVE_CUSTOM_USER_TITLE
    if (lookupuser->title!=0) {
        strcpy(buf,get_user_title(lookupuser->title));
        if (buf[0]!=0) return 0;
    }
#endif
    lvl = lookupuser->userlevel;
    strncpy(userid, lookupuser->userid, IDLEN + 2);

    if (!(lvl & PERM_BASIC) && !(lookupuser->flags & GIVEUP_FLAG)) {
        strcpy(buf, "ĞÂÈË");
        return 0;
    }
/*    if( lvl < PERM_DEFAULT )
    {
        strcpy( buf, "- --" );
        return 1;
    }
*/

    /*
     * Bigman: Ôö¼ÓÖĞÎÄ²éÑ¯ÏÔÊ¾ 2000.8.10 
     */
    /*
     * if( lvl & PERM_ZHANWU ) strcpy(buf,"Õ¾Îñ"); 
     */
    if ((lvl & PERM_ANNOUNCE) && (lvl & PERM_OBOARDS))
        strcpy(buf, "Õ¾Îñ");
    else if (lvl & PERM_JURY)
        strcpy(buf, "ÖÙ²Ã");    /* stephen :Ôö¼ÓÖĞÎÄ²éÑ¯"ÖÙ²Ã" 2001.10.31 */
    else if (lvl & PERM_BMAMANGER)
        strcpy(buf, "ÊµÏ°Õ¾Îñ");
    else if (lvl & PERM_COLLECTIVE)
        strcpy(buf, "×¨ÓÃ");
    else if (lvl & PERM_CHATCLOAK)
        strcpy(buf, "ÔªÀÏ");
    else if (lvl & PERM_CHATOP)
        strcpy(buf, "ÁÄÌìÊÒÀÏ´ó");
    else if (lvl & PERM_BOARDS)
        strcpy(buf, "°æÖ÷");
    else if (lvl & PERM_HORNOR)
        strcpy(buf, "ÈÙÓş");
    /*
     * Bigman: ĞŞ¸ÄÏÔÊ¾ 2001.6.24 
     */
    else if (lvl & (PERM_LOGINOK)) {
        if (lookupuser->flags & GIVEUP_FLAG)
            strcpy(buf, "½äÍø");
        else if (!(lvl & (PERM_CHAT)) || !(lvl & (PERM_PAGE)) || !(lvl & (PERM_POST))
                 || (lvl & (PERM_DENYMAIL)) || (lvl & (PERM_DENYRELAX)))
            strcpy(buf, "ÊÜÏŞ");
        else
            strcpy(buf, "ÓÃ»§");
    } else if (lookupuser->flags & GIVEUP_FLAG)
        strcpy(buf, "½äÍø");
    else if (!(lvl & (PERM_CHAT)) && !(lvl & (PERM_PAGE)) && !(lvl & (PERM_POST)))
        strcpy(buf, "ĞÂÈË");
    else
        strcpy(buf, "ÊÜÏŞ");

/*    else {
        buf[0] = (lvl & (PERM_SYSOP)) ? 'C' : ' ';
        buf[1] = (lvl & (PERM_XEMPT)) ? 'L' : ' ';
        buf[2] = (lvl & (PERM_BOARDS)) ? 'B' : ' ';
        buf[3] = !(lvl & (PERM_POST)) ? 'p' : ' ';
        if( lvl & PERM_ACCOUNTS ) buf[3] = 'A';
        if( lvl & PERM_SYSOP ) buf[3] = 'S'; 
        buf[4] = '\0';
    }
*/

    return 1;
}


#include "modes.h"

char *ModeType(mode)
int mode;
{
    switch (mode) {
    case IDLE:
        return "";
    case NEW:
        return "ĞÂÕ¾ÓÑ×¢²á";
    case LOGIN:
        return "½øÈë±¾Õ¾";
    case CSIE_ANNOUNCE:
        return "¼³È¡¾«»ª";
    case MMENU:
        return "Ö÷²Ëµ¥";
    case ADMIN:
        return "ÏµÍ³Î¬»¤";
    case SELECT:
        return "Ñ¡ÔñÌÖÂÛÇø";
    case READBRD:
        return "ä¯ÀÀÌÖÂÛÇø";
    case READNEW:
        return "ÔÄ¶ÁĞÂÎÄÕÂ";
    case READING:
        return "ÔÄ¶ÁÎÄÕÂ";
    case POSTING:
        return "·¢±íÎÄÕÂ";
    case MAIL:
        return "ĞÅ¼şÑ¡µ¥";
    case SMAIL:
        return "¼ÄĞÅÖĞ";
    case RMAIL:
        return "¶ÁĞÅÖĞ";
    case TMENU:
        return "Ì¸ÌìËµµØÇø";
    case LUSERS:
        return "¿´Ë­ÔÚÏßÉÏ";
    case FRIEND:
        return "ÕÒÏßÉÏºÃÓÑ";
//    case MONITOR:
//        return "¼à¿´ÖĞ";
    case QUERY:
        return "²éÑ¯ÍøÓÑ";
    case TALK:
        return "ÁÄÌì";
    case PAGE:
        return "ºô½ĞÍøÓÑ";
//    case CHAT2:
//        return "ÃÎ»Ã¹ú¶È";
    case CHAT1:
        return "ÁÄÌìÊÒÖĞ";
//    case CHAT3:
//        return "¿ìÔÕÍ¤";
//    case CHAT4:
//        return "ÀÏ´óÁÄÌìÊÒ";
//    case IRCCHAT:
//        return "»áÌ¸IRC";
    case LAUSERS:
        return "Ì½ÊÓÍøÓÑ";
    case XMENU:
        return "ÏµÍ³×ÊÑ¶";
    case VOTING:
        return "Í¶Æ±";
    case BBSNET:
        return "ÏµÍ³Î¬»¤";      //ft
        /*
         * return "´©ËóÒøºÓ";
         */
//    case EDITWELC:
//        return "±à¼­ Welc";
    case EDITUFILE:
        return "±à¼­µµ°¸";
    case EDITSFILE:
        return "ÏµÍ³¹ÜÀí";
        /*
         * case  EDITSIG:  return "¿ÌÓ¡";
         * case  EDITPLAN: return "Äâ¼Æ»­"; 
         */
//    case ZAP:
//        return "¶©ÔÄÌÖÂÛÇø";
//    case EXCE_MJ:
//        return "Î§³ÇÕù°Ô";
//    case EXCE_BIG2:
//        return "±È´óÓª";
//    case EXCE_CHESS:
//        return "³şºÓºº½ç";
    case NOTEPAD:
        return "ÁôÑÔ°å";
    case GMENU:
        return "¹¤¾ßÏä";
//    case FOURM:
//        return "4m Chat";
//    case ULDL:
//        return "UL/DL";
    case MSG:
        return "Ñ¶Ï¢ÖĞ";
    case USERDEF:
        return "×Ô¶©²ÎÊı";
    case EDIT:
        return "ĞŞ¸ÄÎÄÕÂ";
    case OFFLINE:
        return "×ÔÉ±ÖĞ..";
    case EDITANN:
        return "±àĞŞ¾«»ª";
    case WEBEXPLORE:
        return "Webä¯ÀÀ";
//    case CCUGOPHER:
//        return "ËûÕ¾¾«»ª";
    case LOOKMSGS:
        return "²ì¿´Ñ¶Ï¢";
    case WFRIEND:
        return "Ñ°ÈËÃû²á";
    case LOCKSCREEN:
        return "ÆÁÄ»Ëø¶¨";
    case IMAIL:
        return "¼ÄÕ¾ÍâĞÅÖĞ";
    case GIVEUPNET:
        return "½äÍøÖĞ..";
    case SERVICES:
        return "ĞİÏĞÓéÀÖ..";
    case FRIENDTEST:
        return "ĞÄÓĞÁéÏ¬";
    case CHICKEN:
        return "ĞÇ¿ÕÕ½¶·¼¦";
	/*
    case FIVE:
        return "¾öÕ½Îå×ÓÆå";
    case PAGE_FIVE:
	return "ÑûÇëÏÂÆå";
	*/

    default:
        return "È¥ÁËÄÇÀï!?";
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



int multilogin_user(struct userec *user, int usernum, int mode)
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
	/* binxun 2003.5 ÖÙ²Ã£¬°æÖ÷£¬Chatop£¬µÈ¶¼¿ÉÒÔÈıµÇ*/
	if ((HAS_PERM(user, PERM_BOARDS) || HAS_PERM(user, PERM_CHATOP)
				|| HAS_PERM(user, PERM_JURY) || HAS_PERM(user, PERM_CHATCLOAK)
				|| HAS_PERM(user, PERM_BMAMANGER) )
			&& logincount < 3)
		return 0;
	/* allow multiple guest user */
	if (!strcmp("guest", user->userid)) {
		if (logincount > MAX_GUEST_NUM) {
			return 2;
		}
		return 0;
	} else if (((curr_login_num < 30) && (logincount >= 3)) /*Ğ¡ÓÚ700¿ÉÒÔÈıµÇ*/
			|| ((curr_login_num >= 30) && (logincount >= 2)  /*700ÈËÒÔÉÏ*/
				&& !(((arg.telnet_count==0)&&(mode==0))  /* telnet¸öÊıÎªÁã¿ÉÒÔÔÙµÇÒ»¸ötelnet */
					|| (((arg.www_count==0)&&(mode==1)) ))))       /*user login limit */
		return 1;
	return 0;

}


#define LIFE_DAY_USER		120
#define LIFE_DAY_YEAR          365
#define LIFE_DAY_LONG		666
#define LIFE_DAY_SYSOP		120
#define LIFE_DAY_NODIE		999
#define LIFE_DAY_NEW		30
#define LIFE_DAY_SUICIDE	30
int compute_user_value(struct userec *urec)
{
	int value;
	int registeryear;
	int basiclife;

	/*
	 * if (urec) has CHATCLOAK permission, don't kick it 
	 */
	/*
	 * ÔªÀÏºÍÈÙÓşÕÊºÅ ÔÚ²»×ÔÉ±µÄÇé¿öÏÂ£¬ ÉúÃüÁ¦999 Bigman 2001.6.23 
	 */
	/*
	 * Õâ¸öÊÇËÀÈËµÄid,sigh 
	 */
	if (urec->userlevel & PERM_XEMPT)
		return LIFE_DAY_NODIE;

	if ((urec->userlevel & PERM_HORNOR) && !(urec->userlevel & PERM_LOGINOK))
		return LIFE_DAY_LONG;


	if (((urec->userlevel & PERM_HORNOR) || (urec->userlevel & PERM_CHATCLOAK)) && (!(urec->userlevel & PERM_SUICIDE)))
		return LIFE_DAY_NODIE;

	if ((urec->userlevel & PERM_ANNOUNCE) && (urec->userlevel & PERM_OBOARDS))
		return LIFE_DAY_SYSOP;
	/*
	 * Õ¾ÎñÈËÔ±ÉúÃüÁ¦²»±ä Bigman 2001.6.23 
	 */


	value = (time(0) - urec->lastlogin) / 60;   /* min */
	if (0 == value)
		value = 1;              /* Leeward 98.03.30 */

	/*
	 * new user should register in 30 mins 
	 */
	if (strcmp(urec->userid, "new") == 0) {
		return (LIFE_DAY_NEW - value) / 60;     /* *->/ modified by dong, 1998.12.3 */
	}

	/*
	 * ×ÔÉ±¹¦ÄÜ,Luzi 1998.10.10 
	 */
	if (urec->userlevel & PERM_SUICIDE)
		return (LIFE_DAY_SUICIDE * 24 * 60 - value) / (60 * 24);
	/**********************/
	if (urec->numlogins <= 3)
		return (LIFE_DAY_SUICIDE * 24 * 60 - value) / (60 * 24);
	if (!(urec->userlevel & PERM_LOGINOK))
		return (LIFE_DAY_NEW * 24 * 60 - value) / (60 * 24);
	/*
     * if (urec->userlevel & PERM_LONGID)
     * return (667 * 24 * 60 - value)/(60*24); 
     */
    registeryear = (time(0) - urec->firstlogin) / 31536000;
    if (registeryear < 2)
        basiclife = LIFE_DAY_USER + 1;
    else if (registeryear >= 5)
        basiclife = LIFE_DAY_LONG + 1;
    else
        basiclife = LIFE_DAY_YEAR + 1;
    return (basiclife * 24 * 60 - value) / (60 * 24);
}


/**
 * ¾«»ªÇøÏà¹Øº¯Êı¡£
 */
int ann_get_postfilename(char *filename, struct fileheader *fileinfo, MENU * pm)
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


void get_mail_limit(struct userec *user, int *sumlimit, int *numlimit)
{
    /*
     * unlimit
     */
    *sumlimit = 9999;
    *numlimit = 9999;
    return;
}

/* board permissions control */
int check_read_perm(struct userec *user, const struct boardheader *board)
{
    if (board == NULL)
        return 0;
    if (!HAS_PERM(user, PERM_OBOARDS)&&board->title_level
        &&(board->title_level!=user->title))
        return 0;

    if (board->level & PERM_POSTMASK || HAS_PERM(user, board->level) || (board->level & PERM_NOZAP)) {
        if (board->flag & BOARD_CLUB_READ) {    /*¾ãÀÖ²¿ */
            if (HAS_PERM(user, PERM_OBOARDS) && HAS_PERM(user, PERM_SYSOP))
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

int check_see_perm(struct userec *user, const struct boardheader *board)
{
    if (board == NULL)
        return 0;
    if (!HAS_PERM(user, PERM_OBOARDS)&&board->title_level
        &&(board->title_level!=user->title))
        return 0;

    if (board->level & PERM_POSTMASK || ((user == NULL) && (board->level == 0))
        || ((user != NULL) && HAS_PERM(user, board->level))
        || (board->level & PERM_NOZAP)) {
        if (board->flag & BOARD_CLUB_HIDE) {    /*Òş²Ø¾ãÀÖ²¿ */
            if (user == NULL)
                return 0;
            if (HAS_PERM(user, PERM_OBOARDS))
                return 1;
            return check_read_perm(user, board);
        }
        return 1;
    }
    return 0;
}

//×Ô¶¯Í¨¹ı×¢²áµÄº¯Êı  binxun
int auto_register(char *userid, char *email, int msize)
{
    struct userdata ud;
    struct userec *uc;
    char *item, *temp;
    char fdata[7][STRLEN];
    char genbuf[STRLEN];
    char buf[STRLEN];
    char fname[STRLEN];
    int unum;
    FILE *fout;
    int n;
    struct userec deliveruser;
    static const char *finfo[] = { "ÕÊºÅÎ»ÖÃ", "ÉêÇë´úºÅ", "ÕæÊµĞÕÃû", "·şÎñµ¥Î»",
        "Ä¿Ç°×¡Ö·", "Á¬Âçµç»°", "Éú    ÈÕ", NULL
    };
    static const char *field[] = { "usernum", "userid", "realname", "career",
        "addr", "phone", "birth", NULL
    };

    bzero(&deliveruser, sizeof(struct userec));
    strcpy(deliveruser.userid, "deliver");
    deliveruser.userlevel = -1;
    strcpy(deliveruser.username, "×Ô¶¯·¢ĞÅÏµÍ³");



    bzero(fdata, 7 * STRLEN);

    if ((unum = getuser(userid, &uc)) == 0)
        return -1;              //faild
    if (read_userdata(userid, &ud) < 0)
        return -1;

    strncpy(genbuf, email, STRLEN - 16);
    item = strtok(genbuf, "#");
    if (item)
        strncpy(ud.realname, item, NAMELEN);
    item = strtok(NULL, "#");   //Ñ§ºÅ
    item = strtok(NULL, "#");
    if (item)
        strncpy(ud.address, item, STRLEN);

    email[strlen(email) - 3] = '@';
    strncpy(ud.realemail, email, STRLEN - 16);  //email length must be less STRLEN-16


    sprintf(fdata[0], "%d", unum);
    strncpy(fdata[2], ud.realname, NAMELEN);
    strncpy(fdata[4], ud.address, STRLEN);
    strncpy(fdata[5], ud.email, STRLEN);
    strncpy(fdata[1], userid, IDLEN);

    sprintf(buf, "tmp/email/%s", userid);
    if ((fout = fopen(buf, "w")) != NULL) {
        fprintf(fout, "%s\n", email);
        fclose(fout);
    }

    if (write_userdata(userid, &ud) < 0)
        return -1;
    mail_file("deliver", "etc/s_fill", userid, "¹§Ï²Äã,ÄãÒÑ¾­Íê³É×¢²á.", 0, 0);
    //sprintf(genbuf,"deliver ÈÃ %s ×Ô¶¯Í¨¹ıÉí·İÈ·ÈÏ.",uinfo.userid);

    sprintf(fname, "tmp/security.%d", getpid());
    if ((fout = fopen(fname, "w")) != NULL) {
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

        sprintf(genbuf, "%s ×Ô¶¯Í¨¹ı×¢²á", ud.userid);
        post_file(&deliveruser, "", fname, "Registry", genbuf, 0, 1);
        /*
         * if (( fout = fopen(logfile,"a")) != NULL)
         * {
         * fclose(fout);
         * }
         */
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
