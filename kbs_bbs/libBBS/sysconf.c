/*
	KCN,use mmap for system config.
*/
#include "bbs.h"
#define SC_BUFSIZE              20480
#define SC_KEYSIZE              256
#define SC_CMDSIZE              256
#define sysconf_ptr( offset )   (&sysconf_buf[ offset ]);

struct smenuitem *menuitem;
int sysconf_menu;
struct sdefine {
    char *key, *str;
    int val;
};
static struct sdefine *sysvar;

struct sysheader {
    char *buf;
    int menu, key, len;
};
static char *sysconf_buf;
static int sysconf_key, sysconf_len;
static POINTDIFF sysconf_diff;

static char *sysconf_ptr = NULL;
static int sysconf_size;
static int sysconf_version = -1;
static void *sysconf_addstr(char *str)
{
    int len = sysconf_len;
    char *buf;

    buf = sysconf_buf + len;
    strcpy(buf, str);
    sysconf_len = len + strlen(str) + 1;
    return buf;
}

const char *sysconf_str(const char *key)
{
    int n;

    if (sysconf_version == -1)
        load_sysconf();
    for (n = 0; n < sysconf_key; n++)
        if (strcmp(key, sysvar[n].key + sysconf_diff) == 0)
            return (sysvar[n].str + sysconf_diff);
    return NULL;
}

const char *sysconf_str_default(const char *key, const char *default_value)
{
	const char *val = sysconf_str(key);
	if (val != NULL)
		return val;
	else
		return default_value;
}

int sysconf_eval(const char *key,int defaultval)
{
    int n;

    if (sysconf_version == -1)
        load_sysconf();
    for (n = 0; n < sysconf_key; n++)
        if (strcmp(key, sysvar[n].key + sysconf_diff) == 0)
            return (sysvar[n].val);
    return defaultval;
}

char* sysconf_relocate(const char *data){
    return (char*)data+sysconf_diff;
}

struct smenuitem *sysconf_getmenu(const char *menu_name)
{
    if (sysconf_version == -1)
        load_sysconf();
    return &menuitem[sysconf_eval(menu_name,0)];
}

static void sysconf_addkey(char *key, char *str, int val)
{
    int num;

    if (sysconf_key < SC_KEYSIZE) {
        if (str == NULL)
            str = sysconf_buf;
        else
            str = sysconf_addstr(str);
        num = sysconf_key++;
        sysvar[num].key = sysconf_addstr(key);
        sysvar[num].str = str;
        sysvar[num].val = val;
        /*
           sprintf( genbuf, "%s = %s (%x).", key, str, val );
           bbslog("user","%s", genbuf );
         */
    }
}

static void sysconf_addmenu(FILE * fp, char *key)
{
    struct smenuitem *pm;
    char buf[256];
    char *cmd, *arg[5], *ptr;
    int n;

    /*
       bbslog("user","%s", key );
     */
    sysconf_addkey(key, "menu", sysconf_menu);
    while (fgets(buf, sizeof(buf), fp) != NULL && buf[0] != '%') {
        cmd = strtok(buf, " \t\n");
        if (cmd == NULL || *cmd == '#') {
            continue;
        }
        arg[0] = arg[1] = arg[2] = arg[3] = arg[4] = "";
        n = 0;
        for (n = 0; n < 5; n++) {
            if ((ptr = strtok(NULL, ",\n")) == NULL)
                break;
            while (*ptr == ' ' || *ptr == '\t')
                ptr++;
            if (*ptr == '"') {
                arg[n] = ++ptr;
                while (*ptr != '"' && *ptr != '\0')
                    ptr++;
                *ptr = '\0';
            } else {
                arg[n] = ptr;
                while (*ptr != ' ' && *ptr != '\t' && *ptr != '\0')
                    ptr++;
                *ptr = '\0';
            }
        }
        pm = &menuitem[sysconf_menu++];
        pm->line = sysconf_eval(arg[0],strtol(arg[0],NULL,0));        /*菜单项位置 */
        pm->col = sysconf_eval(arg[1],strtol(arg[1],NULL,0));
        if (*cmd == '@') {      /*对应 某功能 */
            pm->level = sysconf_eval(arg[2],strtol(arg[2],NULL,0));
            pm->name = sysconf_addstr(arg[3]);
            pm->desc = sysconf_addstr(arg[4]);
            pm->func_name = sysconf_addstr(cmd + 1);
            pm->arg = pm->name;
        } else if (*cmd == '!') {       /* 对应 下一级菜单 */
            pm->level = sysconf_eval(arg[2],strtol(arg[2],NULL,0));
            pm->name = sysconf_addstr(arg[3]);
            pm->desc = sysconf_addstr(arg[4]);
            pm->func_name = sysconf_addstr("domenu");
            pm->arg = sysconf_addstr(cmd + 1);
        } else {                /* load title or screen */
            pm->level = -2;
            pm->name = sysconf_addstr(cmd);
            pm->desc = sysconf_addstr(arg[2]);
            pm->func_name = sysconf_buf;
            pm->arg = sysconf_buf;
        }
        /*
           sprintf( genbuf, "%s( %s, %s, %s, %s, %s )",
           cmd, arg[0], arg[1], arg[2], arg[3], arg[4] );
           bbslog("user","%s", genbuf );
         */
    }
    pm = &menuitem[sysconf_menu++];
    pm->name = pm->desc = pm->arg = sysconf_buf;
    pm->func_name = sysconf_buf;
    pm->level = -1;
}

static void sysconf_addblock(FILE * fp, char *key)
{                               /* 读入 %和%直接 包含的block */
    char buf[256];
    int num;

    if (sysconf_key < SC_KEYSIZE) {
        num = sysconf_key++;
        sysvar[num].key = sysconf_addstr(key);
        sysvar[num].str = sysconf_buf + sysconf_len;
        sysvar[num].val = -1;
        while (fgets(buf, sizeof(buf), fp) != NULL && buf[0] != '%') {
            encodestr(buf);
            strcpy(sysconf_buf + sysconf_len, buf);
            sysconf_len += strlen(buf);
        }
        sysconf_len++;
    } else {
        while (fgets(buf, sizeof(buf), fp) != NULL && buf[0] != '%') {
        }
    }
}

static void parse_sysconf(const char *fname)
{
    FILE *fp;
    char buf[256];
    char tmp[256], *ptr;
    char *key, *str;
    int val;

    if ((fp = fopen(fname, "r")) == NULL) {
        return;
    }
    sysconf_addstr("(null ptr)");
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        ptr = buf;
        while (*ptr == ' ' || *ptr == '\t')
            ptr++;

        if (*ptr == '%') {
            strtok(ptr, " \t\n");
            if (strcmp(ptr, "%menu" /*菜单 */ ) == 0) {
                str = strtok(NULL, " \t\n");
                if (str != NULL)
                    sysconf_addmenu(fp, str);
            } else {            /*其它，如screen设计 */
                sysconf_addblock(fp, ptr + 1);
            }
        } else if (*ptr == '#') {
            key = strtok(ptr, " \t\"\n");
            str = strtok(NULL, " \t\"\n");
            if (key != NULL && str != NULL && strcmp(key, "#include") == 0) {   /* 用#include filename 来包含其它ini */
                parse_sysconf(str);
            }
        } else if (*ptr != '\n') {      /*系统参量 定义 */
            key = strtok(ptr, "=#\n");
            str = strtok(NULL, "#\n");
            if ((key != NULL) && (str != NULL)) {
                strtok(key, " \t");
                while (*str == ' ' || *str == '\t')
                    str++;
                if (*str == '"') {
                    str++;
                    strtok(str, "\"");
                    val = atoi(str);
                    sysconf_addkey(key, str, val);
                } else {
                    val = 0;
                    strcpy(tmp, str);
                    ptr = strtok(tmp, ", \t");
                    while (ptr != NULL) {
                        val |= sysconf_eval(ptr,strtol(ptr,NULL,0));
                        ptr = strtok(NULL, ", \t");
                    }
                    sysconf_addkey(key, NULL, val);
                }
            } else {
                bbslog("user","%s",ptr);
            }
        }
    }
    fclose(fp);
}

void build_sysconf(const char *configfile,const char *imgfile)
{
    struct smenuitem *old_menuitem;
    struct sdefine *old_sysvar;
    char *old_buf;
    int old_menu, old_key, old_len;
    struct sysheader shead;
    int fh;
    const char *imgfilename;
    struct public_data *p=NULL;
    char buf[255];
    int old_diff;

    old_menuitem = menuitem;
    old_menu = sysconf_menu;
    old_sysvar = sysvar;
    old_key = sysconf_key;
    old_buf = sysconf_buf;
    old_len = sysconf_len;
    old_diff = sysconf_diff;

    menuitem = (void *) malloc(SC_CMDSIZE * sizeof(struct smenuitem));
    sysvar = (void *) malloc(SC_KEYSIZE * sizeof(struct sdefine));
    sysconf_diff = 0;
    sysconf_buf = (void *) malloc(SC_BUFSIZE);
    sysconf_menu = 0;
    sysconf_key = 0;
    sysconf_len = 0;
    parse_sysconf(configfile);

    if (!imgfile) {
        /* rebuid new sysconf.img */
        p = get_publicshm();
        sprintf(buf, "sysconf.img.%d", p->sysconfimg_version + 1);
        imgfilename = buf;
    } else
        imgfilename = imgfile;
    if ((fh = open(imgfilename, O_WRONLY | O_CREAT, 0644)) > 0) {
        ftruncate(fh, 0);
        flock(fh, LOCK_EX);
        shead.buf = sysconf_buf;
        shead.menu = sysconf_menu;
        shead.key = sysconf_key;
        shead.len = sysconf_len;
        write(fh, &shead, sizeof(shead));
        write(fh, menuitem, sysconf_menu * sizeof(struct smenuitem));
        write(fh, sysvar, sysconf_key * sizeof(struct sdefine));
        write(fh, sysconf_buf, sysconf_len);
        flock(fh, LOCK_UN);
        close(fh);
    }
    free(menuitem);
    free(sysvar);
    free(sysconf_buf);
    sysconf_diff = old_diff;
    menuitem = old_menuitem;
    sysconf_menu = old_menu;
    sysvar = old_sysvar;
    sysconf_key = old_key;
    sysconf_buf = old_buf;
    sysconf_len = old_len;
    if (!imgfile) {
        setpublicshmreadonly(0);
        p->sysconfimg_version++;
        setpublicshmreadonly(1);
        /*这样使得所有的后来的进程将使用新的image */
    }
}

int load_sysconf_image(char *imgfile)
{
    struct public_data *p;
    struct sysheader *shead;
    int fd;
    char *ptr;
    struct stat st;

    p = get_publicshm();
    if ((fd = open(imgfile, O_RDONLY, 0)) == -1)
        return -1;
    if (fstat(fd, &st) < 0) {
        close(fd);
        return -1;
    }
    if (sysconf_ptr != NULL)
        munmap(sysconf_ptr, sysconf_size);

    sysconf_ptr = (char *) mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
    sysconf_size = st.st_size;

    ptr = sysconf_ptr;
    close(fd);

    shead = (struct sysheader *) ptr;
    ptr += sizeof(struct sysheader);

    menuitem = (void *) ptr;
    ptr += shead->menu * sizeof(struct smenuitem);
    sysvar = (void *) ptr;
    ptr += shead->key * sizeof(struct sdefine);
    sysconf_buf = (void *) ptr;
    ptr += shead->len;
    sysconf_menu = shead->menu;
    sysconf_key = shead->key;
    sysconf_len = shead->len;

    sysconf_diff = sysconf_buf - shead->buf;
    /*
       for( n = 0; n < sysconf_menu; n++ ) {
       menuitem[n].name += diff;
       menuitem[n].desc += diff;
       menuitem[n].arg  += diff;
       func = (char *) menuitem[n].fptr;
       menuitem[n].fptr = sysconf_funcptr( func + diff );
       }
       for( n = 0; n < sysconf_key; n++ ) {
       sysvar[n].key += diff;
       sysvar[n].str += diff;
       }
       当使用mmap之后，diff应该在每一次自己做,sigh */
    return 0;
}

void load_sysconf()
{
    char buf[60];
    struct public_data *p;

    p = get_publicshm();
    sysconf_version = p->sysconfimg_version;
    sprintf(buf, "sysconf.img.%d", sysconf_version);
    if (dashf("etc/rebuild.sysconf") || !dashf(buf)) {
        /*注   ：这里其实有一个竞争生成img文件的问题,但是....
           先不管 KCN */
        bbslog("3error", "build sysconf.img");
        build_sysconf("etc/sysconf.ini", buf);
    }
    /*    bbslog("user","%s", "load sysconf.img" ); */
    if (load_sysconf_image(buf) != 0) {
        bbslog("3bbs", "can't load sysconf image!");
        exit(-1);
    };
}

int check_sysconf()
{
    struct public_data *p = get_publicshm();

    if (p->sysconfimg_version > sysconf_version) {
        load_sysconf();
        return 1;
    }
    return 0;
}
