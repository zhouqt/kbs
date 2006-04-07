#include "bbs.h"

#ifndef NEWBMLOG

char *curuserid;
int type=-1, flag=0, timed=0, sorttype=0;
char groupid=0;

#define DATALEN 100

char suffix[4][10]={"", "week", "month", "year"};

struct libtruct {
	char boardname[STRLEN];
	char id[STRLEN];
	int data[DATALEN];
} *lib;
int total=0;

int order[10000];

int show(int k,int i)
{
    printf("序号:%-4d  ", k+1);
    if(lib[i].data[0]>=3600)
    	printf("版面名称: %-10s  版主: %-15s  停留时间: %d 小时 %d 分钟 %d 秒\n", lib[i].boardname, lib[i].id, lib[i].data[0]/3600, lib[i].data[0]/60%60, lib[i].data[0]%60);
    else if(lib[i].data[0]>=60)
    	printf("版面名称: %-10s  版主: %-15s  停留时间: %d 分钟 %d 秒\n", lib[i].boardname, lib[i].id, lib[i].data[0]/60, lib[i].data[0]%60);
    else
    	printf("版面名称: %-10s  版主: %-15s  停留时间: %d 秒\n", lib[i].boardname, lib[i].id, lib[i].data[0]);
    printf("   进版次数: %-4d    版内发文: %-4d    收入文摘: %-4d    去掉文摘: %-4d\n", lib[i].data[1], lib[i].data[2], lib[i].data[3], lib[i].data[4]);
    printf("   区段次数: %-4d    标记文章: %-4d    去掉标记: %-4d    删除文章: %-4d\n", lib[i].data[5], lib[i].data[6], lib[i].data[7], lib[i].data[8]);
    printf("   恢复删除: %-4d    封禁人数: %-4d    解封人数: %-4d    收入精华: %-4d\n", lib[i].data[9], lib[i].data[10], lib[i].data[11], lib[i].data[12]);
    printf("   整理精华: %-4d    相同主题: %-4d\n", lib[i].data[13], lib[i].data[14]);
    printf("\n");
    return 0;
}

void sort()
{
	int i,j,k;
	for(i=0;i<total;i++)
		order[i]=i;
	switch(sorttype){
		case 0:
			for(i=0;i<total;i++)
				for(j=i+1;j<total;j++)
				if(lib[order[i]].data[0]<lib[order[j]].data[0]){
					k=order[i];
					order[i]=order[j];
					order[j]=k;
				}
			break;
		case 1:
			for(i=0;i<total;i++)
				for(j=i+1;j<total;j++)
				if(strcasecmp(lib[order[i]].id, lib[order[j]].id)>0){
					k=order[i];
					order[i]=order[j];
					order[j]=k;
				}
			break;
	}
}

void showall()
{
	int i;
	for(i=0;i<total;i++)
		show(i, order[i]);
}

int check_BM(struct boardheader *bptr,void* arg)
{
    int i, fd, data[DATALEN];
    struct flock ldata;
    struct stat buf;
    char direct[PATHLEN];
    
//    if ((bptr->level != 0) && !(bptr->level & PERM_POSTMASK))
//        return 0;
    if (!chk_BM_instr(bptr->BM, curuserid)) return 0;
    if (groupid!=0&&bptr->title[0]!=groupid) return 0;
    sprintf(direct, "boards/%s/.bm.%s%s", bptr->filename, curuserid, suffix[timed]);
    if ((fd = open(direct, O_RDWR | O_CREAT, 0644)) == -1) return 0;
    ldata.l_type = F_RDLCK;
    ldata.l_whence = 0;
    ldata.l_len = 0;
    ldata.l_start = 0;
    if (fcntl(fd, F_SETLKW, &ldata) == -1) {
        close(fd);
        return 0;
    }
    fstat(fd, &buf);
    if(buf.st_size<DATALEN*sizeof(int)){
       	memset(data, 0, sizeof(int)*DATALEN);
    }
    else
       	read(fd, data, sizeof(int)*DATALEN);
    if (flag==1){
    	for(i=0;i<DATALEN;i++)
    		data[i]+=lib[total].data[i];
    	total++;
        lseek(fd, 0, SEEK_SET);
        write(fd, data, sizeof(int)*DATALEN);
    }
    else if (flag==3) {
    	for(i=0;i<DATALEN;i++)
    		data[i]=lib[total].data[i];
    	total++;
        lseek(fd, 0, SEEK_SET);
        write(fd, data, sizeof(int)*DATALEN);
    }
   	ldata.l_type = F_UNLCK;
   	fcntl(fd, F_SETLKW, &ldata);
   	close(fd);
   	
   	if (flag==-1) {
   		unlink(direct);
   	}
   	else if (flag==0)
   		total++;
   	else if (flag==1){
   	}
   	else if (flag==2){
   		strcpy(lib[total].boardname, bptr->filename);
   		strcpy(lib[total].id, curuserid);
   		memcpy(lib[total].data, data, sizeof(int)*DATALEN);
   		total++;
   	}
    
    return 0;
}

int query_BM(struct userec *user, void *arg)
{
    if (!(user->userlevel & PERM_BOARDS))
        return 0;
    curuserid = user->userid;
    return apply_boards(check_BM,NULL);
}

int main(int argc,char **argv){
	time_t t;
	struct tm res;
	if (argc<=2) {
		printf("usage: statBM group day|week|month|year|update|clear\n");
		return -2;
	}
        if(argv[1][1]==0) groupid = argv[1][0];
	if (!strcasecmp(argv[2],"clear")) type=0;
	if (!strcasecmp(argv[2],"day")) type=1;
	if (!strcasecmp(argv[2],"week")) type=2;
	if (!strcasecmp(argv[2],"month")) type=3;
	if (!strcasecmp(argv[2],"year")) type=4;
	if (!strcasecmp(argv[2],"update")) type=5;
	if (argc>=4&&!strcasecmp(argv[3],"id")) sorttype=1;
	if (type==-1) {
		printf("usage: statBM group day|week|month|year|update|clear\n");
		return -1;
	}
    chdir(BBSHOME);
    resolve_ucache();
    resolve_boards();
    switch(type) {
    	case 0:
    		timed=0;
    		flag=-1;
		    apply_users(query_BM, NULL);
    		timed=1;
    		flag=-1;
		    apply_users(query_BM, NULL);
    		timed=2;
    		flag=-1;
		    apply_users(query_BM, NULL);
    		timed=3;
    		flag=-1;
		    apply_users(query_BM, NULL);
    		break;
    	case 1:
    	case 2:
    	case 3:
    	case 4:
    		timed=type-1;
    		flag=0;
		    apply_users(query_BM, NULL);
		    lib = malloc(sizeof(struct libtruct)*total);
		    total=0;
    		flag=2;
		    apply_users(query_BM, NULL);
    		sort();
    		showall();
    		break;
    	case 5:
    		t=time(0);
    		gmtime_r(&t,&res);
    		flag=0;
		    apply_users(query_BM, NULL);
		    lib = malloc(sizeof(struct libtruct)*total);
		    total=0;
    		flag=2;
		    apply_users(query_BM, NULL);
		    
		    timed=1;
		    total=0;
		    if (!res.tm_wday) flag=3;
		    else flag=1;
		    apply_users(query_BM, NULL);
		    
		    timed=2;
		    total=0;
		    if (res.tm_mday==1) flag=3;
		    else flag=1;
		    apply_users(query_BM, NULL);
		    
		    timed=3;
		    total=0;
		    if (!res.tm_yday) flag=3;
		    else flag=1;
		    apply_users(query_BM, NULL);
		    
		    timed=0;
		    total=0;
		    flag=-1;
		    apply_users(query_BM, NULL);
		    break;
    }
    return 0;
}
#else

int main()
{
	return 0;
}
#endif
