#include "bbs.h"

#define MAX 10000
#define MAXK 100
#define MAX_PEOPLE 100
#define MINB 30 //上榜要求的最少次数

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
    int t;
    int peoplet;
    char id[MAX_PEOPLE][IDLEN+2];
    int st[MAX_PEOPLE]; // 0 - 活着平民 1 - 死了平民 2 - 活着杀手 3 - 死了杀手 4 - 其他情况
};

struct statf *statlib;
int statt=0;

int getid(char * s)
{
    int i;
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

int main()
{
    FILE* fp;
    char buf[80];
    struct killer_record r;
    time_t now;
    int i,j,bt,gt,bwt,gwt;
    struct statf temp;
    double rr;
    chdir(BBSHOME);
    now = time(0);
    statlib = (struct statf*) malloc(MAX*sizeof(struct statf));
    if ((fp = fopen("service/.KILLERRESULT", "rb")) == NULL)
        return -1;
    while(!feof(fp)) {
        fread(&r, 1, sizeof(struct killer_record), fp);
        bt=0; gt=0;
        bwt=0; gwt=0;
        for(i=0;i<r.peoplet;i++) {
            j = getid(r.id[i]);
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
            j = getid(r.id[i]);
            switch(r.st[i]) {
                case 0:
                    rr=(double)bt/gwt;
                    break;
                case 1:
                    if(r.w==0)
                        rr=(double)bt/gt/10;
                    else
                        rr=(double)bt/gt/100;
                    break;
                case 2:
                    rr=(double)gt/bwt;
                    break;
                case 3:
                    if(r.w==0)
                        rr=(double)gt/bt/10;
                    else
                        rr=(double)gt/bt/100;
                    break;
                default:
                    continue;
            }
            statlib[j].score+=rr;
        }
    }
    fclose(fp);
    for(i=0;i<statt;i++)
        for(j=i+1;j<statt;j++)
            if(statlib[i].btime==0||(statlib[i].btime>0&&statlib[j].btime>0&&
                (double)statlib[i].bwtime/statlib[i].btime<(double)statlib[j].bwtime/statlib[j].btime)){
                memcpy(&temp, statlib+i, sizeof(struct statf));
                memcpy(statlib+i, statlib+j, sizeof(struct statf));
                memcpy(statlib+j, &temp, sizeof(struct statf));
            }
    fp=fopen("service/killer.1", "w");
    fprintf(fp, "=============江湖杀手榜=============\n");
    fprintf(fp, "%4s %-12s %4s %4s %6s\n", "名次", "杀手名", "命中", "出手", "绝杀率");
    j=0;
    for(i=0;i<statt;i++) {
        if(statlib[i].btime<MINB) continue;
        fprintf(fp, "%3d  %-12s %3d  %3d  %4.2lf%%  \n", j+1, statlib[i].id, statlib[i].bwtime, statlib[i].btime, (statlib[i].btime==0)?0.0:(double)statlib[i].bwtime/statlib[i].btime*100);
        j++;
        if(j>=MAXK) break;
    }
    fclose(fp);

    for(i=0;i<statt;i++)
        for(j=i+1;j<statt;j++)
            if(statlib[i].gtime==0||(statlib[i].gtime>0&&statlib[j].gtime>0&&
                (double)statlib[i].gwtime/statlib[i].gtime<(double)statlib[j].gwtime/statlib[j].gtime)){
                memcpy(&temp, statlib+i, sizeof(struct statf));
                memcpy(statlib+i, statlib+j, sizeof(struct statf));
                memcpy(statlib+j, &temp, sizeof(struct statf));
            }
    fp=fopen("service/killer.2", "w");
    fprintf(fp, "=============江湖捕快榜=============\n");
    fprintf(fp, "%4s %-12s %4s %4s %6s\n", "名次", "捕快名", "命中", "出手", "神捕率");
    j=0;
    for(i=0;i<statt;i++) {
        if(statlib[i].gtime<MINB*6) continue;
        fprintf(fp, "%3d  %-12s %3d  %3d  %4.2lf%%  \n", j+1, statlib[i].id, statlib[i].gwtime, statlib[i].gtime, (statlib[i].gtime==0)?0.0:(double)statlib[i].gwtime/statlib[i].gtime*100);
        j++;
        if(j>=MAXK) break;
    }
    fclose(fp);

    for(i=0;i<statt;i++)
        for(j=i+1;j<statt;j++)
            if(statlib[i].score<statlib[j].score) {
                memcpy(&temp, statlib+i, sizeof(struct statf));
                memcpy(statlib+i, statlib+j, sizeof(struct statf));
                memcpy(statlib+j, &temp, sizeof(struct statf));
            }
    fp=fopen("service/killer.3", "w");
    fprintf(fp, "=============江湖名人榜=============\n");
    fprintf(fp, "%4s %-12s  %8s  %8s\n", "名次", "尊姓大名", "累计积分", "名人等级");
    j=0;
    for(i=0;i<statt;i++) {
        if(statlib[i].score<=0.1) continue;
        strcpy(buf, "");
        if(statlib[i].score<100) strcpy(buf, "无名小卒");
        else if(statlib[i].score<1000) strcpy(buf, "碌碌无闻");
        else if(statlib[i].score<10000) strcpy(buf, "小有成就");
        else if(statlib[i].score<100000) strcpy(buf, "扬名天下");
        else if(statlib[i].score<1000000) strcpy(buf, "一代宗师");
        fprintf(fp, "%3d  %-12s   %6.2lf   %s\n", j+1, statlib[i].id, statlib[i].score/100, buf);
        j++;
        if(j>=MAXK) break;
    }
    fclose(fp);


    statt = 0;
    if ((fp = fopen("service/.KILLERRESULT", "rb"))==NULL)
        return -1;
    while(!feof(fp)) {
        fread(&r, 1, sizeof(struct killer_record), fp);
        if((int)(r.t/86400)!=(int)(now/86400)) continue;
        bt=0; gt=0;
        bwt=0; gwt=0;
        for(i=0;i<r.peoplet;i++) {
            j = getid(r.id[i]);
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
            j = getid(r.id[i]);
            switch(r.st[i]) {
                case 0:
                    rr=(double)bt/gwt;
                    break;
                case 1:
                    if(r.w==0)
                        rr=(double)bt/gt/10;
                    else
                        rr=(double)bt/gt/100;
                    break;
                case 2:
                    rr=(double)gt/bwt;
                    break;
                case 3:
                    if(r.w==0)
                        rr=(double)gt/bt/10;
                    else
                        rr=(double)gt/bt/100;
                    break;
                default:
                    continue;
            }
            statlib[j].score+=rr;
        }
    }
    fclose(fp);

    for(i=0;i<statt;i++)
        for(j=i+1;j<statt;j++)
            if(statlib[i].btime==0||(statlib[i].btime>0&&statlib[j].btime>0&&
                (double)statlib[i].bwtime/statlib[i].btime<(double)statlib[j].bwtime/statlib[j].btime)){
                memcpy(&temp, statlib+i, sizeof(struct statf));
                memcpy(statlib+i, statlib+j, sizeof(struct statf));
                memcpy(statlib+j, &temp, sizeof(struct statf));
            }
    fp=fopen("service/killer.4", "w");
    fprintf(fp, "===========今日江湖杀手榜===========\n");
    fprintf(fp, "%4s %-12s %4s %4s %6s\n", "名次", "杀手名", "命中", "出手", "绝杀率");
    j=0;
    for(i=0;i<statt;i++) {
        if(statlib[i].btime<MINB/6) continue;
        fprintf(fp, "%3d  %-12s %3d  %3d  %4.2lf%%  \n", j+1, statlib[i].id, statlib[i].bwtime, statlib[i].btime, (statlib[i].btime==0)?0.0:(double)statlib[i].bwtime/statlib[i].btime*100);
        j++;
        if(j>=MAXK) break;
    }
    fclose(fp);

    for(i=0;i<statt;i++)
        for(j=i+1;j<statt;j++)
            if(statlib[i].gtime==0||(statlib[i].gtime>0&&statlib[j].gtime>0&&
                (double)statlib[i].gwtime/statlib[i].gtime<(double)statlib[j].gwtime/statlib[j].gtime)){
                memcpy(&temp, statlib+i, sizeof(struct statf));
                memcpy(statlib+i, statlib+j, sizeof(struct statf));
                memcpy(statlib+j, &temp, sizeof(struct statf));
            }
    fp=fopen("service/killer.5", "w");
    fprintf(fp, "===========今日江湖捕快榜===========\n");
    fprintf(fp, "%4s %-12s %4s %4s %6s\n", "名次", "捕快名", "命中", "出手", "神捕率");
    j=0;
    for(i=0;i<statt;i++) {
        if(statlib[i].gtime<MINB) continue;
        fprintf(fp, "%3d  %-12s %3d  %3d  %4.2lf%%  \n", j+1, statlib[i].id, statlib[i].gwtime, statlib[i].gtime, (statlib[i].gtime==0)?0.0:(double)statlib[i].gwtime/statlib[i].gtime*100);
        j++;
        if(j>=MAXK) break;
    }
    fclose(fp);

    for(i=0;i<statt;i++)
        for(j=i+1;j<statt;j++)
            if(statlib[i].score<statlib[j].score) {
                memcpy(&temp, statlib+i, sizeof(struct statf));
                memcpy(statlib+i, statlib+j, sizeof(struct statf));
                memcpy(statlib+j, &temp, sizeof(struct statf));
            }
    fp=fopen("service/killer.6", "w");
    fprintf(fp, "===========今日江湖名人榜===========\n");
    fprintf(fp, "%4s %-12s  %8s  %8s\n", "名次", "尊姓大名", "累计积分", "名人等级");
    j=0;
    for(i=0;i<statt;i++) {
        if(statlib[i].score<=0.1) continue;
        strcpy(buf, "");
        if(statlib[i].score<100) strcpy(buf, "无名小卒");
        else if(statlib[i].score<1000) strcpy(buf, "碌碌无闻");
        else if(statlib[i].score<10000) strcpy(buf, "小有成就");
        else if(statlib[i].score<100000) strcpy(buf, "扬名天下");
        else if(statlib[i].score<1000000) strcpy(buf, "一代宗师");
        fprintf(fp, "%3d  %-12s   %6.2lf   %s\n", j+1, statlib[i].id, statlib[i].score/100, buf);
        j++;
        if(j>=MAXK) break;
    }
    fclose(fp);
    return 0;
}
