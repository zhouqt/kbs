/*****
stiger:    2004.2

记录版面在线人数到日志文件，用于以后统计方便吧

crontab:  2 * * * * /home/bbs/bin/bonlinelog

******/

/* mysql 
 *
 * +-------+-------------+------+-----+------------+----------------+
 * | Field | Type        | Null | Key | Default    | Extra          |
 * +-------+-------------+------+-----+------------+----------------+
 * | id    | int(10)     |      | PRI | NULL       | auto_increment |
 * | bname | varchar(20) |      |     |            |                |
 * | users | int(11)     |      |     | 0          |                |
 * | nowid | int(11)     |      |     | 0          |                |
 * | bdate | date        |      | MUL | 0000-00-00 |                |
 * | bhour | int(2)      |      |     | 0          |                |
 * +-------+-------------+------+-----+------------+----------------+


 CREATE TABLE bonline (
 id int(10) NOT NULL auto_increment,
 bname varchar(20) NOT NULL default '',
 users int(11) NOT NULL default '0',
 nowid int(11) NOT NULL default '0',
 bdate date NOT NULL default '0000-00-00',
 bhour int(2) NOT NULL default '0',
 PRIMARY KEY  (id),
 KEY bdate (bdate)
 ) TYPE=MyISAM COMMENT='bonline';
 */
 
#include <time.h>
#include <stdio.h>
#include "bbs.h"
#include "config.h"

MYSQL s;
struct tm t;

int fillbcache(struct boardheader *fptr,int idx,void* arg)
{

    struct boardheader bp;
	int bnum;
    struct BoardStatus * bs;
    struct userec normaluser;
	char sql[500];

    if (check_see_perm(NULL, fptr)==0 || strlen(fptr->filename) == 0)
        return;

    bnum = getboardnum(fptr->filename,&bp);
    bs = getbstatus(bnum);

	sprintf(sql, "INSERT INTO bonline VALUES ( NULL, '%s', '%d', '%d', \"%d-%d-%d\", '%d' );",
				fptr->filename, bs->currentusers, bs->nowid, 
				t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour);
    if( mysql_real_query( &s, sql, strlen(sql) ))
        printf("%s\n",mysql_error(&s));
    return 0;
}

int fillboard()
{
    apply_record(BOARDS, (APPLY_FUNC_ARG)fillbcache, sizeof(struct boardheader), NULL, 0,false);
}

main()
{
	time_t now;

    chdir(BBSHOME);
	now = time(0) - 3600;
	localtime_r( &now, &t);

	mysql_init(&s);
    if (!my_connect_mysql(&s) ){
        printf("Mysql connect error: %s\n",mysql_error(&s));
        return 0;
    }   
	
    resolve_boards();
    fillboard();

    mysql_close(&s);
    return 1;
}
