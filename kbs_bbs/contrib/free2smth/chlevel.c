/*
 * This program converts Users' attribute from Firebird 2000 to smth_bbs
 */

#include "bbs.h"
main()
{
    FILE *rec;
    int i = 0;
    struct userec user;
    unsigned int perm;

    rec = fopen(BBSHOME "/.PASSWDS", "rb");
    if(NULL==rec) {
	    printf("Can not open .PASSWDS");
	    return 0;
    }

    printf("\033[1;31;5mUserLevel Records Transfering...\n\033[m");

    while (1) {
        if (fread(&user, sizeof(user), 1, rec) <= 0)
            break;
        i++;
        if (user.numlogins <= 0)
            continue;
        printf("%d %s\t\n", i, user.userid);
        perm = user.userlevel;
        user.userlevel = PERM_BASIC;
        user.userlevel |= PERM_CHAT;
        user.userlevel |= PERM_PAGE;
        user.userlevel |= PERM_POST;
        user.userlevel |= PERM_LOGINOK;

        if (perm & PERM_CLOAK)
            user.userlevel |= PERM_CLOAK;
        if (perm & PERM_SEECLOAK)
            user.userlevel |= PERM_SEECLOAK;
        if (perm & PERM_XEMPT)
            user.userlevel |= PERM_XEMPT;
        if (perm & PERM_BOARDS)
            user.userlevel |= PERM_BOARDS;
        if (perm & PERM_ACCOUNTS)
            user.userlevel |= PERM_ACCOUNTS;
        if (perm & PERM_CHATCLOAK)
            user.userlevel |= PERM_CHATCLOAK;
        if (perm & PERM_SYSOP) {
            user.userlevel |= PERM_SYSOP;
            user.userlevel |= PERM_ADMIN;
        }
	if (perm & PERM_HORNOR) 
	    user.userlevel &=~PERM_HORNOR;
        if (perm & PERM_POSTMASK)
            user.userlevel |= PERM_POSTMASK;
        if (perm & PERM_ANNOUNCE)
            user.userlevel |= PERM_ANNOUNCE;
        if (perm & PERM_OBOARDS)
            user.userlevel |= PERM_OBOARDS;
        if (perm & PERM_ACBOARD)
            user.userlevel |= PERM_ACBOARD;
        if (perm & PERM_NOZAP)
            user.userlevel |= PERM_NOZAP;
	if (perm & PERM_JURY)
	    user.userlevel &=~PERM_JURY;
	user.userdefine[0] = -1;
	user.userdefine[1] = -1;
#ifdef HAVE_WFORUM
	SET_UNDEFINE(&user,DEF_SHOWREALUSERDATA);
#endif
        user.flags = CURSOR_FLAG;
        user.flags |= PAGER_FLAG;
        user.title = 0;
/*	
        user.userdefine |= DEF_ALLMSG;
        user.userdefine |= DEF_FRIENDMSG;
        user.userdefine |= DEF_SOUNDMSG;
        user.userdefine &= ~DEF_MAILMSG;
*/
        substitute_record(BBSHOME "/.PASSWDS", &user, sizeof(user), i);
    }
    printf("\n\033[1m%d \033[32mUsers Attribute Tranfered...\033[m\n", i);
    fclose(rec);
}
