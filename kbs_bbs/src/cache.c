/* Written By Leeward@etang.com, Inc. 1999 */

#include "cache.h"


#define CACHE_ROOT "/backup/www/htdocs"


int
ca_mkdir(const char *path, int mode)
{
    char dir[MAX_CA_PATH_LEN + 64];
    FILE *fp;


    if (mode < 1)
        mode = 0755;

    if (!mkdir(path, mode))
        return 0;

    strcpy(dir, "/bin/mkdir -p ");
    strncat(dir, path, MAX_CA_PATH_LEN);
    dir[14 + MAX_CA_PATH_LEN] = 0;

    if (!(fp = popen(dir, "r")))
        return -10;
    else
        pclose(fp);

    if (!getcwd(dir, MAX_CA_PATH_LEN))
        return -20;

    if (-1 == chdir(path))
    {
        if (-1 == chdir(dir))
            return -30;
        else
            return -40;
    }
    else
    {
        if (-1 == chdir(dir))
            return -50;
        else
        {
            if (-1 == chmod(path, mode))
                return -60;
            else
                return 0;
        }
    }
}


int
ca_rmdir(const char *path)
{
    char dir[MAX_CA_PATH_LEN + 64];
    FILE *fp;


    if (!rmdir(path))
        return 0;

    strcpy(dir, "/bin/rm -fr ");
    strncat(dir, path, MAX_CA_PATH_LEN);
    dir[12 + MAX_CA_PATH_LEN] = 0;

    if (!(fp = popen(dir, "r")))
        return -10;
    else
        pclose(fp);

    if (!getcwd(dir, MAX_CA_PATH_LEN))
        return -20;

    if (!chdir(path))
    {
        if (-1 == chdir(dir))
            return -30;
        else
            return -40;
    }
    else
    {
        if (-1 == chdir(dir))
            return -50;
        else
            return 0;
    }
}


int
ca_badpath(const char *path)
{
    if ('/' != path[0])
        return -10;
    else if (!strchr(path + 1, '/'))
        return -20;
    else if (!strlen(strrchr(path, '/') + 1))
        return -30;
    else if (strstr(path, "../"))
        return -40;
    else if (strpbrk(path, "&;`'\"|*?~<>^()[]{}$\n"))
        return -50;
    else
        return 0;
}


FILE *
ca_fopen(const char *path, const char *mode)
{
    int  r;
    char dir[MAX_CA_PATH_LEN + 64];
    char *ptr;

    if (r = ca_badpath(path))
    {
        errno = r;
        return NULL;
    }
    else
    {
        strcpy(dir, CACHE_ROOT);
        strncat(dir, path, MAX_CA_PATH_LEN);
        dir[4 + MAX_CA_PATH_LEN] = 0;
        ptr = strrchr(dir, '/');
        *ptr = 0;
        if (r = ca_mkdir(dir, 0))
        {
            errno = r;
            return NULL;
        }
        else
        {
            *ptr = '/';
            return fopen(dir, mode);
        }
    }
}


int
ca_expire(const char *URL)
{
    int  r;
    char dir[MAX_CA_PATH_LEN + 64];

    if (r = ca_badpath(URL))
        return r;
    else
    {
        strcpy(dir, CACHE_ROOT);
        strncat(dir, URL, MAX_CA_PATH_LEN);
        dir[4 + MAX_CA_PATH_LEN] = 0;
        return ca_rmdir(dir);
    }
}

int
ca_expire_file(const char *URL)
{
    int  r;
    char dir[MAX_CA_PATH_LEN + 64];

/* KCN,speed up expire 
    if (r = ca_badpath(URL))
        return r;
    else
    {
*/
        strcpy(dir, CACHE_ROOT);
        strncat(dir, URL, MAX_CA_PATH_LEN);
        dir[4 + MAX_CA_PATH_LEN] = 0;
	return unlink(dir);
/*
        return ca_rmdir(dir);
    }
*/
}

