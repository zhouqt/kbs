/*******
  从2003年1月21日和2月27日之间的版面数据转换到2月27日以后的版面数据结构
  注意如果在1月21日以前的数据先用cnv_bh.c
  ******/

#include "bbs.h"
#include "stdio.h"

struct boardheader2{                //old boardheader
        char filename[STRLEN];
        char BM[BM_LEN];
        char title[STRLEN];
        unsigned level;
        unsigned int nowid;
        unsigned int clubnum;
        unsigned int flag;
        unsigned int adv_club;
        char des[200];
};

struct boardheader3{                //new boardheader
        char filename[STRLEN];
        char BM[BM_LEN];
        char title[STRLEN];
        unsigned level;
        unsigned int nowid;
        unsigned int clubnum;
        unsigned int flag;
        unsigned int adv_club;
	time_t createtime;
	int toptitle;
	char ann_path[128];
        char des[200];
};

int main(int argc , char* argv[])
{
        FILE * fp,*fp2,*fp3;
        struct boardheader2 bh[MAXBOARD];
        struct boardheader3 bhnew[MAXBOARD];
	char buf[128];
	int len;
	char* ptr = NULL;

        int i;

	if(argc !=3 )
	{
		printf("usage: cnv_bh2 Old_BOARDS_FILE New_BOARDS_FILE\n");
		exit(0);
	}
        if((fp = fopen(argv[1],"r")) == NULL)
        {
                printf("open old board file %s failed!",argv[1]);
                exit(0);
        }

        if((fp2 = fopen(argv[2],"w")) == NULL)
        {
                printf("cant create newboards file %s!",argv[2]);
                exit(0);
        }

	if((fp3 = fopen(".Search","r")) == NULL)
	{
		printf("can't open .Search file!");
		exit(0);
	}

        memset((void*)bh,0,MAXBOARD*sizeof(struct boardheader2));
        memset((void*)bhnew,0,MAXBOARD*sizeof(struct boardheader3));

        fread((void*)bh,sizeof(struct boardheader2),MAXBOARD,fp);

        for(i = 0;i < MAXBOARD; i++)
        {
                strncpy(bhnew[i].filename,bh[i].filename,STRLEN);
                strncpy(bhnew[i].BM,bh[i].BM,BM_LEN);
                strncpy(bhnew[i].title,bh[i].title,STRLEN);
                bhnew[i].level = bh[i].level;
                bhnew[i].nowid = bh[i].nowid+1000;
                bhnew[i].clubnum = bh[i].clubnum;
                bhnew[i].flag = bh[i].flag;
		bhnew[i].adv_club = bh[i].adv_club;
		strncpy(bhnew[i].des,bh[i].des,200);
		bhnew[i].createtime = 0; //default
		bhnew[i].toptitle = 0; //none

		len = strlen(bh[i].filename);
		while(fgets(buf,sizeof(buf),fp3) != NULL)
		{
			if(strncasecmp(buf,bh[i].filename,len) == 0 && buf[len] == ':' && (	ptr = strtok(&buf[len+1],"\t\n"))!=NULL)
			{
				strncpy(bhnew[i].ann_path,ptr+8,128);  //dont'save groups/
				break;
			}
		}
		rewind(fp3);
        }

        fwrite(bhnew,sizeof(struct boardheader3),MAXBOARD,fp2);

        fclose(fp2);
        fclose(fp);
	fclose(fp3);
        return 0;
}

