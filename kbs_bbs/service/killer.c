#define BBSMAIN
#include "bbs.h"

#define ROOM_LOCKED 01
#define ROOM_SECRET 02

struct room_struct {
    char name[NAMELEN];
    char creator[IDLEN+2];
    int style; /* 0 - chat room 1 - killer room */
    unsigned int level;
    int flag;
    int people;
}

struct room_struct * grooms;
int groomst=0;

void load_grooms()
{
    int fd;
    struct flock ldata;
    if(groomst) free(grooms);
    groomst=0;
    if((fd = open(".ROOMS", O_RDONLY, 0644))!=-1) {
        ldata.l_type=F_RDLCK;
        ldata.l_whence=0;
        ldata.l_len=0;
        ldata.l_start=0;
        if(fcntl(fd, F_SETLKW, &ldata)!=-1){
            read(fd, &groomst, sizeof(groomst));
            grooms=(struct room_struct*)malloc(sizeof(struct room_struct)*(groomst+1));
            read(fd, grooms, sizeof(struct room_struct)*groomst);
            	
            ldata.l_type = F_UNLCK;
            fcntl(fd, F_SETLKW, &ldata);
        }
        close(fd);
    }
}

void save_grooms()
{
    int fd;
    struct flock ldata;
    if((fd = open(".ROOMS", O_WRONLY|O_CREAT, 0644))!=-1) {
        ldata.l_type=F_WRLCK;
        ldata.l_whence=0;
        ldata.l_len=0;
        ldata.l_start=0;
        if(fcntl(fd, F_SETLKW, &ldata)!=-1){
            write(fd, &groomst, sizeof(groomst));
            write(fd, grooms, sizeof(struct room_struct)*groomst);
            	
            ldata.l_type = F_UNLCK;
            fcntl(fd, F_SETLKW, &ldata);
        }
        close(fd);
    }
}

int add_room(struct room_struct * r)
{
    int fd, i;
    struct flock ldata;
    if(groomst) free(grooms);
    groomst=0;
    if((fd = open(".ROOMS", O_RDWR|O_CREAT, 0644))!=-1) {
        ldata.l_type=F_WRLCK;
        ldata.l_whence=0;
        ldata.l_len=0;
        ldata.l_start=0;
        if(fcntl(fd, F_SETLKW, &ldata)!=-1){
            ldata.l_type = F_UNLCK;
            lseek(fd, 0, SEEK_SET);
            read(fd, &groomst, sizeof(groomst));
            grooms=(struct room_struct*)malloc(sizeof(struct room_struct)*(groomst+1));
            read(fd, grooms, sizeof(struct room_struct)*groomst);
            for(i=0;i<groomst;i++)
            if(!strcmp(grooms[i].name, r->name)) {
                fcntl(fd, F_SETLKW, &ldata);
                close(fd);
                return -1;
            }
            memcpy(&(grooms[groomst]), r, sizeof(struct room_struct));
            groomst++;
            lseek(fd, 0, SEEK_SET);
            write(fd, &groomst, sizeof(groomst));
            write(fd, grooms, sizeof(struct room_struct)*groomst);
            
            fcntl(fd, F_SETLKW, &ldata);
        }
        close(fd);
    }
    return 0;
}

int del_room(struct room_struct * r)
{
    int fd, i, j;
    struct flock ldata;
    if(groomst) free(grooms);
    groomst=0;
    if((fd = open(".ROOMS", O_RDWR|O_CREAT, 0644))!=-1) {
        ldata.l_type=F_WRLCK;
        ldata.l_whence=0;
        ldata.l_len=0;
        ldata.l_start=0;
        if(fcntl(fd, F_SETLKW, &ldata)!=-1){
            ldata.l_type = F_UNLCK;
            lseek(fd, 0, SEEK_SET);
            read(fd, &groomst, sizeof(groomst));
            grooms=(struct room_struct*)malloc(sizeof(struct room_struct)*(groomst+1));
            read(fd, grooms, sizeof(struct room_struct)*groomst);
            for(i=0;i<groomst;i++)
            if(!strcmp(grooms[i].name, r->name)) {
                groomst--;
                for(j=i;j<groomst;j++)
                    memcpy(&(grooms[i]), &(grooms[i+1]), sizeof(struct room_struct));
                break;
            }
            lseek(fd, 0, SEEK_SET);
            write(fd, &groomst, sizeof(groomst));
            write(fd, grooms, sizeof(struct room_struct)*groomst);
            
            fcntl(fd, F_SETLKW, &ldata);
        }
        close(fd);
    }
    return 0;
}

int can_see(struct room_struct * r)
{
    if(r->level&currentuser->userlevel!=r->level) return 0;
    if(r->style!=1) return 0;
    if(r->flag&ROOM_SECRET&&!HAS_PERM(currentuser, PERM_SYSOP)) return 0;
    return 1;
}

int room_count()
{
    int i,j=0;
    for(i=0;i<groomst;i++)
        if(can_see(grooms+i)) j++;
    return j;
}

struct room_struct * room_get(int w)
{
    int i,j=0;
    for(i=0;i<groomst;i++) {
        if(can_see(grooms+i)) {
            if(w==j) return grooms+i;
            j++;
        }
    }
    return NULL;
}

static int room_list_refresh(struct _select_def *conf)
{
    clear();
    docmdtitle("[ÓÎÏ·ÊÒÑ¡µ¥]",
              "ÍË³ö[\x1b[1;32m¡û\x1b[0;37m,\x1b[1;32me\x1b[0;37m] ½øÈë[\x1b[1;32mEnter\x1b[0;37m] Ñ¡Ôñ[\x1b[1;32m¡ü\x1b[0;37m,\x1b[1;32m¡ý\x1b[0;37m] Ìí¼Ó[\x1b[1;32ma\x1b[0;37m] \x1b[m");
    move(2, 0);
    prints("[0;1;37;44m  %4s %-40s %-12s %4s %4s[m", "±àºÅ", "ÓÎÏ·ÊÒÃû³Æ", "´´½¨Õß", "ÈËÊý", "ÀàÐÍ");
    update_endline();
    return SHOW_CONTINUE;
}

static int room_list_show(struct _select_def *conf, int i)
{
    struct room_struct * r = room_get(i-1);
    if(r)
        prints("  %3d  %-40s %-12s %3d  %4s", i, r->name, r->creator, r->people, "É±ÈË");
    return SHOW_CONTINUE;
}

static int room_list_select(struct _select_def *conf)
{

    return SHOW_REFRESH;
}

static int room_list_getdata(struct _select_def *conf, int pos, int len)
{
    load_grooms();
    conf->item_count = room_count();
    return SHOW_CONTINUE;
}

static int room_list_prekey(struct _select_def *conf, int *key)
{
    switch (*key) {
    case 'e':
    case 'q':
        *key = KEY_LEFT;
        break;
    case 'p':
    case 'k':
        *key = KEY_UP;
        break;
    case ' ':
    case 'N':
        *key = KEY_PGDN;
        break;
    case 'n':
    case 'j':
        *key = KEY_DOWN;
        break;
    }
    return SHOW_CONTINUE;
}

static int room_list_key(struct _select_def *conf, int key)
{
    struct room_struct r;
    switch(key) {
    case 'a':
        strcpy(r.creator, currentuser->userid);
        getdata(0, 0, "·¿¼äÃû:", r.name, 38, 1, NULL, 1);
        if(!r.name[0]) return SHOW_REFRESH;
        r.style = 1;
        r.flag = 0;
        r.people = 0;
        if(add_room(&r)==-1) {
            move(0, 0);
            clrtoeol();
            prints(" ÓÐÒ»ÑùÃû×ÖµÄ·¿¼äÀ²!");
            refresh(); sleep(1);
            return SHOW_REFRESH;
        }
        return SHOW_DIRCHANGE;
    case 'J':
        break;
    }
    return SHOW_CONTINUE;
}

int choose_room()
{
    struct _select_def grouplist_conf;
    int i;
    POINT *pts;

    load_grooms();
    bzero(&grouplist_conf, sizeof(struct _select_def));
    grouplist_conf.item_count = room_count();
    if (grouplist_conf.item_count == 0) {
        grouplist_conf.item_count = 1;
    }
    pts = (POINT *) malloc(sizeof(POINT) * BBS_PAGESIZE);
    for (i = 0; i < BBS_PAGESIZE; i++) {
        pts[i].x = 2;
        pts[i].y = i + 3;
    }
    grouplist_conf.item_per_page = BBS_PAGESIZE;
    grouplist_conf.flag = LF_VSCROLL | LF_BELL | LF_LOOP | LF_MULTIPAGE;
    grouplist_conf.prompt = "¡ô";
    grouplist_conf.item_pos = pts;
    grouplist_conf.arg = NULL;
    grouplist_conf.title_pos.x = 0;
    grouplist_conf.title_pos.y = 0;
    grouplist_conf.pos = 1;     /* initialize cursor on the first mailgroup */
    grouplist_conf.page_pos = 1;        /* initialize page to the first one */

    grouplist_conf.on_select = room_list_select;
    grouplist_conf.show_data = room_list_show;
    grouplist_conf.pre_key_command = room_list_prekey;
    grouplist_conf.key_command = room_list_key;
    grouplist_conf.show_title = room_list_refresh;
    grouplist_conf.get_data = room_list_getdata;
    list_select_loop(&grouplist_conf);
    free(pts);
}

int killer_main()
{
    int i,oldmode;
    oldmode = uinfo.mode;
    modify_user_mode(KILLER);
    choose_room();
    modify_user_mode(oldmode);
}

