#include "../bbs.h"

int do_userlist(struct user_info* uentp,char* arg,int t)
{
    int i;
    int fd,len;
    char  user_info_str[256/*STRLEN*2*/],pagec;
    int   override;

    t++;
    if (!uentp->active||!uentp->pid)
	{
		printf(" %4d °¡,ÎÒ¸Õ×ß\n",t);
		return 0;
    }	
    pagec=' ';
    sprintf( user_info_str,
             /*---	modified by period	2000-10-21	ÔÚÏßÓÃ»§Êý¿ÉÒÔ´óÓÚ1000µÄ
                     " %3d%2s%s%-12.12s%s%s %-16.16s%s %-16.16s %c %c %s%-17.17s[m%5.5s\n",
             ---*/
             " %4d%2s%-12.12s %-16.16s %-16.16s %c %c %s%-12.12s[m%5.5s %d\n",
             t,uentp->invisible?"££":"£®",
                     uentp->userid,
                     uentp->username,
                     uentp->from,
                     pagec,' ',(uentp->invisible==YEA)
                     ?"[34m":"",
                     modestring(uentp->mode, uentp->destuid, 0,/* 1->0 ²»ÏÔÊ¾ÁÄÌì¶ÔÏóµÈ modified by dong 1996.10.26 */
                                (uentp->in_chat ? uentp->chatid : NULL)),
                     idle_str( uentp ),
                     uentp->pid
                     );
    printf( "%s", user_info_str );
    return COUNT;
}

void main(argc,argv)
int argc;
char * argv[];
{
    resolve_utmp();
    printf(" ÐòºÅ  ÓÃ»§ID       êÇ³Æ             À´Ô´                 ×´Ì¬     ·¢´ôÊ±¼ä ½ø³ÌºÅ\n");
    apply_ulist_addr(do_userlist,NULL);
}
