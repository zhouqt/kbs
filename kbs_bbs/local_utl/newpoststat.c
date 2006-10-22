/* 使用mysql的十大统计功能， 2004.5.25 stiger */

/*
   select bname,threadid,count(distinct userid) as count from postlog WHERE time<NOW() GROUP BY bname,threadid order by count desc;
*/
#include <stdio.h>
#include <time.h>
#include "bbs.h"
#include "urlencode.c"

#ifdef NEWPOSTSTAT

char *myfile[] = { "day", "week", "month", "year", "bless" };
int mytop[] = { 10, 50, 100, 100, 10 };
char *mytitle[] = { "日十大热门话题",
    "周五十大热门话题",
    "月百大热门话题",
    "年度百大热门话题",
    "日十大衷心祝福"
};

#define TOPCOUNT 100
#define SECTOPCOUNT 10
#define MAXCMP 10000
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

struct postrec {
	char userid[IDLEN+1];
    char board[BOARDNAMELEN];      /* board name */
	unsigned int groupid;
    time_t date;                /* last post's date */
    int number;                 /* post number */
	char title[81];
};

struct postrec top[TOPCOUNT];
struct postrec sectop[SECNUM][SECTOPCOUNT];

int sectopnum[SECNUM];
int sectopnumtotal=0;

bool force_refresh;

int topnum=0;

#define INTERVAL 200

static char * get_file_name(char *boardname, int threadid, char *fname)
{

	char dirfile[256];
	int fd;
	struct fileheader fh;

	fname[0]='\0';
            sprintf(dirfile, "boards/%s/.DIR", boardname);
			if ((fd = open(dirfile, O_RDWR, 0644)) < 0)
				return fname;

    		if( get_records_from_id(fd, threadid, &fh, 1, NULL) == 0 ){
				close(fd);
				return fname;
			}
			close(fd);

			strcpy(fname, fh.filename);
			return fname;
}

static char * get_file_title(char *boardname, int threadid, char *title, char *userid)
{

	char dirfile[256];
	int fd;
	struct fileheader fh;

            sprintf(dirfile, "boards/%s/.DIR", boardname);
			if ((fd = open(dirfile, O_RDWR, 0644)) < 0)
				return NULL;

    		if( get_records_from_id(fd, threadid, &fh, 1, NULL) == 0 ){
				close(fd);
				return NULL;
			}
			close(fd);

			strncpy(title, fh.title, 80);
			title[80]=0;

			strncpy(userid, fh.owner, IDLEN);
			userid[IDLEN]='\0';

			return title;
}

extern const char seccode[SECNUM][5];
static int get_seccode_index(char prefix)
{
    int i;

    for (i = 0; i < SECNUM; i++) {
        if (strchr(seccode[i], prefix) != NULL)
            return i;
    }
    return -1;
}

/*********记录十大信息到toplog表*********/
/*
create table `toplog` (
 `id` int unsigned NOT NULL auto_increment,
 `userid` char(15) NOT NULL default '',
 `bname` char(31) NOT NULL default '',
 `title` char(81) NOT NULL default '',
 `time` timestamp NOT NULL,
 `date` date NOT NULL,
 `topth` int NOT NULL default '1',
 `count` int NOT NULL default '0',
 `threadid` int unsigned NOT NULL default '0',
 PRIMARY KEY (`id`),
 KEY userid (`userid`),
 KEY bname(`bname`, `threadid`),
 KEY date(`date`),
 UNIQUE top (`date`,`topth`)
) TYPE=MyISAM COMMENT='toplog';
*/
int log_top()
{
	MYSQL s;
	char sqlbuf[500];
	char newtitle[161];
	int i;
	char newts[20];

	mysql_init (&s);

	if (! my_connect_mysql(&s) ){
		return 0;;
	}

	for(i=0;i<topnum;i++){
		
		mysql_escape_string(newtitle, top[i].title, strlen(top[i].title));

		sprintf(sqlbuf,"UPDATE toplog SET userid='%s',bname='%s',title='%s',count='%d',time='%s',threadid='%d' WHERE date=CURDATE() AND topth='%d';", top[i].userid, top[i].board, newtitle, top[i].number, tt2timestamp(top[i].date,newts), top[i].groupid, i+1);

		if( mysql_real_query( &s, sqlbuf, strlen(sqlbuf) )){
			printf("%s\n", mysql_error(&s));
			continue;
		}
		if( (int)mysql_affected_rows(&s) <= 0 ){
			sprintf(sqlbuf, "INSERT INTO toplog VALUES (NULL,'%s','%s','%s','%s',CURDATE(),'%d','%d','%d');",top[i].userid, top[i].board, newtitle, tt2timestamp(top[i].date,newts), i+1, top[i].number, top[i].groupid);
			if( mysql_real_query( &s, sqlbuf, strlen(sqlbuf) )){
				printf("%s\n", mysql_error(&s));
				continue;
			}
		}
	}

	mysql_close(&s);

	return 1;
}

/***********
  根据type得到十大列表,已经经过排序等一系列检查,可以直接输出
  type!=4的时候还得到分区十大
  *******/
int get_top(int type)
{

	MYSQL s;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char sqlbuf[500];
	char cmptime[200];
	int start=0;
	int i,secid;
	int threadid;
	char title[81];
	char userid[IDLEN+1];
	int m,n;
#ifdef BLESS_BOARD
	const struct boardheader *bh;
#endif

	topnum = 0;

	if(type < 0 || type > 4)
		return 0;

	mysql_init (&s);

	if (! my_connect_mysql(&s) ){
		return 0;;
	}

	if(type==0 || type==4){
		//sprintf(cmptime,"YEAR(time)=YEAR(CURDATE()) AND MONTH(time)=MONTH(CURDATE()) AND DAYOFMONTH(time)=DAYOFMONTH(CURDATE())");
		sprintf(cmptime,"time>curdate()");
	}else if(type==1){
		sprintf(cmptime,"YEAR(date)=YEAR(CURDATE()) AND WEEK(date)=WEEK(CURDATE())");
	}else if(type==2){
		sprintf(cmptime,"YEAR(date)=YEAR(CURDATE()) AND MONTH(date)=MONTH(CURDATE())");
	}else if(type==3){
		sprintf(cmptime,"YEAR(date)=YEAR(CURDATE())");
	}

	bzero(top, TOPCOUNT * sizeof(struct postrec));
	bzero(sectop, SECNUM * SECTOPCOUNT * sizeof(struct postrec));

	for(i=0;i<SECNUM;i++) sectopnum[i]=0;
	sectopnumtotal=0;

	while(1){
		if(type==4){
			if(topnum>=mytop[type])
				break;
		}else if(type==0){
			if(topnum>=mytop[type] && sectopnumtotal>=SECNUM*SECTOPCOUNT)
				break;
		}else{
			if(topnum >= mytop[type])
				break;
		}

		if(start > MAXCMP)
			break;

		if(type==0 || type==4)
			sprintf(sqlbuf,"SELECT bname,threadid,MAX(UNIX_TIMESTAMP(time)) AS maxtime,count(DISTINCT userid) AS count FROM postlog WHERE %s GROUP BY bname,threadid ORDER BY count desc LIMIT %d,%d;", cmptime, start, INTERVAL);
		else
			sprintf(sqlbuf,"SELECT bname,threadid,UNIX_TIMESTAMP(time),count,title,userid FROM toplog WHERE %s ORDER BY count desc LIMIT %d,%d",cmptime,start, INTERVAL);
		
		if( mysql_real_query( &s, sqlbuf, strlen(sqlbuf) )){
			printf("%s\n", mysql_error(&s));
			mysql_close(&s);
			
			return topnum;
		}

		res = mysql_store_result(&s);

		while(1){
			row = mysql_fetch_row(res);
			if(row==NULL)
				break;

			/***检查是否该计算十大***/
			bh = getbcache(row[0]);
			if(bh==NULL || bh->flag & BOARD_POSTSTAT){
				continue;
			}
			if(!normal_board(bh->filename))
				continue;
#ifdef BLESS_BOARD
			if(type==0){
				if ( ! strcasecmp(row[0], BLESS_BOARD) ){
					continue;
				}
			}else if(type==4){
				if ( strcasecmp(row[0], BLESS_BOARD) ){
					continue;
				}
			}
#endif

			secid= get_seccode_index(bh->title[0]);

			if(topnum >= mytop[type] && ( secid==-1 || sectopnum[secid] >= SECTOPCOUNT))
				continue;

			threadid = atoi(row[1]);
			if(type==0 || type==4){
				if(get_file_title(row[0], threadid, title, userid) == NULL){
					continue;
				}
			}else{
				strncpy(title, row[4], 80);
				title[80]=0;
				strncpy(userid, row[5], IDLEN);
				userid[IDLEN]=0;
			}
/**一个版面最多3个十大**/
			if(type==0){
                m = 0;
                for (n = 0; n < topnum; n++) {
                    if (!strcmp(row[0], top[n].board))
                        m++;
                }
				if(m>0)
					continue;

				/***分区十大里一个版面也最多3个***/
				if(secid!=-1){
                	m = 0;
                	for (n = 0; n < sectopnum[secid]; n++) {
                    	if (!strcmp(row[0], sectop[secid][n].board))
                        	m++;
                	}
					if(m>0)
						continue;
				}
			}


			/***先记录正常十大的值***/
			if(topnum < mytop[type]){

			strncpy(top[topnum].board, row[0], BOARDNAMELEN);
			top[topnum].board[BOARDNAMELEN-1]='\0';
			top[topnum].groupid = threadid;
			strncpy(top[topnum].title, title, 80);
			top[topnum].title[80]='\0';
			strncpy(top[topnum].userid, userid, IDLEN);
			top[topnum].userid[IDLEN]='\0';
			top[topnum].date = atol(row[2]);
			top[topnum].number = atoi(row[3]);

			topnum++;

			}

			/***计算分区十大***/
			if(type==0){

			i=secid;
			if(i!=-1){
				if( sectopnum[i] < SECTOPCOUNT){

			strncpy(sectop[i][sectopnum[i]].board, row[0], BOARDNAMELEN);
			sectop[i][sectopnum[i]].board[BOARDNAMELEN-1]='\0';
			sectop[i][sectopnum[i]].groupid = threadid;
			strncpy(sectop[i][sectopnum[i]].title, title, 80);
			sectop[i][sectopnum[i]].title[80]='\0';
			strncpy(sectop[i][sectopnum[i]].userid, userid, IDLEN);
			sectop[i][sectopnum[i]].userid[IDLEN]='\0';
			sectop[i][sectopnum[i]].date = atol(row[2]);
			sectop[i][sectopnum[i]].number = atoi(row[3]);

					sectopnum[i]++;
					sectopnumtotal++;
				}
			}

			}//type==0

			if(type==4){
				if(topnum>=mytop[type])
					break;
			}else if(type==0){
				if(topnum >= mytop[type] && sectopnumtotal >= SECNUM*SECTOPCOUNT)
					break;
			}else{
				if(topnum >= mytop[type])
					break;
			}

		}

		mysql_free_result(res);

		start += INTERVAL;

	}
	
	mysql_close(&s);

	return topnum;

}

/*
 * mytype 0 本日
 *        1 本周
 *        2 本月
 *        3 本年
 *        4 祝福榜
 */
void writestat(int mytype)
{
	int i;
	char buf[256];
	char *p;
	char curfile[256];
	FILE *fp;

    struct top_header curr_top[10];

    sprintf(curfile, "etc/posts/%s", myfile[mytype]);
    if ((fp = fopen(curfile, "w")) != NULL) {
#ifdef BLESS_BOARD
        if (mytype == 4)
            fprintf(fp, "              \x1b[1;33m── \x1b[31m☆\x1b[33m☆\x1b[32m☆ \x1b[41;32m  \x1b[33m本日十大衷心祝福  \x1b[m\x1b[1;32m ☆\x1b[31m☆\x1b[33m☆ ──\x1b[m\n"
                        "                                                                         %s\x1b[m\n", surfix_bless[1]);
        else
#endif
            fprintf(fp, "                \033[34m-----\033[37m=====\033[41m 本%s \033[m=====\033[34m-----\033[m\n\n", mytitle[mytype]);

        memset(curr_top,0,(10*sizeof(struct top_header)));

        for (i = 0; i < topnum; i++) {

            strcpy(buf, ctime(&top[i].date));
            buf[20] = NULL;
            p = buf + 4;


#ifdef BLESS_BOARD
            if (mytype == 4)
                fprintf(fp,
                        "                                            %s \x1b[1;31m%4d\x1b[0;37m人      %s\x1b[m\n"
                        "\x1b[1m第\x1b[31m%2d \x1b[37m名 \x1b[4%dm %-51.51s\x1b[m \x1b[1;33m%-12s%s\x1b[m\n",
                        p, top[i].number, surfix_bless[(i+1) * 2], i+1, (i) / 2 + 1, top[i].title, top[i].userid, surfix_bless[(i+1) * 2 + 1]);
            else
#endif
            {
                fprintf(fp,"\033[37m第\033[31m%3d\033[37m 名 \033[37m信区 : \033[33m%-16s\033[37m【\033[32m%s\033[37m】"
                    "\033[36m%4d \033[37m人\033[35m%16s\n     \033[37m标题 : \033[44m\033[37m%-60.60s\033[m\n",
                    (i+1),top[i].board,p,top[i].number,top[i].userid,top[i].title);

                /* etnlegend, 2006.05.28, 阅读十大 ... */
                if(!mytype&&i<10){
                    curr_top[i].bid=getbid(top[i].board,NULL);
                    curr_top[i].gid=top[i].groupid;
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
            fprintf(fp, "                                                                         %s\x1b[m", surfix_bless[22]);
#endif
        fclose(fp);
    }
}

int backup_top(){
	int i;
	char fname[256];
	char dirname[256];
	char aname[256];
	struct tm t;
	time_t tt;
	FILE *fp;

	printf("backup_top\n");
	tt=time(0);
	localtime_r(&tt, &t);
	sprintf(dirname, "bonlinelog/top/%04d%02d%02d/",1900+t.tm_year, t.tm_mon+1, t.tm_mday);
	sprintf(fname, "mkdir -p %s", dirname);
	system(fname);
	sprintf(fname, "%sindex.html", dirname);
	if((fp=fopen(fname, "w"))==NULL) return -1;
	printf("backup_t1op\n");
	for(i=0;i<topnum;i++){
		sprintf(fname,"boards/%s/%s",top[i].board, get_file_name(top[i].board, top[i].groupid, aname));
		if(aname[0]=='\0') continue;
		sprintf(aname, "cp -f %s %s/%d.txt", fname, dirname, i);
		system(aname);
		fprintf(fp,"<a href=%d.txt>%d:%s</a>\n",i,i+1,top[i].title);
	}
	fclose(fp);
    return 0;
}

void gen_sec_hot_subjects_xml(int mytype, int secid)
{
	FILE *fp;
	char curfile[256];
	char xml_buf[256];
	char url_buf[256];
	int i;

    sprintf(curfile, "xml/%s_sec%d.xml", myfile[mytype], secid);
    if ((fp = fopen(curfile, "w")) != NULL) 
	{
		fprintf(fp, "<?xml version=\"1.0\" encoding=\"GBK\"?>\n");
		fprintf(fp, "<hotsubjects>\n");

        for (i = 0; i < sectopnum[secid]; i++) 
		{

			fprintf(fp, "<hotsubject>\n");
			fprintf(fp, "<title>%s</title>\n", encode_url(url_buf,encode_xml(xml_buf, sectop[secid][i].title, 
						sizeof(xml_buf)),sizeof(url_buf)));
			fprintf(fp, "<author>%s</author>\n", encode_url(url_buf,sectop[secid][i].userid,sizeof(url_buf)));
			fprintf(fp, "<board>%s</board>\n", encode_url(url_buf,sectop[secid][i].board,sizeof(url_buf)));
			fprintf(fp, "<time>%ld</time>\n", sectop[secid][i].date);
			fprintf(fp, "<number>%d</number>\n", sectop[secid][i].number);
			fprintf(fp, "<groupid>%d</groupid>\n", sectop[secid][i].groupid);
			fprintf(fp, "</hotsubject>\n");
        }
		fprintf(fp, "</hotsubjects>\n");

        fclose(fp);
    }
}

void gen_secs_hot_subjects_xml(int mytype)
{
	int i;

	for(i=0; i<SECNUM; i++){
		gen_sec_hot_subjects_xml(mytype, i);
	}
}

void gen_blessing_list_xml()
{
	FILE *fp;
	char curfile[256];
	char xml_buf[256];
	char url_buf[256];
	int i;

    sprintf(curfile, "xml/%s.xml", myfile[4]);
    if ((fp = fopen(curfile, "w")) != NULL) 
	{
		fprintf(fp, "<?xml version=\"1.0\" encoding=\"GBK\"?>\n");
		fprintf(fp, "<hotsubjects>\n");

        for (i = 0; i < topnum; i++) 
		{
			fprintf(fp, "<hotsubject>\n");
			fprintf(fp, "<title>%s</title>\n", encode_url(url_buf,encode_xml(xml_buf, top[i].title, 
						sizeof(xml_buf)),sizeof(url_buf)));
			fprintf(fp, "<author>%s</author>\n", encode_url(url_buf,top[i].userid,sizeof(url_buf)));
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

void gen_hot_subjects_xml(int mytype)
{
	FILE *fp;
	char curfile[256];
	char xml_buf[256];
	char url_buf[256];
	int i;

	if(mytype==4) {
		gen_blessing_list_xml();
        return;
	}

    sprintf(curfile, "xml/%s.xml", myfile[mytype]);
    if ((fp = fopen(curfile, "w")) != NULL) 
	{
		fprintf(fp, "<?xml version=\"1.0\" encoding=\"GBK\"?>\n");
		fprintf(fp, "<hotsubjects>\n");

        for (i = 0; i < topnum; i++) 
		{
			fprintf(fp, "<hotsubject>\n");
			fprintf(fp, "<title>%s</title>\n", encode_url(url_buf,encode_xml(xml_buf, top[i].title, 
						sizeof(xml_buf)),sizeof(url_buf)));
			fprintf(fp, "<author>%s</author>\n", encode_url(url_buf,top[i].userid,sizeof(url_buf)));
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

void poststat(int mytype)
{
	get_top(mytype);
	writestat(mytype);
	if(mytype==0)
		backup_top();
	gen_hot_subjects_xml(mytype);
	if(mytype==0)
		gen_secs_hot_subjects_xml(mytype);

	if(mytype==0)
		log_top();
}


int main(int argc, char **argv)
{
    time_t now;
    struct tm ptime;
    int i;

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
            if (i != 0 || argv[1][0]=='0') {
                poststat(i);
                return 0;
            }
        }
    }

    if ((ptime.tm_hour == 0) || force_refresh) {
		if (ptime.tm_yday == 1)
			poststat(3);
        if (ptime.tm_mday == 1)
            poststat(2);
        if (ptime.tm_wday == 0)
            poststat(1);
    }

    poststat(0);
    poststat(4);

    if (ptime.tm_hour == 23) {
        char fname[STRLEN];

        sprintf(fname, "%d年%2d月%2d日十大热门话题", ptime.tm_year + 1900, ptime.tm_mon + 1, ptime.tm_mday);
        post_file(NULL, "", "etc/posts/day", "BBSLists", fname, 0, 1, getSession());
        sprintf(fname, "%d年%2d月%2d日十大祝福话题", ptime.tm_year + 1900, ptime.tm_mon + 1, ptime.tm_mday);
        post_file(NULL, "", "etc/posts/bless", "BBSLists", fname, 0, 1, getSession());
        if (ptime.tm_wday == 6) {
            sprintf(fname, "%d年%2d月%2d日本周五十大热门话题", ptime.tm_year + 1900, ptime.tm_mon + 1, ptime.tm_mday);
            post_file(NULL, "", "etc/posts/week", "BBSLists", fname, 0, 1, getSession());
        }
    }
    return 0;
}

#else
int main(int argc, char** argv)
{
    return 0;
}
#endif
