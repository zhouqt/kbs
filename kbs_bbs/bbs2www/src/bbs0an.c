/*
 * $Id$
 */
#include "bbslib.h"

void ann_add_item(MENU *pm, ITEM *it)
{
	ITEM        *newitem;

	if ( pm->num < MAXITEMS )
	{
		newitem = pm->item[pm->num];
		strncpy( newitem->title, it->title, sizeof(newitem->title)-1 );
		if (it->host != NULL)
		{
			newitem->host = (char *)malloc(sizeof(char)*(strlen(it->host)+1));
			strcpy(newitem->host, it->host);
		}
		else
			newitem->host = it->host;
		newitem->port = it->port;
		strncpy(newitem->fname, it->fname, sizeof(newitem->fname) - 1);
		(pm->num)++;
	}
}

int ann_load_directory(MENU *pm)
{
	FILE        *fn;
	ITEM        litem;
	char        buf[PATHLEN];
	char        *ptr;
	char        hostname[STRLEN];

	pm->num = 0;
	snprintf(buf, sizeof(buf), "%s/.Names", pm->path); /*.Names记录菜单信息*/
	if ( (fn = fopen( buf, "r" )) == NULL )
		return -1;
	hostname[0]='\0';
	while ( fgets( buf, sizeof(buf), fn ) != NULL )
	{
		if ( (ptr = strchr( buf, '\n' )) != NULL )
			*ptr = '\0';
		if ( strncmp( buf, "Name=", 5 ) == 0 )
		{
			strncpy( litem.title, buf + 5, sizeof(litem.title)-1 );
			litem.title[sizeof(litem.title)-1] = '\0';
		}
		else if ( strncmp( buf, "Path=", 5 ) == 0 )
		{
			if ( strncmp( buf, "Path=~/", 7 ) == 0 )
				strncpy( litem.fname, buf + 7, sizeof(litem.fname)-1 );
			else
				strncpy( litem.fname, buf + 5, sizeof(litem.fname)-1 );
			litem.fname[sizeof(litem.fname)-1] = '\0';
			if ((!strstr(litem.title,"(BM: BMS)")||HAS_PERM(currentuser,PERM_BOARDS))&&
				(!strstr(litem.title,"(BM: SYSOPS)")||HAS_PERM(currentuser,PERM_SYSOP))&&
				(!strstr(litem.title,"(BM: ZIXIAs)")||HAS_PERM(currentuser,PERM_SECANC)))
			{
				if (strstr(litem.fname,"!@#$%")) /*取 host & port */
				{
					char *ptr1,*ptr2,gtmp[STRLEN];
					strncpy(gtmp, litem.fname, STRLEN-1);
					/* Warning: NOT thread safe */
					ptr1 = strtok(gtmp,"!#$%@");
					strcpy(hostname, ptr1);
					ptr2 = strtok(NULL,"@");
					strncpy(litem.fname, ptr2, sizeof(litem.fname)-1);
					litem.fname[sizeof(litem.fname)-1] = '\0';
					litem.port = atoi(strtok(NULL,"@"));
				}
				ann_add_item(pm, &litem);
			}
			hostname[0] = '\0';
		}
		else if ( strncmp( buf, "# Title=", 8 ) == 0 )
		{
			if ( pm->mtitle[0] == '\0' )
			{
				strncpy( pm->mtitle, buf + 8, sizeof(pm->mtitle)-1 );
				pm->mtitle[sizeof(pm->mtitle)-1] = '\0';
			}
		}
		else if ( strncmp( buf, "Host=", 5 ) == 0 )
		{
			strncpy( hostname, buf + 5, sizeof(hostname)-1 );
			hostname[sizeof(hostname)-1] = '\0';
		}
		else if ( strncmp( buf, "Port=", 5 ) == 0 )
		{
			litem.port = atoi(buf + 5);
		}
	}
	fclose( fn );
	return 0;
}

ITEM *ann_alloc_items(size_t num)
{
	ITEM *it;

	it = (ITEM *)calloc(num, sizeof(ITEM));
	return it;
}

void ann_free_items(ITEM *it, size_t num)
{
	size_t i;

	if (it != NULL)
	{
		for (i = 0; i < num; i++)
		{
			if (it[i].host != NULL)
			{
				free(it[i].host);
				it[i].host = NULL;
			}
		}
		free(it);
	}
}

void ann_set_items(MENU *pm, ITEM *it, size_t num)
{
	size_t i;

	for (i = 0; i < num; i++)
		pm->item[i] = it + i;
}

/*
 * Get a board name from an announce path.
 * For high performance, we should drop .Search file into shared memory.
*/
int ann_get_board(char *path, char *board, size_t len)
{
	FILE *fp;
	char buf[256];
	char *ptr;
	char *ptr2;
	
	ptr = path;
	if(ptr[0] == '\0')
		return -1;
	if (ptr[0] == '/')
		ptr++;
	fp = fopen("0Announce/.Search", "r");
	if(fp == NULL)
		return -1;
	while(fgets(buf, sizeof(buf), fp) != NULL)
	{
		if ((ptr2 = strchr(buf, ':')) != NULL)
		{
			*ptr2 = '\0';
			if (strncmp(ptr+2, ptr, strlen(ptr+2)) == 0)
			{
				strncpy(board, buf, len-1);
				board[len-1] = '\0';
				fclose(fp);
				return 0;
			}
		}
	}
	fclose(fp);
	return -1;
}

/*
 * Get an announce path from a board name.
 * For high performance, we should drop .Search file into shared memory.
*/
int ann_get_path(char *board, char* path, size_t len)
{
	FILE *fp;
	char buf[256];
	char *ptr;
	
	fp = fopen("0Announce/.Search", "r");
	if(fp == NULL)
		return -1;
	while(fgets(buf, sizeof(buf), fp) != NULL)
	{
		if ((ptr = strchr(buf, ':')) != NULL)
		{
			*ptr = '\0';
			if (strcmp(buf, board) == 0)
			{
				strncpy(path, ptr+2, len-1);
				path[len-1] = '\0';
				fclose(fp);
				return 0;
			}
		}
	}
	fclose(fp);
	return -1;
}

int ann_traverse_check(char *path)
{
	char *ptr;
	size_t i = 0;
	char filename[256];
	char buf[256];
	char pathbuf[256];
	char title[STRLEN];
	char currpath[256];
	FILE *fp;

	/* path parameter can not have leading '/' character */
	if (path[0] == '/')
		return -1;
	bzero(pathbuf, sizeof(pathbuf));
	ptr = path;
	while(*ptr != '\0')
	{
		if(*ptr == '/')
			snprintf(filename, sizeof(filename), "%s/.Names", pathbuf);
		else
		{
			if (i < sizeof(pathbuf))
				pathbuf[i] = *ptr;
			ptr++;
			i++;
			continue;
		}
		if((fp = fopen(filename, "r")) == NULL)
			return -1;
		while(fgets(buf, sizeof(buf), fp) != NULL)
		{
			if ( strncmp( buf, "Name=", 5 ) == 0 )
			{
				strncpy( title, buf + 5, sizeof(title)-1 );
				title[sizeof(title)-1] = '\0';
			}
			else if ( strncmp( buf, "Path=", 5 ) == 0 )
			{
				if ( strncmp( buf, "Path=~/", 7 ) == 0 )
					snprintf(currpath, sizeof(currpath), "%s/%s", 
							 pathbuf, buf+7);
				else
					snprintf(currpath, sizeof(currpath), "%s/%s", 
							 pathbuf, buf+5);
				/*litem.fname[sizeof(litem.fname)-1] = '\0';*/
				if(strncmp(currpath, path, strlen(currpath)) != 0)
				{
					/* directory or file is not existed */
					fclose(fp);
					return -1;
				}
				if ((!strstr(title,"(BM: BMS)")||HAS_PERM(currentuser,PERM_BOARDS))&&
					(!strstr(title,"(BM: SYSOPS)")||HAS_PERM(currentuser,PERM_SYSOP))&&
					(!strstr(title,"(BM: ZIXIAs)")||HAS_PERM(currentuser,PERM_SECANC)))
				{
					/* directory can be accessed */
					break;
				}
				else
				{
					/* diretory cannot be accessed */
					fclose(fp);
					return -1;
				}
			}
		}
		fclose(fp);
		if (i < sizeof(pathbuf))
			pathbuf[i] = *ptr;
		ptr++;
		i++;
	}
	return 0;
}

void ann_show_item(MENU *pm, ITEM *it)
{
	char title[STRLEN];
	char *id;
	char buf[256];
	char *ptr;
	
	strncpy(title, it->title, sizeof(title)-1);
	title[sizeof(title)-1] = '\0';
	if (strlen(title) <= 39)
	{
		id = "";
	}
	else
	{
		title[38] = '\0';
		id = title + 39;
		if (!strncmp(id, "BM: ", 4))
			id += 4;
		ptr = strchr(id, ')');
		if (ptr)
			ptr[0] = '\0';
	}
	printf("<tr><td>%d</td>", pm->now + 1);
	sprintf(buf, "%s/%s", pm->path, it->fname);
	ptr = strchr(pm->path, '/');
	if (!file_exist(buf))
	{
		printf("<td>[错误] </td><td>%s</td>", nohtml(title));
	}
	else if (file_isdir(buf))
	{
		printf("<td>[目录] </td><td><a href=\"bbs0an?path=%s/%s\">%s</a></td>",
			   ptr, it->fname, nohtml(title));
	}
	else
	{
		printf("<td>[文件] </td><td><a href=\"bbsanc?path=%s/%s\">%s</a></td>",
			   ptr, it->fname, nohtml(title));
	}
	if (id[0])
	{
		printf("<td>%s</td>", userid_str(id));
	}
	else
	{
		printf("<td>&nbsp;</td>");
	}
	printf("<td>%6.6s %s</td></tr>\n", wwwCTime(file_time(buf))+4, wwwCTime(file_time(buf))+20);
}

void ann_display(char *path)
{
	MENU me;
	ITEM *its;
	int len;
	char board[STRLEN];
	char pathbuf[256];
	char buf[STRLEN];
	int i;
	
	if (strstr(path, "..") || strstr(path, "SYSHome")) /* SYSHome? */
		http_fatal("此目录不存在");
	len = strlen(path);
	if (path[len-1] == '/')
		path[len-1] = '\0';
	if (path[0] == '/')
		snprintf(pathbuf, sizeof(pathbuf), "0Announce%s", path);
	else
		snprintf(pathbuf, sizeof(pathbuf), "0Announce/%s", path);
	if (ann_traverse_check(pathbuf) < 0)
		http_fatal("此目录不存在");
	if ((its = ann_alloc_items(MAXITEMS)) == NULL)
		http_fatal("分配内存失败");
	ann_set_items(&me, its, MAXITEMS);
	me.path = pathbuf;
	if (ann_load_directory(&me) == 0)
	{
		board[0] = '\0';
		ann_get_board(path, board, sizeof(board));
		buf[0] = '\0';
		if (board[0] != '\0')
			sprintf(buf, "%s版", board);
		printf("<center>\n");
		printf("%s -- %s精华区<hr class=\"default\">\n", BBSNAME, buf);
		if (me.num <= 0)
		{
			printf("<br>&lt;&lt; 目前没有文章 &gt;&gt;\n");
			http_quit();
		}
		printf("<table class=\"default\" border=\"1\" width=\"610\">\n");
		printf("<tr><td>编号</td><td>类别</td><td>标题</td><td>整理</td><td>日期</td></tr>\n");
		me.now = 0;
		for (i = 0; i < me.num; i++)
		{
			trim(me.item[i]->title);
			ann_show_item(&me, me.item[i]);
			me.now++;
		}
		printf("</table><br>[<a href=\"javascript:history.go(-1)\">返回上一页</a>] ");
		if (board[0])
			printf("[<a href=\"bbsdoc?board=%s\">本讨论区</a>]\n", board);
		printf("</center>\n");
	}
	ann_free_items(its, MAXITEMS);
}

int main()
{
	char path[512];

	init_all();
	strsncpy(path, getparm("path"), 511);
	http_quit();
}

int get_count(char *path)
{
	FILE *fp;
	char buf[256];
	int counts=0;

	sprintf(buf, "0Announce%s/.counts", path);
	if (!file_exist(buf))
	{
		fp=fopen(buf, "w+");
	}
	else
	{
		fp=fopen(buf, "r+");
	}
	if (fp == NULL)
		return 0;
	flock(fileno(fp), LOCK_EX);
	fscanf(fp, "%d", &counts);
	counts++;
	fseek(fp, 0, SEEK_SET);
	fprintf(fp, "%d\n", counts);
	flock(fileno(fp), LOCK_UN);
	fclose(fp);
	return counts;
}
