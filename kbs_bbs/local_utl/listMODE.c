#include "bbs.h"

#define unknown "去了那里!?"

int count[200],order[200],total=0;

int check(struct user_info *uentp, char *u)
{
    count[uentp->mode]++;
    total++;
    return 0;
}

int main(int argc, char **argv)
{
    int i,j,k;
    char * s;
    chdir(BBSHOME);
    resolve_boards();
    resolve_ucache();
    resolve_utmp();
    memset(count, 0, 200*sizeof(int));
    apply_ulist_addr((APPLY_UTMP_FUNC)check,NULL);
    for(i=0;i<200;i++)order[i]=i;
    for(i=0;i<200;i++)
        for(j=i+1;j<200;j++)
        if(count[order[i]]<count[order[j]]) {
            k=order[i];
            order[i]=order[j];
            order[j]=k;
        }
    j=0;
    for(i=0;i<200;i++) {
        s = ModeType(order[i]);
        if(s[0]&&strcmp(s,unknown)) {
            j++;
            printf("%2d  %10s  %4d  %.2lf%%\n", j, s, count[order[i]], (double)count[order[i]]/total*100);
        }
    }
    return 0;
}

