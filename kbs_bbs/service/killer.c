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
};

struct room_struct * rooms;
int * roomst;

int add_room(struct room_struct * r)
{
    int i;
    for(i=0;i<*roomst;i++)
    if(!strcmp(rooms[i].name, r->name))
        return -1;
    memcpy(&(rooms[*roomst]), r, sizeof(struct room_struct));
    (*roomst)++;
    return 0;
}

int del_room(struct room_struct * r)
{
    int i, j;
    for(i=0;i<*roomst;i++)
    if(!strcmp(rooms[i].name, r->name)) {
        (*roomst)--;
        for(j=i;j<*roomst;j++)
            memcpy(&(rooms[i]), &(rooms[i+1]), sizeof(struct room_struct));
        break;
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
    for(i=0;i<*roomst;i++)
        if(can_see(rooms+i)) j++;
    return j;
}

struct room_struct * room_get(int w)
{
    int i,j=0;
    for(i=0;i<*roomst;i++) {
        if(can_see(rooms+i)) {
            if(w==j) return rooms+i;
            j++;
        }
    }
    return NULL;
}

struct room_struct * find_room(char * s)
{
    int i;
    struct room_struct * r2;
    for(i=0;i<room_count();i++) {
        r2 = room_get(i);
        if(!strcmp(r2->name, s)) {
            return r2;
        }
    }
    return NULL;
}

static int room_list_refresh(struct _select_def *conf)
{
    clear();
    docmdtitle("[ÓÎÏ·ÊÒÑ¡µ¥]",
              "  ÍË³ö[\x1b[1;32m¡û\x1b[0;37m,\x1b[1;32me\x1b[0;37m] ½øÈë[\x1b[1;32mEnter\x1b[0;37m] Ñ¡Ôñ[\x1b[1;32m¡ü\x1b[0;37m,\x1b[1;32m¡ý\x1b[0;37m] Ìí¼Ó[\x1b[1;32ma\x1b[0;37m] ¼ÓÈë[\x1b[1;32mJ\x1b[0;37m] \x1b[m");
    move(2, 0);
    prints("[0;1;37;44m    %4s %-40s %-12s %4s %4s[m", "±àºÅ", "ÓÎÏ·ÊÒÃû³Æ", "´´½¨Õß", "ÈËÊý", "ÀàÐÍ");
    clrtoeol();
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

void join_room(struct room_struct * r)
{
    r->people++;

    r->people--;
}

static int room_list_key(struct _select_def *conf, int key)
{
    struct room_struct r;
    char name[40];
    switch(key) {
    case 'a':
        strcpy(r.creator, currentuser->userid);
        getdata(0, 0, "·¿¼äÃû:", name, 38, 1, NULL, 1);
        if(!name[0]) return SHOW_REFRESH;
        strcpy(r.name, name);
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
        join_room(find_room(r.name));
        return SHOW_DIRCHANGE;
    case 'J':
        getdata(0, 0, "·¿¼äÃû:", name, 38, 1, NULL, 1);
        if(!name[0]) return SHOW_REFRESH;
        if(find_room(name)==-1) {
            move(0, 0);
            clrtoeol();
            prints(" Ã»ÓÐÕÒµ½¸Ã·¿¼ä!");
            refresh(); sleep(1);
            return SHOW_REFRESH;
        }
        join_room(find_room(name));
        return SHOW_DIRCHANGE;
    }
    return SHOW_CONTINUE;
}

int choose_room()
{
    struct _select_def grouplist_conf;
    int i;
    POINT *pts;

    bzero(&grouplist_conf, sizeof(struct _select_def));
    grouplist_conf.item_count = room_count();
    prints("0"); refresh(); sleep(10);
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
    void * shm;
    shm=attach_shm("KILLER_SHMKEY", 3451, sizeof(struct room_struct)*1000+4, &i);
    prints("%d",i); refresh(); sleep(10);
    rooms = shm+4;
    roomst = shm;
    if(i) roomst = 0;
    oldmode = uinfo.mode;
    modify_user_mode(KILLER);
    choose_room();
    modify_user_mode(oldmode);
}

