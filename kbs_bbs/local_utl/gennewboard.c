/*
* Build newboards xml file.
* $id$
*/
#include <stdio.h>
//#include <time.h>
#include "bbs.h"

#define	MAX_NEW_BOARD_NUM 100

static int load_boards2(struct newpostdata *nbrd,char *boardprefix,int group,int pos,int len,bool sort,bool yank_flag,const char** input_namelist)
{
    int n;
    const struct boardheader *bptr;
    int brdnum;
    struct newpostdata *ptr;
    int curcount;
    const char** namelist;
    const char** titlelist;
    int* indexlist;
	time_t tnow;

	tnow = time(0);
    brdnum = 0;
    curcount=0;
//    if (zapbuf == NULL) {
//        load_zapbuf();
//    }
    if (input_namelist==NULL)
        namelist=(const char**)malloc(sizeof(char**)*(pos+len-1));
    else
    	namelist=input_namelist;
    titlelist=(const char**)malloc(sizeof(char**)*(pos+len-1));
    indexlist=(int*)malloc(sizeof(int*)*(pos+len-1));
    for (n = 0; n < get_boardcount(); n++) {
        bptr = (struct boardheader *) getboard(n + 1);
        if (!bptr)
            continue;
        if (*bptr->filename==0)
            continue;
		if ( group == -2 ){ //ÐÂ°æ
			if( ( tnow - bptr->createtime ) > 86400*30 || ( bptr->flag & BOARD_GROUP ) )
				continue;
		}else if ((bptr->group!=group)&&!((boardprefix==NULL)&&(group==0)))
            continue;
        if (!check_see_perm(currentuser,bptr)) {
            continue;
        }
        if ((group==0)&&(boardprefix != NULL && strchr(boardprefix, bptr->title[0]) == NULL && boardprefix[0] != '*'))
            continue;
//        if (yank_flag || zapbuf[n] != 0 || (bptr->level & PERM_NOZAP)) {
        if (yank_flag  || (bptr->level & PERM_NOZAP)) {
            int i;
            int j;
            brdnum++;
            /*¶¼ÒªÅÅÐò*/
            for (i=0;i<curcount;i++) {
		    int type;
		    type = 0;

		    if (!sort) {
			type = titlelist[i][0] - bptr->title[0];
                        if (type == 0)
                            type = strncasecmp(&titlelist[i][1], bptr->title + 1, 6);
                    }
                    if (type == 0)
                        type = strcasecmp(namelist[i], bptr->filename);
		    if (type>0) break;
            }
            if ((i==curcount)&&curcount>=pos+len-1) /*ÒÑ¾­ÔÚ·¶Î§Ö®ÍâÀÖ*/
                continue;
            else
                for (j=(curcount>=pos+len-1)?pos+len-2:curcount;j>i;j--) {
                    namelist[j]=namelist[j-1];
                    titlelist[j]=titlelist[j-1];
                    indexlist[j]=indexlist[j-1];
                }
            namelist[i]=bptr->filename;
            titlelist[i]=bptr->title;
            indexlist[i]=n;
            if (curcount<pos+len-1) curcount++;
        }
    }
    if (nbrd) {
        for (n=pos-1;n<curcount;n++) {
            ptr=&nbrd[n-(pos-1)];
            bptr = getboard(indexlist[n]+1);
            ptr->dir = bptr->flag&BOARD_GROUP?1:0;
            ptr->name = bptr->filename;
            ptr->title = bptr->title;
            ptr->BM = bptr->BM;
            ptr->flag = bptr->flag | ((bptr->level & PERM_NOZAP) ? BOARD_NOZAPFLAG : 0);
            ptr->pos = indexlist[n];
            if (bptr->flag&BOARD_GROUP) {
                ptr->total = bptr->board_data.group_total;
            } else ptr->total=-1;
 //           ptr->zap = (zapbuf[indexlist[n]] == 0);
        }
    }
    free(titlelist);
    if (input_namelist==NULL)
        free(namelist);
    free(indexlist);
    return brdnum;
}


int gen_newboards_xml()
{
	FILE *fp = NULL;
	char* namelist[MAX_NEW_BOARD_NUM];
	struct newpostdata boarddata[MAX_NEW_BOARD_NUM];
	int boardcount,i;
	char title[STRLEN];

	if ((fp = fopen("xml/newboards.xml", "w")) == NULL) {
		printf("open xml/newboards.xml file failed.\n");
		return -1;
	}
	
	boardcount = load_boards2(&boarddata,NULL, -2, 1, MAX_NEW_BOARD_NUM, false, true, namelist);
	printf("boardcount=%d\n",boardcount);
	//return 0;
	//boardcount = 0;
	
		    
	fprintf(fp, "<?xml version=\"1.0\" encoding=\"GBK\"?>\n");
	fprintf(fp, "<newboards>\n");
	for(i = 0; i < boardcount; i++)
	{
		fprintf(fp, "<newboard>\n");
		fprintf(fp, "<filename>%s</filename>\n", boarddata[i].name);
		strcpy(title, boarddata[i].title);
		title[7] = 0;
		fprintf(fp, "<class>%s</class>\n", title);
		fprintf(fp, "<title>%s</title>\n", title+13);
		fprintf(fp, "<BM>%s</BM>\n", boarddata[i].BM);
		
		fprintf(fp, "<count>%d</count>\n", boarddata[i].total);
		fprintf(fp, "</newboard>\n");
        }

	fprintf(fp, "</newboards>\n");
	fclose(fp);
	return boardcount;
}

int main(int argc, char **argv)
{
    	chdir(BBSHOME);
	resolve_boards();
	gen_newboards_xml();
    	return 0;
}
