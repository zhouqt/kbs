#include "bbs.h"

char *curuserid;

struct libstruct {
    char id[STRLEN];
    char nick[NAMELEN];
    unsigned int data;
} *lib;
int total=0,flag=0;
int tm_mon, tm_mday, cur = 0; //for birthday

int show()
{
    int i;
    printf("            ");
    switch (flag) {
        case 0:
            printf("========== \033[1;36m上站次数排行榜\033[37;0m ==========\n\n");
            break;
        case 1:
            printf("========== \033[1;36m发文次数排行榜\033[37;0m ==========\n\n");
            break;
        case 2:
            printf("========== \033[1;36m上站时间排行榜\033[37;0m ==========\n\n");
            break;
        case 3:
            printf("========== \033[1;36m总表现积分排行榜\033[37;0m ==========\n\n");
            break;
    }
    printf("名次 代号            昵称                           ");
    if (flag==0||flag==1) printf("次数");
    else if (flag==2) printf("总时数");
    else printf("积分");
    printf("\n");
    printf("==== ======================================================\n");
    for (i=0; i<total; i++) {
        if (flag==2)
            printf("[%2d] %-15s %-30s %d小时%d分钟\n", i+1, lib[i].id, lib[i].nick, lib[i].data/3600, (lib[i].data/60)%60);
        else
            printf("[%2d] %-15s %-30s %d\n", i+1, lib[i].id, lib[i].nick, lib[i].data);
    }
    return 0;
}

int statit(struct userec *user, void *arg)
{
    int i,j,d;
    if (!strcmp(user->userid,"guest")) return 0;
    if (user->userid[0]<'A'||user->userid[0]>'z') return 0;
    switch (flag) {
        case 0:
            d = user->numlogins;
            break;
        case 1:
            d = user->numposts;
            break;
        case 2:
            d = user->stay;
            break;
        case 3:
            d = countexp(user);
            break;
        case 4: {
#ifdef HAVE_BIRTHDAY
            struct userdata aman;
            if (read_userdata(user->userid, &aman) == 0) {
                if (aman.birthmonth == tm_mon && aman.birthday == tm_mday) {
                    if (cur < total) {
                        strcpy(lib[cur].id, user->userid);
                        strcpy(lib[cur].nick, user->username);
                        cur++;
                    }
                }
            }
#endif
            return 1;
        }
        break;
        default:
            return 1;
    }
    for (i=0; i<total; i++)
        if (lib[i].data<d) {
            for (j=total-1;j>i;j--)
                memcpy(&lib[j],&lib[j-1],sizeof(struct libstruct));
            strcpy(lib[i].id, user->userid);
            strcpy(lib[i].nick, user->username);
            lib[i].data = d;
            break;
        }
    return 1;
}

int main(int argc,char **argv)
{
    if (argc<=2) {
        printf("usage: statguy login|post|stay|all|birthday <total>\n");
        return -1;
    }
    if (!strcmp(argv[1], "login")) flag = 0;
    if (!strcmp(argv[1], "post")) flag = 1;
    if (!strcmp(argv[1], "stay")) flag = 2;
    if (!strcmp(argv[1], "all")) flag = 3;
    if (!strcmp(argv[1], "birthday")) {
        time_t now;
        struct tm *tmnow;
        flag = 4;
        now = time(0);
        now += 86400;  /* 直接算明天比较准啦! +1 不准 */
        tmnow = localtime(&now);
        tm_mon = tmnow->tm_mon + 1;
        tm_mday = tmnow->tm_mday;
    }
    total=atoi(argv[2]);
    if (!total) {
        printf("error: <total> must be an integer and greater than zero!\n");
        return -2;
    }
    chdir(BBSHOME);
    resolve_ucache();
    resolve_boards();
    lib = malloc(sizeof(struct libstruct)*total);
    memset(lib, 0, sizeof(struct libstruct)*total);
    apply_users(statit, NULL);
    if (flag == 4) {
        int i;
        printf("\n%s明日寿星名表\n\n", NAME_BBS_CHINESE);
        printf("以下是 %d 月 %d 日的寿星:\n\n",tm_mon, tm_mday);
        for (i=0; i<cur; i++) {
            printf(" ** %-15.15s (%s)\n", lib[i].id, lib[i].nick);
        }
        printf("\n\n总共有 %d 位寿星。\n", cur);
    } else
        show();
    return 0;
}
