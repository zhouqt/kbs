#include "bbs.h"
#include <sys/types.h>

#ifdef PERSONAL_CORP
#include <mysql.h>
#endif

#if 0 //buggy unless mysql ver < 4.1
time_t timestamp2tt( char *row )
{

	char ts[10];
	struct tm tetime;

	if( ! row || strlen(row)<14)
		return 0;

	strncpy(ts, row, 4);
	ts[4]=0;
	if(atoi(ts) >= 1900 )
		tetime.tm_year = atoi(ts) - 1900;
	else
		tetime.tm_year = 0;

	strncpy(ts, row+4, 2);
	ts[2]=0;
	tetime.tm_mon = atoi(ts)-1;

	strncpy(ts, row+6, 2);
	ts[2]=0;
	tetime.tm_mday = atoi(ts);

	strncpy(ts, row+8, 2);
	ts[2]=0;
	tetime.tm_hour = atoi(ts);

	strncpy(ts, row+10, 2);
	ts[2]=0;
	tetime.tm_min = atoi(ts);

	strncpy(ts, row+12, 2);
	ts[2]=0;
	tetime.tm_sec = atoi(ts);

	return mktime(&tetime);

}
#endif

char * tt2timestamp( time_t tt, char *c)
{
	struct tm t;

	if( tt <= 0 || ! localtime_r(&tt, &t) ){
		*c=0;
		return c;
	}

	sprintf(c, "%d%s%d%s%d%s%d%s%d%s%d", t.tm_year+1900, (t.tm_mon>8)?"":"0", t.tm_mon+1, (t.tm_mday>9)?"":"0", t.tm_mday, (t.tm_hour>9)?"":"0", t.tm_hour, (t.tm_min>9)?"":"0", t.tm_min, (t.tm_sec>9)?"":"0", t.tm_sec);

	return c;
}

#ifdef PERSONAL_CORP

int pc_conv_file_to_body( char **body, char *fname)
{
	char *ptr;
	off_t size;
	int fd;

	if( (fd=open(fname, O_RDONLY)) == -1)
		return 0;

	if(*body)
		free(*body);
	*body = NULL;

	BBS_TRY{
		if(safe_mmapfile_handle(fd, PROT_READ, MAP_SHARED, &ptr, &size) == 1){
			(*body) = (char *)malloc( size+1 );
			if((*body) != NULL){
				char* data;
				long not;
				int matched=0;
				long bodylen=0;

				data=ptr;
            	for (not=0;not<size;not++,data++) {
                	if (*data==0) {
                    	matched++;
                    	if (matched==ATTACHMENT_SIZE) {
                        	int d;
							long attsize;
                        	data++; not++;
                        	while(*data){
								data++;
								not++;
							}
                        	data++;
                        	not++;
                        	memcpy(&d, data, 4);
                        	attsize = htonl(d);
                        	data+=4+attsize-1;
                        	not+=4+attsize-1;
                        	matched = 0;
                    	}
                    	continue;
                	}
					(*body)[bodylen]=*data;
					bodylen++;
            	}
				(*body)[bodylen]=0;
				if( bodylen != size )
					*body = realloc( *body, bodylen+1 );
			}
		}
	}
	BBS_CATCH{
	}
	BBS_END;
    end_mmapfile((void *)ptr, size, -1);

	close(fd);

	return 1;

}

int pc_load_usr( struct _pc_selusr **ps, char prefix)
{
	MYSQL s;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char sql[256];
	int ret=0;
	int i;

	if(prefix == 0)
		return 0;

	mysql_init (&s);

	if (! my_connect_mysql_blog(&s) ){
        mysql_report_error(&s);
		return 0;
	}

	sprintf(sql, "SELECT COUNT(*) FROM users WHERE pctype != 9 AND username LIKE \"%c%%\"", prefix);

	if( mysql_real_query(&s, sql, strlen(sql)) ){
        mysql_report_error(&s);
		mysql_close(&s);
		return 0;
	}
	res = mysql_store_result(&s);
	row = mysql_fetch_row(res);
	
	if(row != NULL)
		ret = atoi(row[0]);

	if(ret <= 0){
		mysql_free_result(res);
		mysql_close(&s);
		return 0;
	}

	*ps = (struct _pc_selusr *)malloc(sizeof(struct _pc_selusr) * ret);
	if(*ps == NULL){
		mysql_free_result(res);
		mysql_close(&s);
		return 0;
	}

	sprintf(sql, "SELECT username,corpusname,UNIX_TIMESTAMP(createtime) FROM users WHERE pctype != 9 AND username LIKE \"%c%%\" LIMIT %d", prefix,ret);

	if( mysql_real_query(&s, sql, strlen(sql)) ){
        mysql_report_error(&s);
		mysql_close(&s);
		return 0;
	}
	res = mysql_store_result(&s);
	row = mysql_fetch_row(res);

	i=0;
	while(row != NULL){
		i++;
		if(i > ret)
			break;

		strncpy( (*ps)[i-1].userid, row[0], IDLEN+1);
		(*ps)[i-1].userid[IDLEN+1]=0;
		strncpy( (*ps)[i-1].corpusname, row[1], 40);
		(*ps)[i-1].corpusname[40]=0;
		(*ps)[i-1].createtime = atol(row[2]);

		row = mysql_fetch_row(res);
	}

	mysql_free_result(res);
	mysql_close(&s);

	return i;
}

int get_pc_users( struct pc_users * pu, char * userid )
{
	MYSQL s;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char sql[256];

	if(userid == NULL || *userid == 0 || pu==NULL)
		return 0;

	mysql_init (&s);

	if (! my_connect_mysql_blog(&s) ){
        mysql_report_error(&s);
		return 0;
	}

	sprintf(sql, "SELECT `uid`,`username`,`corpusname`,`description`,`theme`,`nodelimit`,`dirlimit`,UNIX_TIMESTAMP(`createtime`) FROM users WHERE pctype != 9 AND username=\"%s\"", userid);

	if( mysql_real_query(&s, sql, strlen(sql)) ){
        mysql_report_error(&s);
		mysql_close(&s);
		return 0;
	}
	res = mysql_store_result(&s);
	row = mysql_fetch_row(res);

	if(row != NULL){

		pu->uid = atoi(row[0]);
		strcpy(pu->username, userid);
		strncpy(pu->corpusname, row[2], 40);
		pu->corpusname[40]='\0';
		strncpy(pu->description, row[3], 200);
		pu->description[200]='\0';
		strncpy(pu->theme, row[4], 10);
		pu->theme[10]=0;
		pu->nodelimit = atoi(row[5]);
		pu->dirlimit = atoi(row[6]);
		pu->createtime = atol( row[7] );

		mysql_free_result(res);
		mysql_close(&s);

		return 1;
	}

	mysql_free_result(res);
	mysql_close(&s);

	return 0;
}

int get_pc_a_node( struct pc_nodes * pn, unsigned long nid)
{

	MYSQL s;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char sql[300];
	int i;

	mysql_init(&s);

	if (! my_connect_mysql_blog(&s) ){
        mysql_report_error(&s);
		return -1;
	}

	sprintf(sql,"SELECT `nid`,`pid`,`type`,`source`,`hostname`,UNIX_TIMESTAMP(`changed`),UNIX_TIMESTAMP(`created`),`uid`,`comment`,`commentcount`,`subject`,`body`,`access`,`visitcount`,`tid`,`emote`,`htmltag` FROM nodes WHERE nid=%lu", nid );

	if( mysql_real_query(&s, sql, strlen(sql)) ){
        mysql_report_error(&s);
		mysql_close(&s);
		return -1;
	}
	res = mysql_store_result(&s);
	row = mysql_fetch_row(res);

	i=0;
	if(row != NULL){
		i=1;

		pn[i-1].nid = atol(row[0]);
		pn[i-1].pid = atol(row[1]);
		pn[i-1].type = atoi(row[2]);
		if( row[3] ){
			strncpy(pn[i-1].source, row[3], 10);
			pn[i-1].source[10]=0;
		}else
			pn[i-1].source[0]=0;
		if( row[4] ){
			strncpy(pn[i-1].hostname, row[4], 20);
			pn[i-1].hostname[20]=0;
		}else
			pn[i-1].hostname[0]=0;
		pn[i-1].changed = atol( row[5]);
		pn[i-1].created = atol( row[6]);
		pn[i-1].uid = atoi(row[7]);
		pn[i-1].comment = atoi(row[8]);
		pn[i-1].commentcount = atol(row[9]);
		strncpy(pn[i-1].subject, row[10], 200);
		pn[i-1].subject[200]=0;
		pn[i-1].access = atoi(row[12]);
		pn[i-1].visitcount = atoi(row[13]);
                pn[i-1].htmltag = atoi(row[16]);

		pn[i-1].body = (char *)malloc( strlen(row[11]) + 1);
		if( pn[i-1].body ){
			strncpy(pn[i-1].body, row[11], strlen(row[11])+1);
			pn[i-1].body[strlen(row[11])]=0;
		}

	}
	mysql_free_result(res);

	mysql_close(&s);
	return i;
}

int count_pc_nodes( int uid, unsigned long pid, int type, int access )
{

	MYSQL s;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char sql[300];
	char qtmp[100];
	int i;

	mysql_init(&s);

	if (! my_connect_mysql_blog(&s) ){
        mysql_report_error(&s);
		return -1;
	}

	sprintf(sql,"SELECT COUNT(*) FROM nodes WHERE uid=%d", uid );

	snprintf(qtmp, 99, " AND pid=%lu", pid);
	strcat(sql, qtmp);

	if( type != -1 ){
		snprintf(qtmp, 99, " AND type=%d", type);
		strcat(sql, qtmp);
	}

	snprintf(qtmp, 99, " AND access=%d", access);
	strcat(sql, qtmp);

	if( mysql_real_query(&s, sql, strlen(sql)) ){
        mysql_report_error(&s);
		mysql_close(&s);
		return -1;
	}
	res = mysql_store_result(&s);
	row = mysql_fetch_row(res);

	i=0;
	if(row != NULL){
		i=atoi(row[0]);
	}
	mysql_free_result(res);

	mysql_close(&s);
	return i;
}

//type == -1 : 读取所有的type
int get_pc_nodes( struct pc_nodes * pn, int uid, unsigned long pid, int type, int access, int start, int num, int withbody)
{

	MYSQL s;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char sql[500];
	char qtmp[100];
	int i;

	mysql_init(&s);

	if (! my_connect_mysql_blog(&s) ){
        mysql_report_error(&s);
		return -1;
	}

	sprintf(sql,"SELECT `nid`,`pid`,`type`,`source`,`hostname`,UNIX_TIMESTAMP(`changed`),UNIX_TIMESTAMP(`created`),`uid`,`comment`,`commentcount`,`subject`,`body`,`access`,`visitcount`,`tid`,`emote`,`htmltag` FROM nodes WHERE uid=%d", uid );

	snprintf(qtmp, 99, " AND pid=%lu", pid);
	strcat(sql, qtmp);

	if( type != -1 ){
		snprintf(qtmp, 99, " AND type=%d", type);
		strcat(sql, qtmp);
	}

	snprintf(qtmp, 99, " AND access=%d", access);
	strcat(sql, qtmp);

	snprintf(qtmp, 99, " ORDER BY created LIMIT %d,%d", start, num);
	strcat(sql, qtmp);

	if( mysql_real_query(&s, sql, strlen(sql)) ){
        mysql_report_error(&s);
		mysql_close(&s);
		return -1;
	}
	res = mysql_store_result(&s);
	row = mysql_fetch_row(res);

	i=0;
	while(row != NULL){
		i++;
		if( i>num )
			break;

		pn[i-1].nid = atol(row[0]);
		pn[i-1].pid = atol(row[1]);
		pn[i-1].type = atoi(row[2]);
		if( row[3] ){
			strncpy(pn[i-1].source, row[3], 10);
			pn[i-1].source[10]=0;
		}else
			pn[i-1].source[0]=0;
		if( row[4] ){
			strncpy(pn[i-1].hostname, row[4], 20);
			pn[i-1].hostname[20]=0;
		}else
			pn[i-1].hostname[0]=0;
		pn[i-1].changed = atol( row[5]);
		pn[i-1].created = atol( row[6]);
		pn[i-1].uid = atoi(row[7]);
		pn[i-1].comment = atoi(row[8]);
		pn[i-1].commentcount = atol(row[9]);
		strncpy(pn[i-1].subject, row[10], 200);
		pn[i-1].subject[200]=0;
		pn[i-1].access = atoi(row[12]);
		pn[i-1].visitcount = atoi(row[13]);

		if(withbody){
			pn[i-1].body = (char *)malloc( strlen(row[11]) + 1);
			if( pn[i-1].body ){
				strncpy(pn[i-1].body, row[11], strlen(row[11])+1);
				pn[i-1].body[strlen(row[11])]=0;
			}
		}else
			pn[i-1].body = NULL;

		row = mysql_fetch_row(res);
	}
	mysql_free_result(res);

	mysql_close(&s);
	return i;
}

int get_pc_a_com( struct pc_comments * pn, unsigned long cid )
{

	MYSQL s;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char sql[256];
	int i;

	mysql_init(&s);

	if (! my_connect_mysql_blog(&s) ){
        mysql_report_error(&s);
		return -1;
	}

	sprintf(sql,"SELECT `cid`,`nid`,`uid`,`hostname`,`username`,`subject`,UNIX_TIMESTAMP(`created`),UNIX_TIMESTAMP(`changed`),`body` FROM comments WHERE cid=\"%lu\"", cid );

	if( mysql_real_query(&s, sql, strlen(sql)) ){
        mysql_report_error(&s);
		mysql_close(&s);
		return -1;
	}
	res = mysql_store_result(&s);
	row = mysql_fetch_row(res);

	i=0;
	if(row != NULL){
		i++;

		pn[i-1].cid = atol(row[0]);
		pn[i-1].nid = atol(row[1]);
		pn[i-1].uid = atoi(row[2]);
		strncpy(pn[i-1].hostname, row[3], 20);
		pn[i-1].hostname[20]=0;
		strncpy(pn[i-1].username, row[4], 20);
		pn[i-1].username[20]=0;
		strncpy(pn[i-1].subject, row[5], 200);
		pn[i-1].subject[200]=0;
		pn[i-1].created = atol( row[6]);
		pn[i-1].changed = atol( row[7]);

		pn[i-1].body = (char *)malloc( strlen(row[8]) + 1);
		if( pn[i-1].body )
			strncpy(pn[i-1].body, row[8], strlen(row[8])+1);

	}
	mysql_free_result(res);

	mysql_close(&s);
	return i;
}

int count_pc_comments( unsigned long nid)
{

	MYSQL s;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char sql[300];
	int i;

	mysql_init(&s);

	if (! my_connect_mysql_blog(&s) ){
        mysql_report_error(&s);
		return -1;
	}

	sprintf(sql,"SELECT COUNT(*) FROM comments WHERE nid=\"%lu\"", nid );

	if( mysql_real_query(&s, sql, strlen(sql)) ){
        mysql_report_error(&s);
		mysql_close(&s);
		return -1;
	}
	res = mysql_store_result(&s);
	row = mysql_fetch_row(res);

	i=0;
	if(row != NULL){
		i=atoi(row[0]);
	}
	mysql_free_result(res);

	mysql_close(&s);
	return i;
}

int get_pc_comments( struct pc_comments * pn, unsigned long nid, int start, int num, int withbody)
{

	MYSQL s;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char sql[300];
	char qtmp[100];
	int i;

	mysql_init(&s);

	if (! my_connect_mysql_blog(&s) ){
        mysql_report_error(&s);
		return -1;
	}

	sprintf(sql,"SELECT `cid`,`nid`,`uid`,`hostname`,`username`,`subject`,UNIX_TIMESTAMP(`created`),UNIX_TIMESTAMP(`changed`),`body` FROM comments WHERE nid=\"%lu\"", nid );

	snprintf(qtmp, 99, " ORDER BY created LIMIT %d,%d", start, num);
	strcat(sql, qtmp);

	if( mysql_real_query(&s, sql, strlen(sql)) ){
        mysql_report_error(&s);
		mysql_close(&s);
		return -1;
	}
	res = mysql_store_result(&s);
	row = mysql_fetch_row(res);

	i=0;
	while(row != NULL){
		i++;
		if( i>num )
			break;

		pn[i-1].cid = atol(row[0]);
		pn[i-1].nid = atol(row[1]);
		pn[i-1].uid = atoi(row[2]);
		strncpy(pn[i-1].hostname, row[3], 20);
		pn[i-1].hostname[20]=0;
		strncpy(pn[i-1].username, row[4], 20);
		pn[i-1].username[20]=0;
		strncpy(pn[i-1].subject, row[5], 200);
		pn[i-1].subject[200]=0;
		pn[i-1].created = atol( row[6]);
		pn[i-1].changed = atol( row[7]);

		if(withbody){
			pn[i-1].body = (char *)malloc( strlen(row[8]) + 1);
			if( pn[i-1].body )
				strncpy(pn[i-1].body, row[8], strlen(row[8])+1);
		}else
			pn[i-1].body = NULL;

		row = mysql_fetch_row(res);
	}
	mysql_free_result(res);

	mysql_close(&s);
	return i;
}

int add_pc_users(struct pc_users *pn)
{
	MYSQL s;
	char sql[600];
	char newtheme[21];
	char newdesc[401];
	char newts[20];
	char newcorp[81];

	newtheme[0]=0;
	newdesc[0]=0;
	newts[0]=0;
	newcorp[0]=0;

	mysql_init(&s);
	if (! my_connect_mysql_blog(&s) ){
        mysql_report_error(&s);
		return 0;
	}

	mysql_escape_string(newtheme, pn->theme, strlen(pn->theme));
	mysql_escape_string(newcorp, pn->corpusname, strlen(pn->corpusname));
	mysql_escape_string(newdesc, pn->description, strlen(pn->description));

	if( pn->uid <= 0 )
		sprintf(sql,"INSERT INTO `users` ( `uid` , `username` , `corpusname` , `description` , `theme` , `nodelimit` , `dirlimit` , `createtime` , `style` , `backimage` , `visitcount` , `nodescount` , `logoimage` , `modifytime` , `links` , `htmleditor` , `indexnodechars` , `indexnodes` , `useremail` , `favmode` , `updatetime` , `userinfor` , `pctype` ,`defaulttopic`) VALUES ('', '%s', '%s', '%s' , 'others', %d, %d, NOW( ) , '0', '' , '0', '0', '' , NOW( ) , '', '1', '600', '5', '', '0', NOW( ) , '' , '0' , '其他类别');",pn->username, newcorp, newdesc, pn->nodelimit, pn->dirlimit );
	else
		sprintf(sql,"UPDATE users SET description='%s', corpusname='%s', theme='%s', nodelimit=%d, dirlimit=%d, createtime='%s' WHERE uid=%u AND username='%s' ;",newdesc, newcorp, newtheme, pn->nodelimit, pn->dirlimit, tt2timestamp(pn->createtime,newts), pn->uid, pn->username );
	

	if( mysql_real_query( &s, sql, strlen(sql) )){
        mysql_report_error(&s);
		mysql_close(&s);
		return 0;
	}
	
	sprintf(sql,"UPDATE newapply SET `apptime` = `apptime` , manager = '%s' , management = 0 WHERE username = '%s ' ;", getCurrentUser()->userid , pn->username);	
	if( mysql_real_query( &s, sql, strlen(sql) )){
        mysql_report_error(&s);
		mysql_close(&s);
		return 0;
	}

	mysql_close(&s);

	return 1;
}

int add_pc_nodes(struct pc_nodes *pn)
{
	MYSQL s;
	char *ql;
	char *newbody=NULL;
	char newsource[21];
	char newhostname[41];
	char newsubject[401];
	char newts[20];
	char newts1[20];

	newsource[0]=0;
	newhostname[0]=0;
	newsubject[0]=0;
	newts[0]=0;
	newts1[0]=0;

	if( pn->body ){
		newbody = (char *)malloc( strlen(pn->body) * 2 + 1 );
		if( ! newbody )
			return 0;
	}

	ql = (char *)malloc( ( pn->body?(strlen(pn->body) * 2):0 ) + 800 );
	if( ! ql ){
		if(pn->body)
			free(newbody);
		return 0;
	}
	bzero(ql, pn->body?(strlen(pn->body)*2):0 + 800);

	mysql_init(&s);
	if (! my_connect_mysql_blog(&s) ){
        mysql_report_error(&s);
		if(pn->body)
			free(newbody);
		free(ql);
		return 0;
	}

	mysql_escape_string(newsource, pn->source, strlen(pn->source));
	mysql_escape_string(newhostname, pn->hostname, strlen(pn->hostname));
	mysql_escape_string(newsubject, pn->subject, strlen(pn->subject));
	if( pn->body )
		mysql_escape_string(newbody, pn->body, strlen(pn->body));

	if( pn->nid <= 0 )
	{
		if(pn->access == 0)
		{
			sprintf(ql,"UPDATE users SET `createtime` = `createtime` , modifytime = '%s' , nodescount = nodescount + 1 WHERE uid=%d ;",tt2timestamp(pn->changed,newts),pn->uid );	
			mysql_real_query( &s, ql, strlen(ql) );
		}
		sprintf(ql,"INSERT INTO `nodes` (  `pid` , `tid` , `type` , `recuser` , `emote` , `hostname` , `changed` , `created` , `uid` , `comment` , `commentcount` , `subject` , `body` , `access` , `visitcount` , `htmltag`,`trackback` ,`trackbackcount`,`nodetype`) VALUES ( %lu , 0 , %d , '%s', 0 ,  '%s', '%s' , '%s' , %d , %d , %lu , '%s', '%s', %d , %d , 0 , 1 , 0 , 0 );",pn->pid, pn->type, newsource, newhostname, tt2timestamp(pn->changed,newts), tt2timestamp(pn->created, newts1), pn->uid, pn->comment, pn->commentcount, newsubject, pn->body?newbody:"", pn->access, pn->visitcount );
	}
	else
	{
		sprintf(ql,"UPDATE nodes SET pid=%lu, type=%d, source='%s', hostname='%s', uid=%d, comment=%d, commentcount=%ld, subject='%s', body='%s', access=%d, visitcount=%d WHERE nodetype = 0 AND nid=%lu ;",pn->pid, pn->type, newsource, newhostname, pn->uid, pn->comment, pn->commentcount, newsubject, pn->body?newbody:"", pn->access, pn->visitcount, pn->nid );
	}
	
	if( mysql_real_query( &s, ql, strlen(ql) )){
        mysql_report_error(&s);
		mysql_close(&s);
		if(pn->body)
			free(newbody);
		free(ql);
		return 0;
	}

	mysql_close(&s);
	if(pn->body)
		free(newbody);
	free(ql);

	return 1;
}

int pc_in_blacklist(char * userid , unsigned long pcuid)
{
	MYSQL s;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char sql[100];

	if(userid == NULL || *userid == 0 || pcuid == 0)
		return 0;

	mysql_init (&s);

	if (! my_connect_mysql_blog(&s) ){
        mysql_report_error(&s);
		return 0;
	}

	sprintf(sql, "SELECT * FROM blacklist WHERE userid=\"%s\" AND (uid=\"%lu\" OR uid=\"0\")", userid , pcuid);

	if( mysql_real_query(&s, sql, strlen(sql)) ){
        mysql_report_error(&s);
		mysql_close(&s);
		return 0;
	}
	res = mysql_store_result(&s);
	row = mysql_fetch_row(res);

	if(row != NULL){
		mysql_free_result(res);
		mysql_close(&s);

		return 1;
	}

	mysql_free_result(res);
	mysql_close(&s);

	return 0;
}

int add_pc_comments(struct pc_comments *pn)
{
	MYSQL s;
	char *sql;
	char *newbody;
	char newhostname[41];
	char newsubject[401];
	char newts[20];
	char newts1[20];

	newhostname[0]=0;
	newsubject[0]=0;
	newts[0]=0;
	newts1[0]=0;

	if( pn->body) {
		newbody = (char *)malloc( strlen(pn->body)*2 + 1 );
		if( ! newbody )
			return 0;
	}else
		newbody = NULL;

	sql = (char *)malloc( ( pn->body ? (strlen(pn->body) * 2) : 0 ) + 800 );
	if( ! sql ){
		if(pn->body)
			free(newbody);
		return 0;
	}

	mysql_init(&s);
	if (! my_connect_mysql_blog(&s) ){
        mysql_report_error(&s);
		if(pn->body)
			free(newbody);
		free(sql);
		return 0;
	}

	mysql_escape_string(newhostname, pn->hostname, strlen(pn->hostname));
	mysql_escape_string(newsubject, pn->subject, strlen(pn->subject));
	if(pn->body)
		mysql_escape_string(newbody, pn->body, strlen(pn->body));

	if( pn->cid <= 0 )
		sprintf(sql,"INSERT INTO comments VALUES (NULL, %lu, %d, '%s', '%s', '%s', '%s', '%s', '%s', 0 , 0 );",pn->nid, pn->uid, newhostname, pn->username, newsubject, tt2timestamp(pn->created,newts), tt2timestamp(pn->changed, newts1), pn->body?newbody:"" );
	else
		sprintf(sql,"UPDATE comments SET nid=%lu, uid=%d, hostname='%s', username='%s', subject='%s', created='%s', changed='%s', body='%s' WHERE cid=%lu ;",pn->nid, pn->uid, newhostname, pn->username, newsubject, tt2timestamp(pn->created,newts), tt2timestamp(pn->changed, newts1), pn->body?newbody:"", pn->cid );
	
	if( mysql_real_query( &s, sql, strlen(sql) )){
        mysql_report_error(&s);
		mysql_close(&s);
		if(pn->body)
			free(newbody);
		free(sql);
		return 0;
	}

	if( pn->cid <= 0){
		sprintf(sql,"UPDATE nodes SET changed=changed,commentcount=commentcount+1 WHERE nid=%lu",pn->nid);
		mysql_real_query( &s, sql, strlen(sql) );
	}

	mysql_close(&s);
	if(pn->body)
		free(newbody);
	free(sql);

	return 1;
}

int del_pc_users(struct pc_users *pn)
{
	MYSQL s;
	char sql[200];

	if( pn->uid <= 0 || ! pn->username[0] )
		return 0;

	mysql_init(&s);
	if (! my_connect_mysql_blog(&s) ){
        mysql_report_error(&s);
		return 0;
	}

	sprintf(sql,"DELETE FROM users WHERE uid=%u AND username='%s' ;", pn->uid, pn->username );

	if( mysql_real_query( &s, sql, strlen(sql) )){
        mysql_report_error(&s);
		mysql_close(&s);
		return 0;
	}
	
	sprintf(sql,"DELETE FROM nodes WHERE uid=%u ;", pn->uid );

	if( mysql_real_query( &s, sql, strlen(sql) )){
        mysql_report_error(&s);
		mysql_close(&s);
		return 0;
	}
	
	sprintf(sql,"DELETE FROM comments WHERE uid=%u ;", pn->uid );

	if( mysql_real_query( &s, sql, strlen(sql) )){
        mysql_report_error(&s);
		mysql_close(&s);
		return 0;
	}
	
	sprintf(sql,"DELETE FROM blacklist WHERE uid=%u ;", pn->uid );

	if( mysql_real_query( &s, sql, strlen(sql) )){
        mysql_report_error(&s);
		mysql_close(&s);
		return 0;
	}
	
	sprintf(sql,"DELETE FROM recommend WHERE uid=%u ;", pn->uid );

	if( mysql_real_query( &s, sql, strlen(sql) )){
        mysql_report_error(&s);
		mysql_close(&s);
		return 0;
	}
	
	sprintf(sql,"DELETE FROM topics WHERE uid=%u ;", pn->uid );

	if( mysql_real_query( &s, sql, strlen(sql) )){
        mysql_report_error(&s);
		mysql_close(&s);
		return 0;
	}
	
	sprintf(sql,"DELETE FROM trackback WHERE uid=%u ;", pn->uid );

	if( mysql_real_query( &s, sql, strlen(sql) )){
        mysql_report_error(&s);
		mysql_close(&s);
		return 0;
	}
		
	mysql_close(&s);

	return 1;
}

int del_pc_nodes( unsigned long nid , int access , int uid )
{
		char sql[100];
		MYSQL s;

		if(nid <= 0)
			return 0;

		mysql_init(&s);
		if (! my_connect_mysql_blog(&s) ){
			mysql_report_error(&s);
			return 0;
		}
		if( access == 0 )
		{
			sprintf(sql,"UPDATE users SET `createtime` = `createtime` , nodescount = nodescount - 1 WHERE uid=%d ;",uid );	
			mysql_real_query( &s, sql, strlen(sql) );
		}
		
		sprintf(sql,"DELETE FROM nodes WHERE nid=%lu;",nid);

		if( mysql_real_query( &s, sql, strlen(sql) ) ){
			mysql_report_error(&s);
			mysql_close(&s);
			return 0;
		}

		mysql_close(&s);

		return 1;
}

int pc_del_junk(int uid)
{
	MYSQL s;
	char ql[200];

	mysql_init(&s);
	if (! my_connect_mysql_blog(&s) ){
        mysql_report_error(&s);
		return 0;
	}

	sprintf(ql,"DELETE FROM nodes WHERE uid=%d AND access=4",uid);
	
	if( mysql_real_query( &s, ql, strlen(ql) )){
        mysql_report_error(&s);
		mysql_close(&s);
		return 0;
	}

	mysql_close(&s);

	return 1;
}

int del_pc_node_junk(unsigned int nid , int access , int uid )
{
	MYSQL s;
	char ql[200];

	mysql_init(&s);
	if (! my_connect_mysql_blog(&s) ){
        mysql_report_error(&s);
		return 0;
	}
	
	if( access == 0 )
		{
			sprintf(ql,"UPDATE users SET `createtime` = `createtime` , nodescount = nodescount - 1 WHERE uid=%d ;",uid );	
			mysql_real_query( &s, ql, strlen(ql) );
		}
	sprintf(ql,"UPDATE nodes SET access=4,pid=0,type=0 WHERE nodetype = 0 AND nid=%u",nid);
	
	if( mysql_real_query( &s, ql, strlen(ql) )){
        mysql_report_error(&s);
		mysql_close(&s);
		return 0;
	}

	mysql_close(&s);

	return 1;
}

int del_pc_comments( unsigned long nid, unsigned long cid )
{
		char sql[100];
		MYSQL s;

		if(cid <= 0)
			return 0;

		mysql_init(&s);
		if (! my_connect_mysql_blog(&s) ){
            mysql_report_error(&s);
			return 0;
		}

		sprintf(sql,"DELETE FROM comments WHERE cid=%lu;",cid);

		if( mysql_real_query( &s, sql, strlen(sql) ) ){
            mysql_report_error(&s);
			mysql_close(&s);
			return 0;
		}

		sprintf(sql,"UPDATE nodes SET changed=changed,commentcount=commentcount-1 WHERE nid=%lu",nid);
		if( mysql_real_query( &s, sql, strlen(sql) ) ){
            mysql_report_error(&s);
			mysql_close(&s);
			return 0;
		}

		mysql_close(&s);

		return 1;
}

int pc_paste_node(unsigned long nid, int newuid, int newaccess, unsigned long newpid ) 
{
	struct pc_nodes pn;
	int ret;

	bzero( &pn, sizeof(pn) );
	if(get_pc_a_node(&pn, nid)<=0)
		return 0;

	pn.created=time(0);
	pn.changed = pn.created;
	pn.pid = newpid;
	pn.uid = newuid;
	pn.type = 0;
	pn.visitcount = 0;
	pn.nid = 0;
	pn.access = newaccess;
	pn.commentcount = 0;

	ret = add_pc_nodes(&pn);

	if(pn.body)
		free(pn.body);

	return ret;
}

int pc_add_visitcount(unsigned long nid)
{
	MYSQL s;
	char ql[100];

	mysql_init(&s);
	if (! my_connect_mysql_blog(&s) ){
        mysql_report_error(&s);
		return 0;
	}

	sprintf(ql,"UPDATE nodes SET changed=changed,visitcount=visitcount+1 WHERE nid=%lu",nid);
	
	if( mysql_real_query( &s, ql, strlen(ql) )){
        mysql_report_error(&s);
		mysql_close(&s);
		return 0;
	}

	mysql_close(&s);

	return 1;
}

int pc_logs(struct pc_logs *pn)
{
	MYSQL s;
	char sql[600];
	char newhostname[21];
	char newaction[101];
	char newcomment[201];
	char newts[20];
	
	newhostname[0]=0;
	newaction[0]=0;
	newcomment[0]=0;
	newts[0] = 0;
	
	mysql_init(&s);
	if (! my_connect_mysql_blog(&s) ){
        mysql_report_error(&s);
		return 0;
	}

	mysql_escape_string(newhostname, pn->hostname, strlen(pn->hostname));
	mysql_escape_string(newaction, pn->action, strlen(pn->action));
	mysql_escape_string(newcomment, pn->comment, strlen(pn->comment));
	
	sprintf(sql,"INSERT INTO logs VALUES (NULL, '%s', '%s', '%s', '%s' , '%s' , '%s', '%s' );",pn->username, newhostname,newaction, pn->pri_id, pn->sec_id, newcomment, tt2timestamp(pn->logtime,newts));
	
	if( mysql_real_query( &s, sql, strlen(sql) )){
        mysql_report_error(&s);
		mysql_close(&s);
		return 0;
	}

	mysql_close(&s);

	return 1;
}


#endif
