#include "bbs.h"

#define MAX 10000
#define MAXK 100

struct statf {
    char id[IDLEN+2];
    int btime;
    int bwtime;
    int gtime;
    int gwtime;
    double score;
};

struct killer_record {
    int w; //0 - 平民胜利 1 - 杀手胜利
    time_t t;
    int peoplet;
    char id[MAX_PEOPLE][IDLEN+2];
    int st[MAX_PEOPLE]; // 0 - 活着平民 1 - 死了平民 2 - 活着杀手 3 - 死了杀手 4 - 其他情况
};

struct statf *statlib;
int statt=0;

int getid(char * s)
{
    int i,j;
    for(i=0;i<statt;i++)
        if(!strcmp(s, statlib[i].id)) return i;
    strcpy(statlib[statt].id, s);
    statlib[statt].btime = 0;
    statlib[statt].bwtime = 0;
    statlib[statt].gtime = 0;
    statlib[statt].gwtime = 0;
    statlib[statt].score = 0;
    statt++;
    return statt-1;
}

void main()
{
    FILE* fp;
    struct killer_record r;
    int i,j,k,bt,gt,bwt,gwt;
    struct statf temp;
    double rr;
    chdir(BBSHOME);
    statlib = (struct statf*) malloc(MAX*sizeof(struct statf));
    fp = fopen("service/.KILLERRESULT", "rb");
    while(!feof(fp)) {
        fread(&r, 1, sizeof(struct killer_record), fp);
        bt=0; gt=0;
        bwt=0; gwt=0;
        for(i=0;i<r.peoplet;i++) {
            j = getid(r.id);
            switch(r.st[i]) {
                case 0:
                    statlib[j].gwtime++;
                    gwt++;
                case 1:
                    statlib[j].gtime++;
                    gt++;
                    break;
                case 2:
                    statlib[j].bwtime++;
                    bwt++;
                case 3:
                    statlib[j].btime++;
                    bt++;
                    break;
            }
        }
        for(i=0;i<r.peoplet;i++) {
            j = getid(r.id);
            switch(r.st[i]) {
                case 0:
                    rr=bt/gwt;
                    break;
                case 1:
                    if(r.w==0)
                        rr=bt/gt/10;
                    else
                        rr=bt/gt/100;
                    break;
                case 2:
                    rr=gt/bwt;
                    break;
                case 3:
                    if(r.w==0)
                        rr=gt/bt/10;
                    else
                        rr=gt/bt/100;
                    break;
            }
            statlib[j].score+=rr;
        }
    }

    for(i=0;i<statt;i++)
        for(j=i+1;j<statt;j++)
            if(statlib[i].btime==0||statlib[i].btime>0&&statlib[j].btime>0&&
                statlib[i].bwtime/statlib[i].btime>statlib[j].bwtime/statlib[j].btime) {
                memcpy(&temp, statlib+i, sizeof(struct statf));
                memcpy(statlib+i, statlib+j, sizeof(struct statf));
                memcpy(statlib+j, &temp, sizeof(struct statf));
            }
    printf("========江湖十大杀手榜========\n");
    printf("%4s %-12s  %8s  %8s %6s\n", "名次", "杀手名", "命中次数", "出手次数", "绝杀率");
    for(i=0;i<MAXK;i++) {
        printf("%3d  %-12s  %6d    %6d   %4.2lf%%  \n", i+1, statlib[i].id, statlib[i].bwtime, statlib[i].btime, statlib[i].bwtime/statlib[i].btime*100);
    }
}
