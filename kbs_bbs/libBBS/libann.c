#include "bbs.h"

static int ann_can_access(char *title, char *board, struct userec *user)
{
    struct boardheader *bp;
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

int ann_load_directory(MENU * pm)
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
            litem.fname[sizeof(litem.fname) - 1] = '\0';
            /*if ((!strstr(litem.title,"(BM: BMS)")||HAS_PERM(currentuser,PERM_BOARDS))&&
               (!strstr(litem.title,"(BM: SYSOPS)")||HAS_PERM(currentuser,PERM_SYSOP))) */
            if (ann_can_access(litem.title, board, currentuser)) {
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
    if (ptr[0] == '\0')
        return -1;
    if (ptr[0] == '/')
        ptr++;
    fp = fopen("0Announce/.Search", "r");
    if (fp == NULL)
        return -1;
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        if ((ptr2 = strrchr(buf, '\n')) != NULL)
            *ptr2 = '\0';
        if ((ptr2 = strchr(buf, ':')) != NULL) {
            *ptr2 = '\0';
            if (strncmp(ptr2 + 2, ptr, strlen(ptr2 + 2)) == 0) {
                strncpy(board, buf, len - 1);
                board[len - 1] = '\0';
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
int ann_get_path(char *board, char *path, size_t len)
{
    FILE *fp;
    char buf[256];
    char *ptr;

    fp = fopen("0Announce/.Search", "r");
    if (fp == NULL)
        return -1;
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        if ((ptr = strrchr(buf, '\n')) != NULL)
            *ptr = '\0';
        if ((ptr = strchr(buf, ':')) != NULL) {
            *ptr = '\0';
            if (strcmp(buf, board) == 0) {
                strncpy(path, ptr + 2, len - 1);
                path[len - 1] = '\0';
                fclose(fp);
                return 0;
            }
        }
    }
    fclose(fp);
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
    if ((ptr = strstr(path, "groups/")) != NULL)
        ann_get_board(ptr, board, sizeof(board));
    else
        board[0] = '\0';
    bzero(pathbuf, sizeof(pathbuf));
    ptr = path;
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
            /*if ((!strstr(title,"(BM: BMS)")||HAS_PERM(currentuser,PERM_BOARDS))&&
               (!strstr(title,"(BM: SYSOPS)")||HAS_PERM(currentuser,PERM_SYSOP))&&
               (!strstr(title,"(BM: ZIXIAs)")||HAS_PERM(currentuser,PERM_SECANC))) */
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

/* Add a board searching path to .Search file. */
int ann_addto_search(char *group, char *board)
{
	char buf[PATHLEN];
	char searchname[STRLEN];

	strcpy(buf, "0Announce/.Search");
	sprintf(searchname, "%s: groups/%s/%s", board, group, board);
    if (!seek_in_file(buf, board))
	{
		addtofile(buf, searchname);
		return 0;
	}
	else
		return -1;
}

/* Delete a board searching path from .Search file. */
int ann_delfrom_search(char *board)
{
    FILE *fp, *nfp;
    int deleted = false;
	char filename[PATHLEN];
    char fnnew[256 /*STRLEN*/];
    char buf[256 /*STRLEN*/];

	strcpy(filename, "0Announce/.Search");
    if ((fp = fopen(filename, "r")) == NULL)
        return -1;
    sprintf(fnnew, "%s.%d", filename, getuid());
    if ((nfp = fopen(fnnew, "w")) == NULL)
        return -1;
    while (fgets(buf, 256 /*STRLEN*/, fp) != NULL) {
        if (strncasecmp(buf, board, strlen(board)) == 0)
            deleted = true;

        else if (*buf > ' ')
            fputs(buf, nfp);
    }
    fclose(fp);
    fclose(nfp);
    if (!deleted)
        return -1;
    return (f_mv(fnnew, filename));
}

