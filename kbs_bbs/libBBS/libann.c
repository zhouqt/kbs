#include "bbs.h"

static int ann_can_access(char *title, const char *board, struct userec *user)
{
    const struct boardheader *bp;
    char BM[STRLEN];

    if (strstr(title, "(BM: BMS)")) {
        if (board[0] == '\0')
            return 0;
        bp = getbcache(board);
        if (bp == NULL)
            return 0;
        strncpy(BM, bp->BM, sizeof(BM) - 1);
        BM[sizeof(BM) - 1] = '\0';
        if (chk_currBM(BM, user) == 0)
            return 0;
        return 2;
    }
    if (strstr(title, "(BM: SYSOPS)")) {
        if (!HAS_PERM(user, PERM_SYSOP))
            return 0;
        return 3;
    }
    return 1;
}

void ann_add_item(MENU * pm, ITEM * it)
{
    ITEM *newitem;

    if (pm->num < MAXITEMS) {
        newitem = pm->item[pm->num];
        strncpy(newitem->title, it->title, sizeof(newitem->title) - 1);
        if (it->host != NULL) {
            newitem->host = (char *) malloc(sizeof(char) * (strlen(it->host) + 1));
            strcpy(newitem->host, it->host);
        } else
            newitem->host = it->host;
        newitem->port = it->port;
        newitem->attachpos= it->attachpos;
        strncpy(newitem->fname, it->fname, sizeof(newitem->fname) - 1);
        (pm->num)++;
    }
}

int ann_load_directory(MENU * pm, session_t* session)
{
    FILE *fn;
    ITEM litem;
    char buf[PATHLEN];
    char *ptr;
    char hostname[STRLEN];
    char board[STRLEN];

    pm->num = 0;
    if ((ptr = strstr(pm->path, "groups/")) != NULL)
        ann_get_board(ptr, board, sizeof(board));
    else
        board[0] = '\0';
    snprintf(buf, sizeof(buf), "%s/.Names", pm->path);  /*.Names记录菜单信息 */
    if ((fn = fopen(buf, "r")) == NULL)
        return -1;
    bzero(&litem, sizeof(litem));
    hostname[0] = '\0';
    while (fgets(buf, sizeof(buf), fn) != NULL) {
        if ((ptr = strchr(buf, '\n')) != NULL)
            *ptr = '\0';
        if (strncmp(buf, "Name=", 5) == 0) {
            strncpy(litem.title, buf + 5, sizeof(litem.title) - 1);
            litem.title[sizeof(litem.title) - 1] = '\0';
            litem.attachpos = 0;
        } else if (strncmp(buf, "Path=", 5) == 0) {
            if (strncmp(buf, "Path=~/", 7) == 0)
                strncpy(litem.fname, buf + 7, sizeof(litem.fname) - 1);
            else
                strncpy(litem.fname, buf + 5, sizeof(litem.fname) - 1);
            if (strstr(litem.fname,"..")) continue;
            litem.fname[sizeof(litem.fname) - 1] = '\0';
            /*if ((!strstr(litem.title,"(BM: BMS)")||HAS_PERM(session->getCurrentUser(),PERM_BOARDS))&&
               (!strstr(litem.title,"(BM: SYSOPS)")||HAS_PERM(session->getCurrentUser(),PERM_SYSOP))) */
            if (ann_can_access(litem.title, board, session->currentuser)) {
                if (strstr(litem.fname, "!@#$%")) {     /*取 host & port */
                    char *ptr1, *ptr2, gtmp[STRLEN];

                    strncpy(gtmp, litem.fname, STRLEN - 1);
                    /* Warning: NOT thread safe */
                    ptr1 = strtok(gtmp, "!#$%@");
                    strcpy(hostname, ptr1);
                    ptr2 = strtok(NULL, "@");
                    strncpy(litem.fname, ptr2, sizeof(litem.fname) - 1);
                    litem.fname[sizeof(litem.fname) - 1] = '\0';
                    litem.port = atoi(strtok(NULL, "@"));
                }
                ann_add_item(pm, &litem);
            }
            hostname[0] = '\0';
        } else if (strncmp(buf, "# Title=", 8) == 0) {
            if (pm->mtitle[0] == '\0') {
                strncpy(pm->mtitle, buf + 8, sizeof(pm->mtitle) - 1);
                pm->mtitle[sizeof(pm->mtitle) - 1] = '\0';
            }
        } else if (strncmp(buf, "Host=", 5) == 0) {
            strncpy(hostname, buf + 5, sizeof(hostname) - 1);
            hostname[sizeof(hostname) - 1] = '\0';
        } else if (strncmp(buf, "Port=", 5) == 0) {
            litem.port = atoi(buf + 5);
        } else if (strncmp(buf, "Attach=", 7) == 0) {
            litem.attachpos= atol(buf + 7);
        }
    }
    fclose(fn);
    return 0;
}

ITEM *ann_alloc_items(size_t num)
{
    ITEM *it;

    it = (ITEM *) calloc(num, sizeof(ITEM));
    return it;
}

void ann_free_items(ITEM * it, size_t num)
{
    size_t i;

    if (it != NULL) {
        for (i = 0; i < num; i++) {
            if (it[i].host != NULL) {
                free(it[i].host);
                it[i].host = NULL;
            }
        }
        free(it);
    }
}

void ann_set_items(MENU * pm, ITEM * it, size_t num)
{
    size_t i;

    for (i = 0; i < num; i++)
        pm->item[i] = it + i;
}

struct _tmp_findboard {
    char *path;
    char *board;
    size_t len;
};

static int findboard(struct boardheader* bh,void* data)
{
    struct _tmp_findboard *arg = (struct _tmp_findboard*)data;

	if(! bh->ann_path || ! bh->ann_path[0] ) return 0;

    if (!strncmp(bh->ann_path,arg->path,strlen(bh->ann_path))) {
        strncpy(arg->board,bh->filename,arg->len);
        arg->board[arg->len-1]=0;
        return QUIT;
    }
    return 0;
}

/*
 * Get a board name from an announce path.
*/
int ann_get_board(char *path, char *board, size_t len)
{
    char *ptr;
    struct _tmp_findboard arg;

    ptr = path;
    if (ptr[0] == '\0')
        return -1;
    if (ptr[0] == '/')
        ptr++;
    if (! strncmp(path,"0Announce/groups/",strlen("0Announce/groups/"))){
	    arg.path=path+strlen("0Announce/groups/");
	    arg.board=board;
	    arg.len=len;
	    if (apply_boards(findboard,&arg)==QUIT)
	        return 0;
	    return -1;
	}else if(! strncmp(path,"groups/",7)){
	    arg.path=path+7;
	    arg.board=board;
	    arg.len=len;
	    if (apply_boards(findboard,&arg)==QUIT)
	        return 0;
	    return -1;
	}
	return -1;
}

/*
 * Get an announce path from a board name.
*/
int ann_get_path(char *board, char *path, size_t len)
{
    const struct boardheader* bh;
    if ((bh=getbcache(board))!=NULL) {
        if (strlen(bh->ann_path)+strlen("/groups")>len)
            return -1;
        sprintf(path,"/groups/%s",bh->ann_path);
        return 0;
    }
    return -1;
}

int ann_traverse_check(char *path, struct userec *user)
{
    char *ptr;
    char *ptr2;
    size_t i = 0;
    char filename[256];
    char buf[256];
    char pathbuf[256];
    char title[STRLEN];
    char currpath[256];
    FILE *fp;
    char board[STRLEN];
    int ret = 0;

    /* path parameter can not have leading '/' character */
    if (path[0] == '/')
        return -1;
    board[0] = '\0';
    if ((ptr = strstr(path, "groups/")) != NULL)
        ann_get_board(ptr, board, sizeof(board));
    bzero(pathbuf, sizeof(pathbuf));
    if (board[0] == '\0') {
        ptr = path;
    } else {
        if (check_read_perm(user, getbcache(board)) == 0) return -1;
        ann_get_path(board, filename, sizeof(filename));
        snprintf(pathbuf, sizeof(pathbuf), "0Announce%s", filename);
        ptr = path + strlen(pathbuf);
        i = strlen(pathbuf);
    }
    while (*ptr != '\0') {
        if (*ptr == '/')
            snprintf(filename, sizeof(filename), "%s/.Names", pathbuf);
        else {
            if (i < sizeof(pathbuf))
                pathbuf[i] = *ptr;
            ptr++;
            i++;
            continue;
        }
        if ((fp = fopen(filename, "r")) == NULL)
            return -1;
        while (fgets(buf, sizeof(buf), fp) != NULL) {
            int t;

            if ((ptr2 = strrchr(buf, '\n')) != NULL)
                *ptr2 = '\0';
            if (strncmp(buf, "Name=", 5) == 0) {
                strncpy(title, buf + 5, sizeof(title) - 1);
                title[sizeof(title) - 1] = '\0';
                continue;
            }
            if (strncmp(buf, "Path=~/", 7) == 0)
                snprintf(currpath, sizeof(currpath), "%s/%s", pathbuf, buf + 7);
            else if (strncmp(buf, "Path=", 5) == 0)
                snprintf(currpath, sizeof(currpath), "%s/%s", pathbuf, buf + 5);
            else
                continue;
            if (strncmp(currpath, path, strlen(currpath)) != 0)
                continue;
			if (path[strlen(currpath)] != '/' && path[strlen(currpath)]!='\0' ) continue;
            /*if ((!strstr(title,"(BM: BMS)")||HAS_PERM(session->getCurrentUser(),PERM_BOARDS))&&
               (!strstr(title,"(BM: SYSOPS)")||HAS_PERM(session->getCurrentUser(),PERM_SYSOP))&&
               (!strstr(title,"(BM: ZIXIAs)")||HAS_PERM(session->getCurrentUser(),PERM_SECANC))) */
            if ((t = ann_can_access(title, board, user)) != 0) {
                if (ret < t)
                    ret = t;    /* directory can be accessed but it should be access with some
                                   permission */
                break;
            } else {
                /* diretory cannot be accessed */
                fclose(fp);
                return -1;
            }
        }
        if (feof(fp)) {
            fclose(fp);
            return -1;
        }
        fclose(fp);
        if (i < sizeof(pathbuf))
            pathbuf[i] = *ptr;
        ptr++;
        i++;
    }
    return ret;
}

char * ann_numtopath(char *path, char *numpath, struct userec *user)
{
	int bid=0;
	char *c;
	char *ptr = NULL;
	const struct boardheader *bh = NULL;
    char filename[256];
    FILE *fp;
    char buf[256];
	int endfile=0;
    char currpath[256];
    char title[STRLEN];
	int ok;

	path[0]='\0';
	title[0]='\0';
	currpath[0]='\0';

	while(1){

		if(path[0]=='\0'){
			c=strchr(numpath, '-');
			if(c==NULL) return NULL;
			*c='\0';
			bid = atoi(numpath);

			if((bh=getboard(bid))==NULL) return NULL;

    		if (check_read_perm(user, bh) == 0)
		        return NULL;

		    snprintf(path,255,"0Announce/groups/%s",bh->ann_path);

			ptr = c + 1;

			continue;
		}else{
			if(ptr[0]=='\0') break;
			c = strchr(ptr, '-');
			if(c==NULL) endfile=1;
			else{
				*c='\0';
			}
			bid = atoi(ptr);
			if(c!=NULL) ptr = c+1;
			if(bid <=0) return NULL;
		}

	    snprintf(filename, sizeof(filename), "%s/.Names", path);

		ok = 0;
        if ((fp = fopen(filename, "r")) == NULL)
            return NULL;
        while (fgets(buf, sizeof(buf), fp) != NULL) {
            if ((c = strrchr(buf, '\n')) != NULL)
                *c = '\0';
            if (strncmp(buf, "Name=", 5) == 0) {
                strncpy(title, buf + 5, sizeof(title) - 1);
                title[sizeof(title) - 1] = '\0';
                continue;
            }else if (strncmp(buf, "Path=~/", 7) == 0){
                snprintf(currpath, sizeof(currpath), "%s/%s", path, buf + 7);
				continue;
			}else if (strncmp(buf, "Path=", 5) == 0){
                snprintf(currpath, sizeof(currpath), "%s/%s", path, buf + 5);
				continue;
			}else if(strncmp(buf, "Numb=", 5) == 0){
				if(bid != atoi(buf+5)){
					title[0]='\0';
					currpath[0]='\0';
					continue;
				}
            	if (ann_can_access(title, bh->filename, user) == 0) {
					break;
				}else{
					ok = 1;
					strcpy(path, currpath);
					break;
				}
			}else
				continue;
        }
        fclose(fp);
		if(!ok)
			return NULL;
		if(endfile)
			break;
    }
	if(path[0]=='/' || strncmp(path, "0Announce/groups/", 17) || strstr(path, "..") ) return NULL;
    return path;
}

void a_freenames(MENU * pm)
{
    int i;

    for (i = 0; i < pm->num; i++)
        free(pm->item[i]);
}

void a_additem(MENU* pm,const char* title,const char* fname,char* host,int port,long attachpos)    /* 产生ITEM object,并初始化 */
{
    ITEM *newitem;

    if (pm->num < MAXITEMS) {
        newitem = (ITEM *) malloc(sizeof(ITEM));
        strncpy(newitem->title, title, sizeof(newitem->title) - 1);
        if (host != NULL) {
            newitem->host = (char *) malloc(sizeof(char) * (strlen(host) + 1));
            strcpy(newitem->host, host);
        } else
            newitem->host = host;
        newitem->port = port;
        newitem->attachpos = attachpos;
        strncpy(newitem->fname, fname, sizeof(newitem->fname) - 1);
        pm->item[(pm->num)++] = newitem;
    }
}

#ifdef FB2KPC
int fb2kpc_is_owner(char *path)
{
	char *c;
	char owner[IDLEN+1];

	if(strlen(path) < strlen(FB2KPC)+3)
		return 0;
	c=path+strlen(FB2KPC);
	if(*c=='/') c++;
	c+=2;
	strncpy(owner, c, IDLEN);
	owner[IDLEN]='\0';
	if((c=strchr(owner,'/'))!=NULL) *c='\0';
	if(strcasecmp(owner,getCurrentUser()->userid))
		return 0;
	return 1;
}
#endif

#ifdef ANN_CTRLK
static int canread(int level, char *path, char * fname, char *title)
{
	char buf[PATHLEN+20];

	if(strlen(path)+strlen(fname) > PATHLEN)
		return 0;

	sprintf(buf,"%s/%s",path,fname);

#ifdef FB2KPC
	if(!strncmp(path,FB2KPC,strlen(FB2KPC))){
		if(fb2kpc_is_owner(buf))
			return 1;
		if(strstr(title,"<secret>"))
			return 0;
	}else{
#endif
	if(level & PERM_BOARDS) return 1;
#ifdef FB2KPC
	}
#endif

	if(dashd(buf)){
		strcat(buf,"/.allow");
		if(!dashf(buf)) return 1;
		if(!seek_in_file(buf,getCurrentUser()->userid)) return 0;
		return 1;
	}
	return 1;
}
#endif

int a_loadnames(MENU* pm, session_t* session)             /* 装入 .Names */
{
    FILE *fn;
    ITEM litem;
    char buf[PATHLEN], *ptr;
    char hostname[STRLEN];
    struct stat st;

    a_freenames(pm);
    pm->num = 0;
#ifdef ANN_COUNT
	sprintf(buf, "%s/counter.person", pm->path);
	if(fn=fopen(buf,"r")){
		fscanf(fn,"%d", &(pm->count));
		fclose(fn);
	}else
		pm->count = 0;

	pm->count++;
	if(fn=fopen(buf,"w")){
		fprintf(fn,"%d",pm->count);
		fclose(fn);
	}
#endif
    sprintf(buf, "%s/.Names", pm->path);        /*.Names记录菜单信息 */
    if ((fn = fopen(buf, "r")) == NULL)
        return 0;
    if (fstat(fileno(fn), &st) != -1)
        pm->modified_time = st.st_mtime;
    hostname[0] = '\0';
    while (fgets(buf, sizeof(buf), fn) != NULL) {
        if ((ptr = strchr(buf, '\n')) != NULL)
            *ptr = '\0';
        if (strncmp(buf, "Name=", 5) == 0) {
            strncpy(litem.title, buf + 5, sizeof(litem.title));
            litem.attachpos = 0;
        } else if (strncmp(buf, "Path=", 5) == 0) {
            if (strncmp(buf, "Path=~/", 7) == 0)
                strncpy(litem.fname, buf + 7, sizeof(litem.fname));
            else
                strncpy(litem.fname, buf + 5, sizeof(litem.fname));
            if (strstr(litem.fname,"..")) continue;
#ifdef ANN_CTRLK
			if (!canread(pm->level, pm->path, litem.fname, litem.title)) continue;
#endif
            if (HAS_PERM(session->currentuser, PERM_SYSOP)
                ||((!strstr(litem.title, "(BM: BMS)") || HAS_PERM(session->currentuser, PERM_BOARDS))
                 &&(!strstr(litem.title, "(BM: SYSOPS)") || HAS_PERM(session->currentuser, PERM_SYSOP)) 
                 &&(!strstr(litem.title, "(BM: ZIXIAs)") || HAS_PERM(session->currentuser, PERM_SECANC)))){
                if (strstr(litem.fname, "!@#$%")) {     /*取 host & port */
                    char *ptr1, *ptr2, gtmp[STRLEN];

                    strncpy(gtmp, litem.fname, STRLEN - 1);
                    ptr1 = strtok(gtmp, "!#$%@");
                    strcpy(hostname, ptr1);
                    ptr2 = strtok(NULL, "@");
                    strncpy(litem.fname, ptr2, sizeof(litem.fname) - 1);
                    litem.port = atoi(strtok(NULL, "@"));
                }
                a_additem(pm, litem.title, litem.fname, (strlen(hostname) == 0) ?       /*产生ITEM */
                          NULL : hostname, litem.port, litem.attachpos);
            }
            hostname[0] = '\0';
        } else if (strncmp(buf, "# Title=", 8) == 0) {
            if (pm->mtitle[0] == '\0')
                strncpy(pm->mtitle, buf + 8, STRLEN);
        } else if (strncmp(buf, "Host=", 5) == 0) {
            strcpy(hostname, buf + 5);
        } else if (strncmp(buf, "Port=", 5) == 0) {
            litem.port = atoi(buf + 5);
        } else if (strncmp(buf, "Attach=", 7) == 0) {
            litem.attachpos= atoi(buf + 7);
        }
    }
    fclose(fn);
    return 1;
}

int a_savenames(MENU* pm)             /*保存当前MENU到 .Names */
{
    FILE *fn;
    ITEM *item;
    char fpath[PATHLEN];
    int n;
    struct stat st;

    sprintf(fpath, "%s/.Names", pm->path);
    if (stat(fpath, &st) != -1) {
        if (st.st_mtime != pm->modified_time)
            return -3;
    }
    if ((fn = fopen(fpath, "w")) == NULL)
        return -1;
    fprintf(fn, "#\n");
    if (!strncmp(pm->mtitle, "[目录] ", 7) || !strncmp(pm->mtitle, "[文件] ", 7)
        || !strncmp(pm->mtitle, "[连线] ", 7)) {
        fprintf(fn, "# Title=%s\n", pm->mtitle + 7);
    } else {
        fprintf(fn, "# Title=%s\n", pm->mtitle);
    }
    fprintf(fn, "#\n");
    for (n = 0; n < pm->num; n++) {
        item = pm->item[n];
        if (!strncmp(item->title, "[目录] ", 7) || !strncmp(item->title, "[文件] ", 7)
            || !strncmp(item->title, "[连线] ", 7)) {
            fprintf(fn, "Name=%s\n", item->title + 7);
        } else
            fprintf(fn, "Name=%s\n", item->title);
        fprintf(fn, "Attach=%ld\n", item->attachpos);
        if (item->host != NULL) {
            fprintf(fn, "Host=%s\n", item->host);
            fprintf(fn, "Port=%d\n", item->port);
            fprintf(fn, "Type=1\n");
            fprintf(fn, "Path=%s\n", item->fname);
        } else
            fprintf(fn, "Path=~/%s\n", item->fname);
        fprintf(fn, "Numb=%d\n", n + 1);
        fprintf(fn, "#\n");
    }
    fclose(fn);
    if (stat(fpath, &st) != -1)
        pm->modified_time = st.st_mtime;
    chmod(fpath, 0644);
    return 0;
}

int save_import_path(char **i_path,char **i_title,time_t* i_path_time, session_t* session)
{
    FILE *fn;
    int i;
    char buf[MAXPATH];

    sethomefile(buf, session->currentuser->userid, "BMpath");
    fn = fopen(buf, "wt");
    if (fn) {
        struct stat st;

        for (i = 0; i < ANNPATH_NUM; i++) {
            fputs(i_path[i], fn);
            fputs("\n", fn);
            fputs(i_title[i], fn);
            fputs("\n", fn);
        }
        fstat(fileno(fn), &st);
        fclose(fn);
        * i_path_time = st.st_mtime;
        return 0;
    }
    return -1;
}

void load_import_path(char ** i_path,char ** i_title, time_t* i_path_time,int * i_path_select, session_t* session)
{
    FILE *fn;
    char buf[MAXPATH];
    int i;
    struct stat st;

    sethomefile(buf, session->currentuser->userid, "BMpath");
    if (stat(buf, &st) != -1)
        if (st.st_mtime == * i_path_time)
            return;
    if (* i_path_select != 0)
        free_import_path(i_path,i_title,i_path_time);
    fn = fopen(buf, "rt");
    if (fn) {
        * i_path_time = st.st_mtime;
        for (i = 0; i < ANNPATH_NUM; i++) {
            if (!feof(fn)) {
                fgets(buf, MAXPATH - 1, fn);
                if (buf[strlen(buf) - 1] == '\n')
                    buf[strlen(buf) - 1] = 0;
            } else
                buf[0] = 0;
            /*
             * TODO: access check need complete!
             * if (buf[0]!=0&&(ann_traverse_check(buf, session->getCurrentUser())!=0))
             * buf[0]=0;  can't access 
             */

            i_path[i] = (char *) malloc(strlen(buf) + 1);
            strcpy(i_path[i], buf);
            if (!feof(fn)) {
                fgets(buf, MAXPATH - 1, fn);
                if (buf[strlen(buf) - 1] == '\n')
                    buf[strlen(buf) - 1] = 0;
            } else {            //get the title of pm
                buf[0] = 0;
                if (i_path[i][0]) {
                    MENU pm;

                    bzero(&pm, sizeof(pm));
                    pm.path = i_path[i];
                    a_loadnames(&pm, session);
                    strncpy(buf, pm.mtitle, MAXPATH - 1);
                    buf[MAXPATH - 1] = 0;
                    a_freenames(&pm);
                }
            }
            if (i_path[i][0] == 0) /* if invalid path,then let the title empty */
                buf[0] = 0;
            i_title[i] = (char *) malloc(strlen(buf) + 1);
            strcpy(i_title[i], buf);
        }
        fclose(fn);
    } else {
        for (i = 0; i < ANNPATH_NUM; i++) {
            i_path[i] = (char *) malloc(1);
            i_path[i][0] = 0;
            i_title[i] = (char *) malloc(1);
            i_title[i][0] = 0;
        }
        save_import_path(i_path,i_title,i_path_time, session);
    }
    * i_path_select = 1;
}

void free_import_path(char ** i_path,char ** i_title,time_t * i_path_time)
{
    int i;

    for (i = 0; i < ANNPATH_NUM; i++) {
        if (i_path[i] != NULL) {
            free(i_path[i]);
            i_path[i] = NULL;
        }
        if (i_title[i] != NULL) {
            free(i_title[i]);
            i_title[i] = NULL;
        }
    }
    * i_path_time = 0;
}

int valid_fname(str)
char *str;
{
    char ch;

	if(strstr(str,"..")) return 0;

    while ((ch = *str++) != '\0') {
        if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || strchr("0123456789@[]-._", ch) != NULL) {
            ;
        } else {
            return 0;
        }
    }
    return 1;
}

int linkto(char *path, const char *fname, const char *title, session_t* session)
{
    MENU pm;

    bzero(&pm,sizeof(pm));
    pm.path = path;

    strcpy(pm.mtitle, title);
    a_loadnames(&pm, session);
    a_additem(&pm, title, fname, NULL, 0, 0);
    if (a_savenames(&pm) != 0) {
#ifdef BBSMAIN
        char buf[80], ans[40];

        sprintf(buf, "整理精华区失败，可能有其他版主在处理同一目录，按 Enter 继续 ");
        a_prompt(-1, buf, ans);
#endif
    }
    a_freenames(&pm);
    return 0;
}



/*etnlegend,2005.06.28,修改精华区*/
int ann_show_board(const struct boardheader *bh){
    if(bh->title_level||bh->flag&(BOARD_CLUB_READ|BOARD_CLUB_HIDE))
        return 0;
    if(!(bh->level)||bh->level&PERM_POSTMASK||bh->level&PERM_DEFAULT)
        return 1;
    return 0;
}
int add_group(const struct boardheader *bh){
    MENU m;
    FILE *fp;
    char gpath[256],bpath[256],*p;
    char genbuf[1024];
    int ret,i;
    if(!bh)
        return 0;
    bzero(&m,sizeof(MENU));
    ret=0;
    if(!dashd("0Announce")){
        mkdir("0Announce",0755);
        chmod("0Announce",0755);
        if(!(fp=fopen("0Announce/.Names","w")))
            return -1;
        fprintf(fp,"#\n# Title=%s 精华区公布栏\n#\n",BBS_FULL_NAME);
        fclose(fp);
    }
    if(!dashd("0Announce/groups")){
        mkdir("0Announce/groups",0755);
        chmod("0Announce/groups",0755);
        m.path="0Announce";
        a_loadnames(&m,getSession());
        a_additem(&m,"讨论区精华","groups",NULL,0,0);
        if(a_savenames(&m))
            ret|=(1<<0);
    }
    sprintf(gpath,"0Announce/groups/%s",bh->ann_path);
    p=strrchr(gpath,'/');
    *p++=0;
    if(!dashd(gpath)){
        mkdir(gpath,0755);
        chmod(gpath,0755);
        for(i=0;groups[i];i++)
            if(!strcmp(&gpath[17],groups[i]))
                break;
        m.path="0Announce/groups";
        a_loadnames(&m,getSession());
        a_additem(&m,(groups[i]?secname[i][0]:&gpath[17]),&gpath[17],NULL,0,0);
        if(a_savenames(&m))
            ret|=(1<<1);
    }
    sprintf(bpath,"0Announce/groups/%s",bh->ann_path);
    if(!dashd(bpath)){
        char buf[128];
        mkdir(bpath,0755);
        chmod(bpath,0755);
        sprintf(buf,"%s/.Names",bpath);
        if(!(fp=fopen(buf,"w")))
            return -1;
        fprintf(fp,"#\n# Title=%-32.32s",&bh->title[13]);
        if(bh->BM[0])
            fprintf(fp,"(BM: %s)",bh->BM);
        fprintf(fp,"\n#\n");
        fclose(fp);
        m.path=gpath;
        a_loadnames(&m,getSession());
        sprintf(genbuf,"%s/%s",bh->filename,&bh->title[13]);
        if(!ann_show_board(bh))
            sprintf(buf,"%-38.38s(BM: SYSOPS)",genbuf);
        else
            sprintf(buf,"%-38.38s",genbuf);
        a_additem(&m,buf,p,NULL,0,0);
        if(a_savenames(&m))
            ret|=(1<<3);
    }
    return ret;
}
int del_group(const struct boardheader *bh){
    MENU m;
    char path[256],*p;
    int ret,i;
    if(!bh)
        return 0;
    bzero(&m,sizeof(MENU));
    ret=0;
    sprintf(path,"0Announce/groups/%s",bh->ann_path);
    my_f_rm(path);
    m.path=path;
    p=strrchr(path,'/');
    *p++=0;
    a_loadnames(&m,getSession());
    for(i=0;i<m.num;i++)
        if(!strcmp(m.item[i]->fname,p)){
            free(m.item[i]);
            m.num--;
            memmove(&m.item[i],&m.item[i+1],(m.num-i)*sizeof(ITEM*));
        }
    if(a_savenames(&m))
        ret|=(1<<0);
    a_freenames(&m);
    return ret;
}
int edit_group(const struct boardheader *oldbh,const struct boardheader *newbh){
    MENU m;
    char path[256],*p;
    char genbuf[1024];
    int ret,i;
    /*无效参数*/
    if(!oldbh&&!newbh)
        return 0;
    /*目的精华区位置检测*/
    if (newbh) {
        sprintf(path,"0Announce/groups/%s",newbh->ann_path);
        if((!oldbh||strcmp(oldbh->ann_path,newbh->ann_path))&&dashd(path))
            del_group(newbh);
    }
    /*增加*/
    if(!oldbh)
        return add_group(newbh);
    /*删除*/
    if(!newbh)
        return del_group(oldbh);
    /*源精华区位置检测*/
    sprintf(path,"0Announce/groups/%s",oldbh->ann_path);
    if(!dashd(path)){
        del_group(oldbh);
        return add_group(newbh);
    }
    /*修改*/
    bzero(&m,sizeof(MENU));
    ret=0;
    if(strcmp(&oldbh->title[13],&newbh->title[13])||strcmp(oldbh->BM,newbh->BM)){/*需要更新标题和版主*/
        /*对精华区标题的修改*/
        m.path=path;
        a_loadnames(&m,getSession());
        sprintf(genbuf,"%-32.32s",&newbh->title[13]);
        if(newbh->BM[0])
            sprintf(&genbuf[32],"(BM: %s)",newbh->BM);
        snprintf(m.mtitle,MTITLE_LEN,"%s",genbuf);
        if(a_savenames(&m))
            ret|=(1<<0);
        /*对精华公布栏的修改*/
        p=strrchr(path,'/');
        *p++=0;
        a_loadnames(&m,getSession());
        for(i=0;i<m.num;i++)
            if(!strcmp(m.item[i]->fname,p)){
                sprintf(genbuf,"%s/%s",newbh->filename,&newbh->title[13]);
                if((strlen(m.item[i]->title)>38&&!strncmp(&m.item[i]->title[38],"(BM: SYSOPS)",12)&&ann_show_board(oldbh))
                    ||!ann_show_board(newbh))
                    sprintf(m.item[i]->title,"%-38.38s(BM: SYSOPS)",genbuf);
                else
                    sprintf(m.item[i]->title,"%-38.38s",genbuf);
            }
        if(a_savenames(&m))
            ret|=(1<<1);
        a_freenames(&m);
    }
    if(strcmp(oldbh->ann_path,newbh->ann_path)){/*需要更新路径*/
        char oldpath[256],newpath[256];
        sprintf(oldpath,"0Announce/groups/%s",oldbh->ann_path);
        sprintf(newpath,"0Announce/groups/%s",newbh->ann_path);
        add_group(newbh);
        f_rm(newpath);
        if(rename(oldpath,newpath)==-1)
            ret|=(1<<2);
        else if(del_group(oldbh))
            ret|=(1<<3);
    }
    return ret;
}


/* a_SeSave 用来删除存到暂存档时的文件头和尾 Life 1997.4.6 */
/* mode-- 0 小b操作,保留引文, 1 大B操作,去掉引文 */
/*    path: 要收录的文件路径，没用。
      key: 版名。
      fileinfo: 要收录的文件头。
      append: 是否追加在原有的暂存档后面。
      direct: 没用。
      ent: 没用。
      mode: 模式，上面有说明。
      userid: 操作者的ID。
 */
int a_SeSave(char *path, const char *key, struct fileheader *fileinfo, bool append, char *direct, int ent,int mode, char *userid)
{

    FILE *inf, *outf;
    char qfile[STRLEN], filepath[STRLEN], genbuf[PATHLEN];
    char buf[256];
    bool findattach=false;
    struct fileheader savefileheader;
    char userinfo[STRLEN],posttime[STRLEN];
    char* t;
		
    sprintf(qfile, "boards/%s/%s", key, fileinfo->filename);
    sprintf(filepath, "tmp/se.%s", userid);
    outf = fopen(filepath, "w");
    if (*qfile != '\0' && (inf = fopen(qfile, "r")) != NULL) {
        fgets(buf, 256, inf);
	
	t = strrchr(buf,')');
	if (t) { 
		*(t+1)='\0';
	    memcpy(userinfo,buf+8,STRLEN);
	} else strcpy(userinfo,"未知发信人");
	fgets(buf, 256, inf);
	fgets(buf, 256, inf);
	t = strrchr(buf,')');
	if (t) {
		*(t+1)='\0';
	    if (NULL!=(t = strchr(buf,'(')))
			memcpy(posttime,t,STRLEN);
		else
			strcpy(posttime,"未知时间");
	} else 
			strcpy(posttime,"未知时间");
										
        fprintf(outf, "\033[0;1;32m☆─────────────────────────────────────☆\033[0;37m\n");
        fprintf(outf, "  \033[0;1;32m %s \033[0;1;37m于 \033[0;1;36m %s \033[0;1;37m 提到:\033[m\n", userinfo,posttime);

	fprintf(outf,"\n");	
        while (fgets(buf, 256, inf) != NULL)
            if (buf[0] == '\n')
                break;

        while (fgets(buf, 256, inf) != NULL) {
            /*结束*/
            if(!strcmp(buf,"--\n"))
                break;
            /*引文*/
            if((mode==1)&&(strstr(buf,": ")==buf||(strstr(buf,"【 在")==buf&&strstr(buf,") 的大作中提到: 】"))))
                continue;
            /*来源和修改信息*/
            if((strstr(buf,"\033[m\033")==buf&&strstr(buf,"※ 来源:·")==buf+10)
                ||(strstr(buf,"\033[36m※ 修改:·")==buf))
                continue;
            if (fileinfo->attachment&&
                !memcmp(buf,ATTACHMENT_PAD,ATTACHMENT_SIZE)) {
                findattach=true;
                break;
            }
            /* if (buf[250] != '\0')
                strcpy(buf + 250, "\n"); - disabled by atppp */
            fprintf(outf, "%s", buf);
        }
        fprintf(outf, "\n\n");
        fclose(inf);
    }

    fclose(outf);
    memcpy(&savefileheader,fileinfo,sizeof(savefileheader));
    savefileheader.attachment=0;
    if (fileinfo->attachment) {
        int fsrc,fdst;
        char *src = (char *) malloc(BLK_SIZ);
        if ((fsrc = open(qfile, O_RDONLY)) >= 0) {
            lseek(fsrc,fileinfo->attachment-1,SEEK_SET);
            sprintf(genbuf,"tmp/bm.%s.attach",userid);
            if ((fdst=open(genbuf,O_WRONLY | O_CREAT | O_APPEND, 0600)) >= 0) {
                long ret;
                do {
                    ret = read(fsrc, src, BLK_SIZ);
                    if (ret <= 0)
                        break;
                } while (write(fdst, src, ret) > 0);
                close(fdst);
            }
            close(fsrc);
        }
        free(src);
    }
    if (append)/* 如果需要附加暂存档，调用a_Save去保存正文。*/
        a_Save(filepath, key, &savefileheader, true,NULL,ent, userid);
    else {
        sprintf(qfile, "tmp/bm.%s", userid);
        f_mv(filepath,qfile);
    }
    return 1;
}


/* added by netty to handle post saving into (0)Announce */
int a_Save(char *path, const char *key, struct fileheader *fileinfo, bool append, char *direct, int ent, char *userid)
{
    char board[STRLEN], genbuf[PATHLEN];
    char buf[256];
    char* filepath;

    int mode;
    int fsrc,fdst1,fdst2;
    if (append)
        mode=O_APPEND;
    else
        mode=O_TRUNC;
    if (path==NULL) {
        sprintf(buf, "boards/%s/%s", key, fileinfo->filename);
        filepath=buf;
    } else filepath=path;
    sprintf(board, "tmp/bm.%s", userid);
    if ((fsrc = open(filepath, O_RDONLY)) >= 0) {
        sprintf(genbuf,"tmp/bm.%s.attach",userid);
        if ((fdst2=open(board,O_WRONLY | O_CREAT | mode, 0600)) >= 0) {
            int ret=0;
            char *src = (char *) malloc(BLK_SIZ);
            long saved=0,needsave=0;
            if ((fdst1=open(genbuf,O_WRONLY | O_CREAT | mode, 0600)) >= 0) {
                do {
                    /* read content and save (fileinfo->attachment) bytes */
                    ret = read(fsrc, src, BLK_SIZ);
                    if (ret <= 0)
                        break;
                    if (fileinfo->attachment) {
                        if (saved+ret>fileinfo->attachment-1) {
                            needsave=fileinfo->attachment-1-saved;
                        } else needsave=ret;
                    } else needsave=ret;
                    saved+=needsave;
                } while ((write(fdst2, src, needsave) > 0)&&(needsave==ret));
                close(fdst2);
            }
            if ((needsave!=ret)&&(ret>0))
                write(fdst1, src+needsave, ret-needsave);
            if (fileinfo->attachment)
                /* save attachment */
                do {
                    ret = read(fsrc, src, BLK_SIZ);
                    if (ret <= 0)
                        break;
                } while (write(fdst1, src, ret) > 0);
            free(src);
            close(fdst1);
        }
        close(fsrc);
    }
    /*sprintf(buf, "将 boards/%s/%s 存入暂存档", key, fileinfo->filename);
    a_report(buf);*/
    return 1;
}

/* 将附件附加到文章后面 返回attachpos */
long a_append_attachment(char *fpath, char *attachpath)
{
    struct stat st;
    int fsrc,fdst;
    long attachpos=0;

    if ((fsrc = open(attachpath, O_RDONLY)) != NULL) {
        fstat(fsrc,&st);
        if (st.st_size>0)
            if ((fdst = open(fpath, O_RDWR , 0600)) >= 0) {
                char *src = (char *) malloc(BLK_SIZ);
                long ret;
                fstat(fdst,&st);
                if (st.st_size>1) {
                    lseek(fdst,st.st_size-1,SEEK_SET);
                    read(fdst,src,1);
                    if (src[0]!='\n')
                        write(fdst, "\n", 1);
                }
                do {
                    ret = read(fsrc, src, BLK_SIZ);
                    if (ret <= 0)
                        break;
                } while (write(fdst, src, ret) > 0);
                close(fdst);
                attachpos=st.st_size;
                free(src);
            }
            close(fsrc);
    }
    return attachpos;
}

