#include "bbs.h"

struct userec deliveruser;

int main(int argc, char **argv)
{
    FILE *fp, *fp2;
    char fn[80],fn2[80];
    char * p;
    int i,j,ip[4],t,k,count=0;
    time_t tt;
    chdir(BBSHOME);
    resolve_boards();
    resolve_ucache();
    strcpy(fn, ".IPdenys");
    strcpy(fn2, "tmp/showipdeny.txt");
    fp=fopen(fn, "r");
    if(fp) {
        fp2=fopen(fn2, "w");
        while(!feof(fp)) {
            if(fscanf(fp, "%d %ld %d.%d.%d.%d %d", &i, &j, &ip[0], &ip[1], &ip[2], &ip[3], &t)<=0) break;
            if(t<=10) continue;
            count++;
            tt=(time_t) j;
            p = ctime(&tt);
            p[19]=0; p+=4;
            if(i==0)
                fprintf(fp2, "%s 来自 %d.%d.%d.%d 两次连接时间太短.一小时内共连接%d次.\n", p, ip[0],ip[1],ip[2],ip[3],t+1);
            else
                fprintf(fp2, "%s 来自 %d.%d.%d.%d 连接过于频繁.    一小时内共连接%d次.\n", p, ip[0],ip[1],ip[2],ip[3],t);
        }
        fclose(fp);
        fclose(fp2);
        unlink(fn);
        if(count==0) {
            unlink(fn2);
            return;
        }
        bzero(&deliveruser, sizeof(struct userec));
        strcpy(deliveruser.userid, "deliver");
        deliveruser.userlevel = -1;
        strcpy(deliveruser.username, "自动发信系统");
        currentuser = &deliveruser;
        strcpy(fromhost, "天堂");
        tt = time(0);
        p = ctime(&tt);
        p[19]=0; p+=4;
        sprintf(fn, "%s的犯罪记录", p);
        post_file(&deliveruser, NULL, fn2, "surrender", fn, 0, 1);
        unlink(fn2);
    }
    return 0;
}
