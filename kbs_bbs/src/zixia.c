
#include "bbs.h"

int
uleveltochar( char* buf, struct userec *lookupuser )
{
	unsigned lvl;
	char userid[IDLEN+2];
	
	lvl = lookupuser->userlevel;
	strncpy( userid, lookupuser->userid, IDLEN+2 );

	// buf[10], buf 最多 4 个汉字 + 1 byte （\0结尾）
	//根据 level
   	if( lvl < PERM_DEFAULT ) strcpy( buf, "书生" );
    	else if( lvl & PERM_SYSOP ) strcpy(buf,"帮主");
      	else if( lvl & PERM_MM && lvl & PERM_CHATCLOAK ) strcpy(buf,"晶晶姑娘");
      	else if( lvl & PERM_MM ) strcpy(buf,"观音姐姐");
      	else  if( lvl & PERM_CHATCLOAK ) strcpy(buf,"无敌牛虱");
      	else if  ( lvl & PERM_BOARDS ) strcpy(buf,"二当家的");
      	else if ( lvl & PERM_DENYPOST ) strcpy( buf, "哑巴" ); 
      	else if ( ! (lvl & PERM_BASIC) ) strcpy( buf, "瞎子" ); 
      	else strcpy(buf,"斧头帮众");

	//中文说明，根据 level
    	if( !strcmp(lookupuser->userid,"SYSOP")
		    || !strcmp(lookupuser->userid,"Rama") )
	    strcpy( buf, "强盗头" );
    	else if( !strcmp(lookupuser->userid,"zixia") )
	    strcpy( buf, "旺财" );
    	else if( !strcmp(lookupuser->userid,"halen") )
	    strcpy( buf, "小皮卡秋" );
    	else if( !strcmp(lookupuser->userid,"cityhunter") 
		    || !strcmp(lookupuser->userid,"soso")
	  	    || !strcmp(lookupuser->userid,"KCN") )
	    strcpy( buf, "牛魔王" );
    	else if( !strcmp(lookupuser->userid,"guest") )
	    strcpy( buf, "葡萄" );

    	return 1;
}
modetype.c
