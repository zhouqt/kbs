#include "bbs.h"
#include "permissions.h"

main()
{
    FILE *rec;
    int i = 0;
    struct userec user;

    rec = fopen("/home/bbs/.PASSWDS", "rb");

    printf("\033[1;31;5mUserLevel Records Transfering...\n\033[m");
    while (1) {
        if (fread(&user, sizeof(user), 1, rec) <= 0)
            break;
        i++;
        if (user.numlogins <= 0)
            continue;
        printf("%d %s\t\n", i, user.userid);
        user.userdefine[0] |= DEF_ALLMSG;
        user.userdefine[0] |= DEF_FRIENDMSG;
        user.userdefine[0] |= DEF_SOUNDMSG;
        user.userdefine[0] &= ~DEF_MAILMSG;
        substitute_record("/home/bbs/.PASSWDS", &user, sizeof(user), i);
    }
    printf("\n\033[1m%d \033[32mFriends Records Tranfered...\033[m\n", i);
    fclose(rec);
}
