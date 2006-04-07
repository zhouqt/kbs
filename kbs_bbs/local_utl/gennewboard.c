/*
* Build newboards xml file.
* $id$
*/
#include <stdio.h>
//#include <time.h>
#include "bbs.h"
#include "urlencode.c"

#define	MAX_NEW_BOARD_NUM 100

struct newboarddata {
    struct boardheader *boards[MAX_NEW_BOARD_NUM];
    int nBoards;
};


int fill_newboards (struct boardheader *bptr, void* arg)
{
    time_t tnow;
    struct newboarddata *boarddata = (struct newboarddata *)arg;
	tnow = time(0);
    
	if( ( tnow - bptr->createtime ) > 86400*30 || ( bptr->flag & BOARD_GROUP ) )
		return 0;
    if (!check_see_perm(NULL,bptr))
        return 0;
    if (boarddata->nBoards < MAX_NEW_BOARD_NUM) {
        boarddata->boards[boarddata->nBoards] = bptr;
        boarddata->nBoards++;
    }
    return 0;
}

int compare_createtime(const void *a, const void *b) {
    struct boardheader *bptr1 = *(struct boardheader **)a;
    struct boardheader *bptr2 = *(struct boardheader **)b;
    return (bptr2->createtime - bptr1->createtime);
}

int gen_newboards_xml(struct newboarddata *boarddata)
{
	FILE *fp = NULL;
	int i;
	char title[STRLEN];
	char buf[128];

	if ((fp = fopen("xml/newboards.xml", "w")) == NULL) {
		printf("open xml/newboards.xml file failed.\n");
		return -1;
	}

	printf("boardcount=%d\n", boarddata->nBoards);
		    
	fprintf(fp, "<?xml version=\"1.0\" encoding=\"GBK\"?>\n");
	fprintf(fp, "<newboards>\n");
	for(i = 0; i < boarddata->nBoards; i++)
	{
		fprintf(fp, "<newboard>\n");
		
		fprintf(fp, "<filename>%s</filename>\n",encode_url(buf, boarddata->boards[i]->filename, sizeof(buf)));
		strcpy(title, boarddata->boards[i]->title);
		title[7] = 0;
		fprintf(fp, "<class>%s</class>\n", encode_url(buf, title, sizeof(buf)));
		fprintf(fp, "<title>%s</title>\n", encode_url(buf, title+13, sizeof(buf)));
		fprintf(fp, "<BM>%s</BM>\n", encode_url(buf, boarddata->boards[i]->BM, sizeof(buf)));

		fprintf(fp, "<count>%d</count>\n", -1);
		fprintf(fp, "</newboard>\n");
        }

	fprintf(fp, "</newboards>\n");
	fclose(fp);
	return boarddata->nBoards;
}

int main(int argc, char **argv)
{
    struct newboarddata boarddata;
    if (init_all()) {
        printf("init data fail\n");
        return -1;
    }
    boarddata.nBoards = 0;
    apply_boards(fill_newboards, &boarddata);
    qsort(boarddata.boards, boarddata.nBoards, sizeof(struct boardheader *), compare_createtime);
	gen_newboards_xml(&boarddata);
    	return 0;
}
