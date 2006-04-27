/*****
stiger:    2004.2

记录版面在线人数到日志文件，用于以后统计方便吧

crontab:  2 * * * * /home/bbs/bin/bonlinelog

******/

/* mysql 
 *
 CREATE TABLE bonline (
 id int(10) NOT NULL auto_increment,
 bname varchar(20) NOT NULL default '',
 users int(11) NOT NULL default '0',
 nowid int(11) NOT NULL default '0',
 bdate date NOT NULL default '0000-00-00',
 bhour int(2) NOT NULL default '0',
 min int(2) NOT NULL default '0',
 PRIMARY KEY  (id),
 KEY bdate (bdate)
 ) TYPE=MyISAM COMMENT='bonline';
 */
 
#include <time.h>
#include <stdio.h>
#include "bbs.h"

#ifdef HAVE_MYSQL_SMTH
MYSQL s;
struct tm t;

int fillbcache(struct boardheader *fptr,int idx,void* arg)
{
	int bnum;
    struct BoardStatus * bs;
	char sql[500];

    if (check_see_perm(NULL, fptr)==0 || strlen(fptr->filename) == 0)
        return 0;

    bnum = getbid(fptr->filename,NULL);
    bs = getbstatus(bnum);

	sprintf(sql, "INSERT INTO bonline VALUES ( NULL, '%s', '%d', '%d', \"%d-%d-%d\", '%d', '%d' );",
				fptr->filename, bs->currentusers, bs->nowid, 
				t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min/6);
    if( mysql_real_query( &s, sql, strlen(sql) ))
        printf("%s\n",mysql_error(&s));
    return 0;
}

int fillboard(void){
    return apply_record(BOARDS, (APPLY_FUNC_ARG)fillbcache, sizeof(struct boardheader), NULL, 0,false);
}

int main(void){
	time_t now;

    chdir(BBSHOME);
	now = time(0);
	localtime_r( &now, &t);

	mysql_init(&s);
    if (!my_connect_mysql(&s) ){
        printf("Mysql connect error: %s\n",mysql_error(&s));
        return 0;
    }   
	
    resolve_boards();
    fillboard();

    mysql_close(&s);
    return 0;
}
#else
int main(void){
	fprintf(stderr, "MySQL support had been disabled.\n");
	return -1;
}
#endif
