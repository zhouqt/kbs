/*
 *   cnv_pc4fb2k.c 
 *
 *   将fb2k格式的个人文集转换成SMTH_STYLE_PC
 *   转换过后所有的文章都放在公共区域
 *
 *                            deem@hit
 *
 */

#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include "bbs.h"
#include "mysql.h"

/*
 *   由于SMTH格式的个人文集(BLOG)在公共区域不支持目录，
 *   所以把目录转换成TOPIC, 下面的TOPIC_LEVLE设置把目录
 *   转化成TOPIC的层次，层次大于TOPIC_LEVEL的目录将被忽
 *   略(文件归属于上层目录)。
 */
#define TOPIC_LEVEL    1

#define TRIM(a) { trimleft(a); trimright(a); }

unsigned int TotalAccount = 0;
unsigned int TotalArtical = 0;
char curruser[32];
MYSQL s;


void trimright(char *buff)
{
	int i = 0;
	while (buff[i])
	{
		if (buff[i] == ' ')
			buff[i] = buff[i+1];
		else
			break;	

	}
}

void trimleft(char *buff)
{
	int i = strlen(buff) - 1;

	while( (i >= 0) && ((buff[i] == ' ') || (buff[i] == '\r') || (buff[i] == '\n')) )
	{
		buff[i] = 0;
		i--;
	}
}

// 分析.Names中的字符串，返回字符串的值
// -1: Error line
// 0 : comment
// 1 : Name=
// 2 : Path=
// 3 : Numb=
// 
int ParseDotNames(char *buff, char *ret)
{

	TRIM(buff);
	if (buff[0] == '#')
		return 0;             //comment
	if (strlen(buff) < 6)
		return -1;             //error line
	if (!strncmp(buff, "Name=", 5))
	{
		char *p;
		p = strrchr(buff, '(');
		if (p)
			*p = 0;
		
		trimleft(buff);
		memset(ret, 0, 512);

		strncpy(ret, buff + 5, 512);
		return 1;
	}
	else if (!strncmp(buff, "Path=", 5))
	{
		memset(ret, 0, 512);
		strncpy(ret, buff + 7, 512);
		return 2;
	}
	else if (!strncmp(buff, "Numb=", 5))
	{
		return 3;
	}
	return -1;
}


int main(int argc, char **argv)
{
	char rootchar, pc_path[256], fn[256];
	FILE *fp;
        
	chdir(BBSHOME);
        resolve_ucache();
	resolve_boards();
	resolve_utmp();
	
	mysql_init(&s);

	if (!my_connect_mysql(&s)) 
	{
		printf("%s\n", mysql_error(&s));
		return 0;
	}

	if (argc < 2)
	{
		fprintf(stdout, "cnv_pc4fb2k personal_corp_path\n");
		return 1;
	}
	
	for(rootchar = 'A'; rootchar <= 'Z'; rootchar++)
	{
		memset(pc_path, 0, 256);
		memset(fn, 0, 256);
		snprintf(pc_path, 256, "%s%c/", argv[1], rootchar);
		strcpy(fn, pc_path);
		strcat(fn, ".Names");
		if (!dashf(fn))
			continue;
		
		if ( (fp = fopen(fn, "r")) == NULL )
		{
			fprintf(stdout, "open file %s error!\n", fn);
			continue;
		}
		while(!feof(fp))
		{
			char readb[1024], nameb[512], dn[512], tempstr[512];
			memset(readb, 0, 1024);

			if (fgets(readb, 1024, fp) != NULL)
			{
					switch(ParseDotNames(readb, tempstr))
					{
					case 1:
						strcpy(nameb, tempstr);
						break;
					case 2:
						strcpy(dn, tempstr);
						break;
					case 3:
						printf("Found User %s, With ID %s\n", nameb, dn);
						TotalAccount++;
						cnv_user(dn, nameb, pc_path);
					default:
						break;
					}		
			}	
		}
		fclose(fp);
	}

	return 0;
}

//
//  id = User ID
//  desc = PC Description
//  r_path = PC ROOT DIR
//
int cnv_user(char *id, char *desc, char *r_path)
{
	struct stat st;
	char fn[256], path[256];
	int uid;
	
	sprintf(path, "%s/%s", r_path, id);

	bzero(curruser, 32);
	strcpy(curruser, id);
	
	if ( stat(path, &st) == 0 )
	{
		struct pc_users pu;
		memset(&pu, 0, sizeof(pu));
		pu.createtime = st.st_mtime;
		pu.dirlimit = 200;
		pu.nodelimit = 500;

		strncpy(pu.theme, "others", 6);
		if (!strncmp(desc + strlen(desc) - strlen(id), id, strlen(id)))
			desc[strlen(desc) - strlen(id)] = 0;

		strncpy(pu.corpusname, desc, strlen(desc)>40?40:strlen(desc));
		strncpy(pu.username, id, 14);

		if ( uid = add_pc_users(&pu) )
		{
			fprintf(stdout, "Add User %s Success\n", id);
			//go on with personal dir
			cnv_personal_dir(path, 0, uid, 0);
		}
		else
			fprintf(stdout, "Add User %s Error\n", id);

	}
	
	return 0;
}

int cnv_personal_dir(char *r_path, int level, int uid, int tid)
{
	struct stat st;
	char fn[256], path;
	FILE *fp;
	int added_pid;

	sprintf(fn, "%s/.Names", r_path);
	if ( stat(fn, &st) != 0 )
	{
		printf("stat %s error!\n");
		return 0; 
	}
	
	if ( (fp = fopen(fn, "r")) == NULL)
	{
		printf("open %s error!\n");
		return 0;
	}

	while (!feof(fp))
	{
		char readb[1024], nameb[512], dn[512], tempstr[512];
		memset(readb, 0, 1024);
		if (fgets(readb, 1024, fp) != NULL)
		{
			switch(ParseDotNames(readb, tempstr))
			{
				case 1:
					bzero(nameb, 512);
					strcpy(nameb, tempstr);
					break;
				case 2:
					bzero(dn, 512);
					strcpy(dn, tempstr);
					break;
				case 3:  // found a complete entry
					sprintf(tempstr, "%s/%s", r_path, dn);
					if (dashf(tempstr))
					{
						if (add_node_to_pc(tempstr, nameb, uid, tid) < 1)
							return 0;
					}
					else if (dashd(tempstr))
					{
						if (level < TOPIC_LEVEL)
						{
							int newtid  = add_pc_topic(nameb, uid);
							if (newtid  < 1)
								return 0;
							cnv_personal_dir(tempstr, level + 1, uid, newtid);
						}
						else
							cnv_personal_dir(tempstr, level + 1, uid, tid);
					}
				default:
					break;
			}
		}
	}
	fclose(fp);
}

//
//  add file to mysql
//
int add_node_to_pc(char *filename, char *subject, int uid, int tid)
{
	struct stat st;
	struct pc_nodes pn;
	int ret;

	if ( stat(filename, &st) != 0 )
		return 0;

	bzero(&pn, sizeof(pn));

	if (!strncmp(subject + strlen(subject) - strlen(curruser), curruser, strlen(curruser)))
		subject[strlen(subject) - strlen(curruser)] = 0;
	strcpy(pn.subject, subject);

	printf("SUBJECT: %s\n", subject);
	pn.body = NULL;
	if (!pc_conv_file_to_body( &(pn.body), filename))
		return 0;

	strcpy(pn.hostname, "202.118.224.2");
	pn.created = st.st_mtime;
	pn.changed = st.st_mtime;

	pn.uid = uid;
	pn.pid = 0;

	pn.access = 0;  //put it in public area
	pn.type = 0;
	
	ret = add_pc_nodes_to_mysql(&pn, tid);
	
	if (pn.body)
		free(pn.body);

	return ret;
}

//
// modified form libpc.c
//
int add_pc_nodes_to_mysql(struct pc_nodes *pn, int tid)
{
	MYSQL_RES *res;
	MYSQL_ROW row;
	char sql[1024];
	int nid;
	
	char *ql;
	char *newbody;
	char newsource[21];
	char newhostname[41];
	char newsubject[401];
	char newts[20];
	char newts1[20];

	newsource[0] = 0;
	newhostname[0] = 0;
	newsubject[0] = 0;
	newts[0] = 0;
	newts1[0] = 0;
	if (pn->body) {
		newbody = (char *) malloc(strlen(pn->body) * 2 + 1);
		if (!newbody)
		return 0;
	}

	ql = (char *) malloc((pn->body ? (strlen(pn->body) * 2) : 0) + 800);
	if (!ql) {
		if (pn->body)
			free(newbody);
		return 0;
	}
	bzero(ql, pn->body ? (strlen(pn->body) * 2) : 0 + 800);
/*	
	mysql_init(&s);

	if (!my_connect_mysql(&s)) 
	{
		printf("%s\n", mysql_error(&s));
		if (pn->body)
			free(newbody);
		free(ql);
		return 0;
	}
*/
	mysql_escape_string(newsource, pn->source, strlen(pn->source));
	mysql_escape_string(newhostname, pn->hostname, strlen(pn->hostname));
	mysql_escape_string(newsubject, pn->subject, strlen(pn->subject));
	if (pn->body)
		mysql_escape_string(newbody, pn->body, strlen(pn->body));
	
	sprintf(ql, "INSERT INTO nodes VALUES (NULL, 0, 0, '%s', '%s', '%s', '%s', %d, 1, 0, '%s', '%s', 0, 0, %d , 0, 0, 1, 0, 0, '%s', 0);",  newsource, newhostname, 
			 tt2timestamp(pn->changed, newts), 
			 tt2timestamp(pn->created, newts1), 
			 pn->uid, 
			 newsubject, 
			 pn->body ? newbody : "", 
			 tid,
			 tt2timestamp(pn->created, newts1)
	);
	
	if (mysql_real_query(&s, ql, strlen(ql))) {
		printf("%s\n", mysql_error(&s));
//		mysql_close(&s);
		if (pn->body)
			free(newbody);
		free(ql);
		return 0;
	}

//	mysql_close(&s);
	if (pn->body)
		free(newbody);

	free(ql);
	return nid;
}


MYSQL *my_connect_mysql(MYSQL * s)
{
	return mysql_real_connect(s,
		"localhost",
		"root", 
		"passwd",
	   	"blog", 
		1521, 
		NULL, 
		0);
}

//
//  return 0 failed
//  return >0 user pid
int add_pc_users(struct pc_users *pn)
{
//	MYSQL s;
	MYSQL_RES *res;
	MYSQL_ROW row;
	
	char sql[600];
	char newtheme[21];
	char newdesc[401];
	char newts[20];
	char newcorp[81];
	int  uid = 0;

	
	newtheme[0] = 0;
	newdesc[0] = 0;
	newts[0] = 0;
	newcorp[0] = 0;
	
	
/*	mysql_init(&s);
	if (!my_connect_mysql(&s)) {
		printf("%s\n", mysql_error(&s));
		return 0;
	}
*/
	mysql_escape_string(newtheme, pn->theme, strlen(pn->theme));
	mysql_escape_string(newcorp, pn->corpusname, strlen(pn->corpusname));
	mysql_escape_string(newdesc, pn->description, strlen(pn->description));

	if (pn->uid <= 0)
              sprintf(sql, "INSERT INTO users VALUES (NULL, '%s', '%s', '%s', 'others', %d, %d, '%s', 0 ,NULL,0,0,NULL,'%s' , '' , 0 , 600 , 5 , '' , 0 , '%s' , '' , 0 , 0, '其他类别');", pn->username, newcorp, newdesc, pn->nodelimit, pn->dirlimit, tt2timestamp(pn->createtime, newts), tt2timestamp(pn->createtime, newts), tt2timestamp(pn->createtime, newts));

	if (mysql_real_query(&s, sql, strlen(sql))) {
		printf("%s\n", mysql_error(&s));
//		mysql_close(&s);
		return 0;
	}
	sprintf(sql, "SELECT uid FROM users WHERE username = \"%s\"", 
					pn->username);

	if (mysql_real_query(&s, sql, strlen(sql)))
	{
		printf("%s\n", mysql_error(&s));
		return 0;
	}
	res = mysql_store_result(&s);
	row = mysql_fetch_row(res);

	if( row != NULL)
	{
		uid = atoi(row[0]);
		printf("UserID is %d, ", uid);
	}
	else
		uid = 0;
	mysql_free_result(res);
//	mysql_close(&s);

	return uid;
}



//
//  return 0 failed
//  return >0 user pid
int add_pc_topic(char *topic, int uid)
{
//	MYSQL s;
	MYSQL_RES *res;
	MYSQL_ROW row;
	int tid;
	char name[512];
	char sql[2048];

	name[0] = 0;
/*	
	mysql_init(&s);
	if (!my_connect_mysql(&s)) {
		printf("%s\n", mysql_error(&s));
		return 0;
	}
*/
	mysql_escape_string(name, topic, strlen(topic));

	sprintf(sql, "INSERT INTO topics VALUES (NULL, %d, 0, '%s',0)", uid, name);

	if (mysql_real_query(&s, sql, strlen(sql))) {
		printf("%s\n", mysql_error(&s));
//		mysql_close(&s);
		return 0;
	}
	sprintf(sql, "SELECT tid FROM topics WHERE topicname = \"%s\" and uid = %d", 
					topic, uid);

	if (mysql_real_query(&s, sql, strlen(sql)))
	{
		printf("%s\n", mysql_error(&s));
		return 0;
	}
	res = mysql_store_result(&s);
	row = mysql_fetch_row(res);

	if( row != NULL)
	{
		tid  = atoi(row[0]);
	}
	else
		tid = 0;
	mysql_free_result(res);
//	mysql_close(&s);

	return tid;
}
