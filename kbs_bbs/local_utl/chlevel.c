#include "bbs.h"
#include "../permissions.h"

main()
{
        FILE *rec;
        int i=0;
        struct userec user;

        rec=fopen("/home/bbs/.PASSWDS","rb");

        printf("[1;31;5mUserLevel Records Transfering...\n[m");
        while(1)
        {
                if(fread(&user,sizeof(user),1,rec)<=0) break;
                i++;
                if(user.numlogins<=0)
                        continue;
                printf("%d %s\t\n",i,user.userid);
                user.userdefine|=DEF_ALLMSG;
                user.userdefine|=DEF_FRIENDMSG;
                user.userdefine|=DEF_SOUNDMSG;
                user.userdefine&=~DEF_MAILMSG;
                substitute_record("/home/bbs/.PASSWDS",&user,
                                         sizeof(user),i);
        }
        printf("\n[1m%d [32mFriends Records Tranfered...[m\n",i);
        fclose(rec);
}
