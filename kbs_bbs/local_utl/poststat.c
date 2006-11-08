/*-------------------------------------------------------*/
/* util/poststat.c      ( NTHU CS MapleBBS Ver 2.36 )    */
/*-------------------------------------------------------*/
/* target : 统计今日、周、月、年热门话题                 */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------
#define DEBUG*/

#include <stdio.h>
#include <time.h>
#include "bbs.h"
#include "urlencode.c"
#define	DELETE

char *myfile[] = { "day", "week", "month", "year", "bless" };
int mycount[4] = { 7, 4, 12 };
int mytop[] = { 10, 50, 100, 100, 10 };
char *mytitle[] = { "日十大热门话题",
    "周五十大热门话题",
    "月百大热门话题",
    "年度百大热门话题",
    "日十大衷心祝福"
};


#define HASHSIZE 10240
#define TOPCOUNT 10000
#ifdef BLESS_BOARD
const char *surfix_bless[23] = {
	" ",
    "  \x1b[1;34m◆  ",
    "\x1b[1;32m┏\x1b[0;32m┴\x1b[1m┓",
    "\x1b[0;32m│\x1b[1m本\x1b[0;32m│",
    "\x1b[0;32m│\x1b[1m日\x1b[0;32m│",
    "\x1b[1;32m┗\x1b[0;32m┬\x1b[1m┛",
    "  \x1b[0;32m│  ",
    "\x1b[1;32m┏\x1b[0;32m┴\x1b[1m┓",
    "\x1b[0;32m│\x1b[1m十\x1b[0;32m│",
    "\x1b[0;32m│\x1b[1m大\x1b[0;32m│",
    "\x1b[1;32m┗\x1b[0;32m┬\x1b[1m┛",
    "  \x1b[1;34m◆  ",
    "  \x1b[1;34m◆  ",
    "\x1b[1;32m┏\x1b[0;32m┴\x1b[1m┓",
    "\x1b[0;32m│\x1b[1m深\x1b[0;32m│",
    "\x1b[0;32m│\x1b[1m情\x1b[0;32m│",
    "\x1b[1;32m┗\x1b[0;32m┬\x1b[1m┛",
    "  \x1b[32m│  ",
    "\x1b[1;32m┏\x1b[0;32m┴\x1b[1m┓",
    "\x1b[0;32m│\x1b[1m祝\x1b[0;32m│",
    "\x1b[0;32m│\x1b[1m福\x1b[0;32m│",
    "\x1b[1;32m┗\x1b[0;32m┬\x1b[1m┛",
    "  \x1b[1;34m◆  "
};
#endif

struct fileheader               /* This structure is used to hold data in */
 fh[1];

struct postrec_old {
    struct posttop pt;
    struct postrec_old *next;       /* next rec */
} *bucket[HASHSIZE], *blessbucket[HASHSIZE];


/* 100 bytes */
struct posttop top[TOPCOUNT], postlog;

bool force_refresh;


int hash(key)
unsigned int key;
{
    return key % HASHSIZE;
}


/* ---------------------------------- */
/* hash structure : array + link list */
/* ---------------------------------- */


void search(struct posttop *t)
{
    struct postrec_old *p, *q, *s;
    int i, found = 0;

    i = hash(t->groupid);
    q = NULL;

#ifdef BLESS_BOARD
    if (!strcasecmp(t->board, BLESS_BOARD))
        p = blessbucket[i];
    else
#endif
        p = bucket[i];
    while (p && (!found)) {
        if (p->pt.groupid == t->groupid && !strcmp(p->pt.board, t->board))
            found = 1;
        else {
            q = p;
            p = p->next;
        }
    }

    if (found) {
        p->pt.number += t->number;
        if (p->pt.date < t->date)  /* 取较近日期 */
            p->pt.date = t->date;
    } else {
        s = (struct postrec_old *) malloc(sizeof(struct postrec_old));
        memcpy(&(s->pt), t, sizeof(struct posttop));
        s->next = NULL;
        if (q == NULL)
#ifdef BLESS_BOARD
            if (!strcasecmp(t->board, BLESS_BOARD))
                blessbucket[i] = s;
            else
#endif
                bucket[i] = s;
        else
            q->next = s;
    }
}


int sort(struct postrec_old *pp, int count)
{
    int i, j;

    for (i = 0; i <= count; i++) {
        if (pp->pt.number > top[i].number) {
            if (count < TOPCOUNT - 1)
                count++;
            for (j = count - 1; j >= i; j--)
                memcpy(&top[j + 1], &top[j], sizeof(struct posttop));

            memcpy(&top[i], pp, sizeof(struct posttop));
            break;
        }
    }
    return count;
}


void load_stat(const char *fname)
{
    FILE *fp;

    if ((fp = fopen(fname, "r")) != NULL) {
        int count = fread(top, sizeof(struct posttop), TOPCOUNT, fp);

        fclose(fp);
        while (count)
            search(&top[--count]);
    }
}

/*
 * mytype 0 本日
 *        1 本周
 *        2 本月
 *        3 本年
 *        4 祝福榜
 */
void writestat(int mytype, struct postrec_old *dobucket[HASHSIZE])
{
    struct postrec_old *pp;
    FILE *fp;
    int i, j;
    char *p, curfile[40];

    /*
     * Haohmaru.99.11.20.检查是否已被删 
     */
	int fd;
    char dirfile[80];
    int real;
	fileheader_t fh;

    /*
     * Bigman.2000.8.28: 修改统计方式 
     */
    int m, n;
    char BoardName[100][BOARDNAMELEN];
    char buf[40];

    struct top_header curr_top[10];

/* ---------------------------------------------- */
    /*
     * sort top 100 issue and save results            
     */
    /*
     * ---------------------------------------------- 
     */

    memset(top, 0, sizeof(top));
    for (i = j = 0; i < HASHSIZE; i++) {
        for (pp = dobucket[i]; pp; pp = pp->next)
            j = sort(pp, j);
    }

    p = myfile[mytype];
    sprintf(curfile, "etc/posts/%s.0", p);
    if ((fp = fopen(curfile, "w")) != NULL) {
        fwrite(top, sizeof(struct posttop), j, fp);
        fclose(fp);
    }

    sprintf(curfile, "etc/posts/%s", p);
    if ((fp = fopen(curfile, "w")) != NULL) {
#ifdef BLESS_BOARD
        if (mytype == 4)
            fprintf(fp, "              \x1b[1;33m── \x1b[31m☆\x1b[33m☆\x1b[32m☆ \x1b[41;32m  \x1b[33m本日十大衷心祝福  \x1b[m\x1b[1;32m ☆\x1b[31m☆\x1b[33m☆ ──\x1b[m\n\n");
        else
#endif
            fprintf(fp, "                \033[34m-----\033[37m=====\033[41m 本%s \033[m=====\033[34m-----\033[m\n\n", mytitle[mytype]);

        i = mytop[mytype];
//        if (j > i)
//            j = i;
#ifdef  DEBUG
        printf("i : %d, j: %d \n", i, j);
#endif
        real = 0;

        memset(curr_top,0,(10*sizeof(struct top_header)));

        for (i = 0; i < j && real < mytop[mytype]; i++) {
            strcpy(buf, ctime(&top[i].date));
            buf[20] = NULL;
            p = buf + 4;

            sprintf(dirfile, "boards/%s/.DIR", top[i].board);
			if ((fd = open(dirfile, O_RDWR, 0644)) < 0)
				continue;

    		if( get_records_from_id(fd, top[i].groupid, &fh, 1, NULL) == 0 ){
				close(fd);
				continue;
			}
			close(fd);

                m = 0;
                for (n = 0; n < real; n++) {
                    if (!strcmp(top[i].board, BoardName[n]))
                        m++;
                }
#ifdef BLESS_BOARD
				if(mytype != 4){
#endif

                if (m >= 3)
                    continue;
#ifdef BLESS_BOARD
				}
#endif

                strcpy(BoardName[real], top[i].board);

            real++;


#ifdef BLESS_BOARD
            if (mytype == 4)
                fprintf(fp,
                        "                                            %s \x1b[1;31m%4d\x1b[0;37m人      %s\x1b[m\n"
                        "\x1b[1m第\x1b[31m%2d \x1b[37m名 \x1b[4%dm %-51.51s\x1b[m \x1b[1;33m%-12s%s\x1b[m\n",
                        p, top[i].number, surfix_bless[(real - 1) * 2], real, (real - 1) / 2 + 1, fh.title, fh.owner, surfix_bless[(real - 1) * 2 + 1]);
            else
#endif
            {
                fprintf(fp,"\033[37m第\033[31m%3d\033[37m 名 \033[37m信区 : \033[33m%-16s\033[37m【\033[32m%s\033[37m】"
                    "\033[36m%4d \033[37m人\033[35m%16s\n     \033[37m标题 : \033[44m\033[37m%-60.60s\033[m\n",
                    (!mytype?real:(i+1)),top[i].board,p,top[i].number,fh.owner,fh.title);

                /* etnlegend, 2006.05.28, 阅读十大 ... */
                if(!mytype&&real<=10){
                    curr_top[real-1].bid=getbid(top[i].board,NULL);
                    curr_top[real-1].gid=top[i].groupid;
                }

            }
        }

        if(!mytype){
            const struct boardheader *bh;
            char path[PATHLEN];
            int k;
            for(k=0;k<10;k++){
                if(!(bh=getboard(publicshm->top[k].bid)))
                    continue;
                snprintf(path,PATHLEN,"boards/%s/.TOP.%u",bh->filename,publicshm->top[k].gid);
                unlink(path);
            }
            setpublicshmreadonly(0);
            memcpy(publicshm->top,curr_top,(10*sizeof(struct top_header)));
            publicshm->top_version++;
            setpublicshmreadonly(1);
        }

#ifdef BLESS_BOARD
        if (mytype == 4)
            fprintf(fp, "                                                                         %s\x1b[m", surfix_bless[20]);
#endif
        fclose(fp);
    }
}

static int get_seccode_index(char prefix)
{
    int i;

    for (i = 0; i < SECNUM; i++) {
        if (strchr(seccode[i], prefix) != NULL)
            return i;
    }
    return -1;
}

void gen_sec_hot_subjects_xml(int mytype, struct postrec_old *dobucket[HASHSIZE], int secid, int j)
{
    FILE *fp;
    int i;
    char *p, curfile[40];

    /*
     * Haohmaru.99.11.20.检查是否已被删 
     */
	int fd;
    char dirfile[80];
    int real;
	fileheader_t fh;

    /*
     * Bigman.2000.8.28: 修改统计方式 
     */
    int m, n;
    char BoardName[100][BOARDNAMELEN];
    char url_buf[256];
	char xml_buf[256];
	const struct boardheader *bp;

	/* ---------------------------------------------- */
    /*
     * sort top 100 issue and save results            
     */
    /*
     * ---------------------------------------------- 
     */
    p = myfile[mytype];

    sprintf(curfile, "xml/%s_sec%d.xml", p, secid);
    if ((fp = fopen(curfile, "w")) != NULL) 
	{
		fprintf(fp, "<?xml version=\"1.0\" encoding=\"GBK\"?>\n");
		fprintf(fp, "<hotsubjects>\n");

        real = 0;
        for (i = 0; i < j && real < mytop[mytype]; i++) 
		{

			bp = getbcache(top[i].board);
			if (bp == NULL)
				continue;
			if ( secid != get_seccode_index(bp->title[0]) )
				continue;

			setbdir(DIR_MODE_NORMAL, dirfile, top[i].board);
			if ((fd = open(dirfile, O_RDWR, 0644)) < 0)
				continue;

    		if(get_records_from_id(fd, top[i].groupid, &fh, 1, NULL) == 0 )
			{
				close(fd);
				continue;
			}
			close(fd);

			m = 0;
			for (n = 0; n < real; n++) 
			{
				if (!strcmp(top[i].board, BoardName[n]))
					m++;
			}

			if (m >= 3)
				continue;

			strcpy(BoardName[real], top[i].board);

            real++;

			fprintf(fp, "<hotsubject>\n");
			fprintf(fp, "<title>%s</title>\n", encode_url(url_buf,encode_xml(xml_buf, fh.title, 
						sizeof(xml_buf)),sizeof(url_buf)));
			fprintf(fp, "<author>%s</author>\n", encode_url(url_buf,fh.owner,sizeof(url_buf)));
			fprintf(fp, "<board>%s</board>\n", encode_url(url_buf,top[i].board,sizeof(url_buf)));
			fprintf(fp, "<time>%ld</time>\n", top[i].date);
			fprintf(fp, "<number>%d</number>\n", top[i].number);
			fprintf(fp, "<groupid>%d</groupid>\n", top[i].groupid);
			fprintf(fp, "</hotsubject>\n");
        }
		fprintf(fp, "</hotsubjects>\n");

        fclose(fp);
    }
}

void gen_secs_hot_subjects_xml(int mytype, struct postrec_old *dobucket[HASHSIZE])
{
	int i,j;
    struct postrec_old *pp;

    memset(top, 0, sizeof(top));
    for (i = j = 0; i < HASHSIZE; i++) {
        for (pp = dobucket[i]; pp; pp = pp->next)
            j = sort(pp, j);
    }

	for(i=0; i<SECNUM; i++){
		gen_sec_hot_subjects_xml(mytype, dobucket, i, j);
	}
}

void gen_hot_subjects_xml(int mytype, struct postrec_old *dobucket[HASHSIZE])
{
    struct postrec_old *pp;
    FILE *fp;
    int i, j;
    char *p, curfile[40];

    /*
     * Haohmaru.99.11.20.检查是否已被删 
     */
	int fd;
    char dirfile[80];
    int real;
	fileheader_t fh;

    /*
     * Bigman.2000.8.28: 修改统计方式 
     */
    int m, n;
    char BoardName[100][BOARDNAMELEN];
	char xml_buf[256];
	char url_buf[256];

	/* ---------------------------------------------- */
    /*
     * sort top 100 issue and save results            
     */
    /*
     * ---------------------------------------------- 
     */

    memset(top, 0, sizeof(top));
    for (i = j = 0; i < HASHSIZE; i++) {
        for (pp = dobucket[i]; pp; pp = pp->next)
            j = sort(pp, j);
    }

    p = myfile[mytype];
	/*
    sprintf(curfile, "etc/posts/%s.0", p);
    if ((fp = fopen(curfile, "w")) != NULL) {
        fwrite(top, sizeof(struct posttop), j, fp);
        fclose(fp);
    }
	*/

    sprintf(curfile, "xml/%s.xml", p);
    if ((fp = fopen(curfile, "w")) != NULL) 
	{
		fprintf(fp, "<?xml version=\"1.0\" encoding=\"GBK\"?>\n");
		fprintf(fp, "<hotsubjects>\n");

        i = mytop[mytype];
        real = 0;
        for (i = 0; i < j && real < mytop[mytype]; i++) 
		{
			setbdir(DIR_MODE_NORMAL, dirfile, top[i].board);
			if ((fd = open(dirfile, O_RDWR, 0644)) < 0)
				continue;

    		if(get_records_from_id(fd, top[i].groupid, &fh, 1, NULL) == 0 )
			{
				close(fd);
				continue;
			}
			close(fd);

			m = 0;
			for (n = 0; n < real; n++) 
			{
				if (!strcmp(top[i].board, BoardName[n]))
					m++;
			}

			if (m >= 3)
				continue;

			strcpy(BoardName[real], top[i].board);

            real++;

			fprintf(fp, "<hotsubject>\n");
			fprintf(fp, "<title>%s</title>\n", encode_url(url_buf,encode_xml(xml_buf, fh.title, 
						sizeof(xml_buf)),sizeof(url_buf)));
			fprintf(fp, "<author>%s</author>\n", encode_url(url_buf,fh.owner,sizeof(url_buf)));
			fprintf(fp, "<board>%s</board>\n", encode_url(url_buf,top[i].board,sizeof(url_buf)));
			fprintf(fp, "<time>%ld</time>\n", top[i].date);
			fprintf(fp, "<number>%d</number>\n", top[i].number);
			fprintf(fp, "<groupid>%d</groupid>\n", top[i].groupid);
			fprintf(fp, "</hotsubject>\n");
        }
		fprintf(fp, "</hotsubjects>\n");

        fclose(fp);
    }
}

void gen_blessing_list_xml(struct postrec_old *dobucket[HASHSIZE])
{
    struct postrec_old *pp;
    FILE *fp;
    int i, j;
    char *p, curfile[40];

    /*
     * Haohmaru.99.11.20.检查是否已被删 
     */
	int fd;
    char dirfile[80];
    int real;
	fileheader_t fh;

    /*
     * Bigman.2000.8.28: 修改统计方式 
     */
    int m, n;
    char BoardName[100][BOARDNAMELEN];
	char xml_buf[256];
	char url_buf[256];
	int mytype = 4;

	/* ---------------------------------------------- */
    /*
     * sort top 100 issue and save results            
     */
    /*
     * ---------------------------------------------- 
     */

    memset(top, 0, sizeof(top));
    for (i = j = 0; i < HASHSIZE; i++) {
        for (pp = dobucket[i]; pp; pp = pp->next)
            j = sort(pp, j);
    }

    p = myfile[mytype];
	/*
    sprintf(curfile, "etc/posts/%s.0", p);
    if ((fp = fopen(curfile, "w")) != NULL) {
        fwrite(top, sizeof(struct posttop), j, fp);
        fclose(fp);
    }
	*/

    sprintf(curfile, "xml/%s.xml", p);
    if ((fp = fopen(curfile, "w")) != NULL) 
	{
		fprintf(fp, "<?xml version=\"1.0\" encoding=\"GBK\"?>\n");
		fprintf(fp, "<hotsubjects>\n");

        i = mytop[mytype];
        real = 0;
        for (i = 0; i < j && real < mytop[mytype]; i++) 
		{
			setbdir(DIR_MODE_NORMAL, dirfile, top[i].board);
			if ((fd = open(dirfile, O_RDWR, 0644)) < 0)
				continue;

    		if(get_records_from_id(fd, top[i].groupid, &fh, 1, NULL) == 0 )
			{
				close(fd);
				continue;
			}
			close(fd);

			m = 0;
			for (n = 0; n < real; n++) 
			{
				if (!strcmp(top[i].board, BoardName[n]))
					m++;
			}

			strcpy(BoardName[real], top[i].board);

            real++;

			fprintf(fp, "<hotsubject>\n");
			fprintf(fp, "<title>%s</title>\n", encode_url(url_buf,encode_xml(xml_buf, fh.title, 
						sizeof(xml_buf)),sizeof(url_buf)));
			fprintf(fp, "<author>%s</author>\n", encode_url(url_buf,fh.owner,sizeof(url_buf)));
			fprintf(fp, "<board>%s</board>\n", encode_url(url_buf,top[i].board,sizeof(url_buf)));
			fprintf(fp, "<time>%ld</time>\n", top[i].date);
			fprintf(fp, "<number>%d</number>\n", top[i].number);
			fprintf(fp, "<groupid>%d</groupid>\n", top[i].groupid);
			fprintf(fp, "</hotsubject>\n");
        }
		fprintf(fp, "</hotsubjects>\n");

        fclose(fp);
    }
}

void poststat(int mytype, time_t now, struct tm *ptime)
{
    static char *logfile = ".newpost";
    static char *oldfile = ".newpost.old";

    char buf[40], curfile[40] = "etc/posts/day.0";
    struct postrec_old *pp;
    struct postrec_old *qq;
    FILE *fp=NULL;
    int i;

    for (i = 0; i < HASHSIZE; i++) {
        bucket[i] = NULL;
    }
#ifdef BLESS_BOARD
    for (i = 0; i < HASHSIZE; i++) {
        blessbucket[i] = NULL;
    }
#endif

    if (mytype < 0) {
        /*
         * --------------------------------------- 
         */
        /*
         * load .post and statictic processing     
         */
        /*
         * --------------------------------------- 
         */

        remove(oldfile);
        rename(logfile, oldfile);
        if (!force_refresh)
            if ((fp = fopen(oldfile, "r")) == NULL)
                return;
        mytype = 0;
        load_stat(curfile);
#ifdef BLESS_BOARD
        load_stat("etc/posts/bless.0");
#endif

        if (fp != NULL) {
      	    while (fread(top, sizeof(struct posttop), 1, fp))
                search(top);
            fclose(fp);
        }
    } else {
        /*
         * ---------------------------------------------- 
         */
        /*
         * load previous results and statictic processing 
         */
        /*
         * ---------------------------------------------- 
         */

        char *p;

        i = mycount[mytype];
        p = myfile[mytype];
        while (i) {
            sprintf(buf, "etc/posts/%s.%d", p, i);
            sprintf(curfile, "etc/posts/%s.%d", p, --i);
            load_stat(curfile);
            //rename(curfile, buf);
        }
        mytype++;
    }

	if( mytype > 0 && mytype < 3){
		char *p;
		i = mycount[mytype];
        p = myfile[mytype];
        while (i) {
            sprintf(buf, "etc/posts/%s.%d", p, i);
            sprintf(curfile, "etc/posts/%s.%d", p, --i);
            rename(curfile, buf);
        }
	}

    /*
     * free statistics 
     */

    writestat(mytype, bucket);
	gen_hot_subjects_xml(mytype, bucket);
	gen_secs_hot_subjects_xml(mytype, bucket);
#ifdef BLESS_BOARD
    if (mytype == 0)
        writestat(4, blessbucket);
	gen_blessing_list_xml(blessbucket);
#endif
    for (i = 0; i < HASHSIZE; i++) {
		pp=bucket[i];
		while( pp ){
			qq=pp;
			pp=pp->next;
            free(qq);
		}
        bucket[i] = NULL;
    }
#ifdef BLESS_BOARD
    for (i = 0; i < HASHSIZE; i++) {
		pp=blessbucket[i];
		while(pp){
			qq=pp;
			pp=pp->next;
            free(qq);
		}
        blessbucket[i] = NULL;
    }
#endif
}


int main(int argc, char **argv)
{
    time_t now;
    struct tm ptime;
    int i;
    char buf[80], curfile[80] ;

	if (init_all()) {
		printf("init data fail\n");
		return -1;
	}
    time(&now);
    ptime = *localtime(&now);
    force_refresh = false;    /* 增加强制重新统计十大功能 pig2532 2006.04.06 */
    if (argc == 2) {
        if (strcmp(argv[1], "refresh") == 0) {
            force_refresh = true;
        }
        else {
            i = atoi(argv[1]);
            if (i != 0) {
                poststat(i, now, &ptime);
                return 0;
            }
        }
    }

    if ((ptime.tm_hour == 0) || force_refresh) {
		if (ptime.tm_yday == 1)
			poststat(2, now, &ptime);
        if (ptime.tm_mday == 1)
            poststat(1, now, &ptime);
        if (ptime.tm_wday == 0)
            poststat(0, now, &ptime);
		//unlink("etc/posts/day.0");
		i=7;
        while (i) {
            sprintf(buf, "etc/posts/day.%d", i);
            sprintf(curfile, "etc/posts/day.%d", --i);
            rename(curfile, buf);
		}
#ifdef BLESS_BOARD
        unlink("etc/posts/bless.0");
        post_file(NULL, "", "etc/posts/bless", BLESS_BOARD, "十大祝福", 0, 1, getSession());
#endif
    }
    poststat(-1, now, &ptime);
    if (ptime.tm_hour == 23) {
        char fname[STRLEN];

        sprintf(fname, "%d年%2d月%2d日十大热门话题", ptime.tm_year + 1900, ptime.tm_mon + 1, ptime.tm_mday);
        post_file(NULL, "", "etc/posts/day", "BBSLists", fname, 0, 1, getSession());
        if (ptime.tm_wday == 6) {
            sprintf(fname, "%d年%2d月%2d日本周五十大热门话题", ptime.tm_year + 1900, ptime.tm_mon + 1, ptime.tm_mday);
            post_file(NULL, "", "etc/posts/week", "BBSLists", fname, 0, 1, getSession());
        }
    }
    return 0;
}
