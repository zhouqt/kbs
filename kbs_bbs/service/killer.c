#define BBSMAIN
#include "bbs.h"

#define ROOM_LOCKED 01
#define ROOM_SECRET 02

struct room_struct {
    char name[14];
    char creator[IDLEN+2];
    int style; /* 0 - chat room 1 - killer room */
    unsigned int level;
    int flag;
    int people;
    int maxpeople;
};

#define PEOPLE_SPECTATOR 01
#define PEOPLE_KILLER 02
#define PEOPLE_ALIVE 04
#define PEOPLE_ROOMOP 010

struct people_struct {
    char id[IDLEN+2];
    char nick[NAMELEN];
    int flag;
    int pid;
    int vote;
};

#define INROOM_STOP 1
#define INROOM_NIGHT 2
#define INROOM_DAY 3

struct inroom_struct {
    char title[NAMELEN];
    int status;
    int killernum;
    struct people_struct peoples[100];
};

struct room_struct * rooms;
int * roomst;
struct inroom_struct inrooms;

char msgs[200][80];
int msgst;

void load_msgs()
{
    FILE* fp;
    int i;
    char filename[80], buf[80];
    msgst=0;
    sprintf(filename, "home/%c/%s/.INROOMMSG%d", toupper(currentuser->userid[0]), currentuser->userid, uinfo.pid);
    fp = fopen(filename, "r");
    if(fp) {
        while(!feof(fp)) {
            if(fgets(buf, 79, fp)==NULL) break;
            if(buf[0]) {
                if(msgst==200) {
                    msgst--;
                    for(i=0;i<msgst;i++)
                        strcpy(msgs[i],msgs[i+1]);
                }
                strcpy(msgs[msgst],buf);
                msgst++;
            }
        }
        fclose(fp);
    }
}

void send_msg(struct people_struct * u, char* msg)
{
    FILE* fp;
    int i;
    char filename[80], buf[80];
    sprintf(filename, "home/%c/%s/.INROOMMSG%d", toupper(u->id[0]), u->id, u->pid);
    fp = fopen(filename, "a");
    if(fp) {
        fprintf(fp, "%s\n", msg);
        fclose(fp);
    }
}

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

void load_inroom(struct room_struct * r)
{
    int fd;
    struct flock ldata;
    char filename[80];
    sprintf(filename, "home/%c/%s/.INROOM", toupper(r->creator[0]), r->creator);
    if((fd = open(filename, O_RDONLY, 0644))!=-1) {
        ldata.l_type=F_RDLCK;
        ldata.l_whence=0;
        ldata.l_len=0;
        ldata.l_start=0;
        if(fcntl(fd, F_SETLKW, &ldata)!=-1){
            read(fd, &inrooms, sizeof(struct inroom_struct));
            	
            ldata.l_type = F_UNLCK;
            fcntl(fd, F_SETLKW, &ldata);
        }
        close(fd);
    }
}

void clear_inroom(struct room_struct * r)
{
    char filename[80];
    sprintf(filename, "home/%c/%s/.INROOM", toupper(r->creator[0]), r->creator);
    unlink(filename);
}

void save_inroom(struct room_struct * r)
{
    int fd;
    struct flock ldata;
    char filename[80];
    sprintf(filename, "home/%c/%s/.INROOM", toupper(r->creator[0]), r->creator);
    if((fd = open(filename, O_WRONLY|O_CREAT, 0644))!=-1) {
        ldata.l_type=F_WRLCK;
        ldata.l_whence=0;
        ldata.l_len=0;
        ldata.l_start=0;
        if(fcntl(fd, F_SETLKW, &ldata)!=-1){
            write(fd, &inrooms, sizeof(struct inroom_struct));
            	
            ldata.l_type = F_UNLCK;
            fcntl(fd, F_SETLKW, &ldata);
        }
        close(fd);
    }
}

int change_fd;

void start_change_inroom(struct room_struct * r)
{
    struct flock ldata;
    char filename[80];
    sprintf(filename, "home/%c/%s/.INROOM", toupper(r->creator[0]), r->creator);
    if((change_fd = open(filename, O_RDWR|O_CREAT, 0644))!=-1) {
        ldata.l_type=F_WRLCK;
        ldata.l_whence=0;
        ldata.l_len=0;
        ldata.l_start=0;
        if(fcntl(change_fd, F_SETLKW, &ldata)!=-1){
            lseek(change_fd, 0, SEEK_SET);
            read(change_fd, &inrooms, sizeof(struct inroom_struct));
        }
    }
}

void end_change_inroom()
{
    struct flock ldata;
    ldata.l_whence=0;
    ldata.l_len=0;
    ldata.l_start=0;
    lseek(change_fd, 0, SEEK_SET);
    write(change_fd, &inrooms, sizeof(struct inroom_struct));
    	
    ldata.l_type = F_UNLCK;
    fcntl(change_fd, F_SETLKW, &ldata);
    close(change_fd);
}

int can_see(struct room_struct * r)
{
    if(r->level&currentuser->userlevel!=r->level) return 0;
    if(r->style!=1) return 0;
    if(r->flag&ROOM_SECRET&&!HAS_PERM(currentuser, PERM_SYSOP)) return 0;
    return 1;
}

int can_enter(struct room_struct * r)
{
    if(r->level&currentuser->userlevel!=r->level) return 0;
    if(r->style!=1) return 0;
    if(r->flag&ROOM_LOCKED&&!HAS_PERM(currentuser, PERM_SYSOP)) return 0;
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
    for(i=0;i<*roomst;i++) {
        r2 = rooms+i;
        if(!can_enter(r2)) continue;
        if(!strcmp(r2->name, s))
            return r2;
    }
    return NULL;
}

struct room_struct * myroom;
int selected = 0, ipage=0;

void refreshit()
{
    int i,j,me;
    for(i=0;i<t_lines-1;i++) {
        move(i, 0);
        clrtoeol();
    }
    move(0,0);
    prints("[44;33;1m ·¿¼ä:[36m%-12s[33m»°Ìâ:[36m%-40s[33m×´Ì¬:[36m%6s",
        myroom->name, inrooms.title, (inrooms.status==INROOM_STOP?"Î´¿ªÊ¼":(inrooms.status==INROOM_NIGHT?"ºÚÒ¹ÖÐ":"´ó°×Ìì")));
    clrtoeol();
    resetcolor();
    setfcolor(YELLOW, 1);
    move(1,0);
    prints("¨q¡ª¡ª¡ª¡ª¡ª¡ª¡ª¨r¨q¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¨r");
    move(t_lines-2,0);
    prints("¨t¡ª¡ª¡ª¡ª¡ª¡ª¡ª¨s¨t¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¨s");
    for(i=2;i<=t_lines-3;i++) {
        move(i,0); prints("©¦");
        move(i,16); prints("©¦");
        move(i,18); prints("©¦");
        move(i,78); prints("©¦");
    }
    for(me=0;me<myroom->people;me++)
        if(inrooms.peoples[me].pid == uinfo.pid) break;
    for(i=2;i<=t_lines-3;i++) 
    if(ipage+i-2>=0&&ipage+i-2<myroom->people) {
        j=ipage+i-2;
        if(inrooms.peoples[j].flag&PEOPLE_KILLER && (inrooms.peoples[me].flag&PEOPLE_KILLER ||
            inrooms.peoples[me].flag&PEOPLE_SPECTATOR ||
            !(inrooms.peoples[j].flag&PEOPLE_ALIVE))) {
            resetcolor();
            move(i,2);
            setfcolor(RED, 1);
            prints("*");
        }
        if(!inrooms.peoples[j].flag&PEOPLE_ALIVE) {
            resetcolor();
            move(i,3);
            setfcolor(BLUE, 1);
            prints("X");
        }
        resetcolor();
        move(i,4);
        if(j==selected) {
            setbcolor(BLUE);
        }
        if(inrooms.peoples[j].nick[0])
            prints(inrooms.peoples[j].nick);
        else
            prints(inrooms.peoples[j].id);
    }
    resetcolor();
    for(i=2;i<=t_lines-3;i++) 
    if(msgst-1-(t_lines-3-i)>=0)
    {
        move(i,20);
        prints(msgs[msgst-1-(t_lines-3-i)]);
    }
}

void room_refresh(int signo)
{
    int y,x;
    signal(SIGUSR1, room_refresh);

    load_inroom(myroom);
    load_msgs();
    getyx(&y, &x);
    refreshit();
    move(y, x);
    refresh();
}

void start_game()
{
    int i;
    char buf[80];
    start_change_inroom(myroom);
    inrooms.status = INROOM_NIGHT;
    end_change_inroom();
    sprintf(buf, "\x1b[31mÓÎÏ·¿ªÊ¼À²!\x1b[m\n");
    for(i=0;i<myroom->people;i++) {
        send_msg(inrooms.peoples+i, buf);
        kill(inrooms.peoples[i].pid, SIGUSR1);
    }
}

#define menust 8
int do_com_menu()
{
    char menus[menust][15]=
        {"0-·µ»Ø","1-ÍË³öÓÎÏ·","2-¸ÄÃû×Ö", "3-Íæ¼ÒÁÐ±í", "4-¸Ä»°Ìâ", "5-ÉèÖÃ·¿¼ä", "6-ÌßÍæ¼Ò", "7-¿ªÊ¼ÓÎÏ·"};
    int menupos[menust],i,j,sel=0,ch;
    menupos[0]=0;
    for(i=1;i<menust;i++)
        menupos[i]=menupos[i-1]+strlen(menus[i-1])+1;
    do{
        resetcolor();
        move(t_lines-1,0);
        clrtoeol();
        for(j=0;j<myroom->people;j++)
            if(inrooms.peoples[j].pid == uinfo.pid) break;
        for(i=0;i<menust;i++) 
        if(inrooms.peoples[j].flag&PEOPLE_ROOMOP||i<=3)
        if(i!=7||inrooms.status==INROOM_STOP) {
            resetcolor();
            move(t_lines-1, menupos[i]);
            if(i==sel) {
                setfcolor(RED,1);
            }
            prints(menus[i]);
        }
        ch=igetkey();
        switch(ch){
        case KEY_LEFT:
            sel--;
            if(sel<0) sel=menust-1;
            break;
        case KEY_RIGHT:
            sel++;
            if(sel>=menust) sel=0;
            break;
        case '\n':
        case '\r':
            switch(sel) {
                case 0:
                    return 0;
                case 1:
                    return 1;
                case 7:
                    start_game();
                    return 0;
            }
            break;
        default:
            for(i=0;i<menust;i++)
                if(ch==menus[i][0]) sel=i;
            break;
        }
    }while(1);
}

void join_room(struct room_struct * r)
{
    char buf[80],buf2[80];
    int i,j,killer;
    clear();
    sprintf(buf, "home/%c/%s/.INROOMMSG%d", toupper(r->creator[0]), r->creator, uinfo.pid);
    unlink(buf);
    myroom = r;
    signal(SIGUSR1, room_refresh);
    start_change_inroom(r);
    i=r->people;
    inrooms.peoples[i].flag = 0;
    strcpy(inrooms.peoples[i].id, currentuser->userid);
    inrooms.peoples[i].nick[0]=0;
    inrooms.peoples[i].pid = uinfo.pid;
    if(i==0) {
        inrooms.status = INROOM_STOP;
        strcpy(inrooms.title, "ÎÒÉ±ÎÒÉ±ÎÒÉ±É±É±");
        inrooms.peoples[i].flag = PEOPLE_ROOMOP;
    }
    r->people++;
    end_change_inroom();

    sprintf(buf, "%s½øÈë·¿¼ä", currentuser->userid);
    for(i=0;i<myroom->people;i++) {
        send_msg(inrooms.peoples+i, buf);
        kill(inrooms.peoples[i].pid, SIGUSR1);
    }

    room_refresh(0);
    while(1){
        do{
            getdata(t_lines-1, 0, "ÊäÈë:", buf, 75, 1, NULL, 1);
            if(!buf[0]) {
                if(do_com_menu()) goto quitgame;
            }
        }while(!buf[0]);
        for(i=0;i<myroom->people;i++) {
            send_msg(inrooms.peoples+i, buf);
            kill(inrooms.peoples[i].pid, SIGUSR1);
        }
    }

quitgame:
    killer=0;
    start_change_inroom(r);
    for(i=0;i<myroom->people;i++)
        if(inrooms.peoples[i].pid==uinfo.pid) {
            if(inrooms.peoples[i].flag&PEOPLE_KILLER) killer=1;
            strcpy(buf2, inrooms.peoples[i].nick);
            if(!buf2[0])
                strcpy(buf2, inrooms.peoples[i].id);
            for(j=i;j<myroom->people-1;j++)
                memcpy(inrooms.peoples+j, inrooms.peoples+j+1, sizeof(struct people_struct));
            break;
        }
    r->people--;
    end_change_inroom();

    if(killer)
        sprintf(buf, "É±ÊÖ%sÇ±ÌÓÁË", buf2);
    else
        sprintf(buf, "%sÀë¿ª·¿¼ä", buf2);
    for(i=0;i<myroom->people;i++) {
        send_msg(inrooms.peoples+i, buf);
        kill(inrooms.peoples[i].pid, SIGUSR1);
    }

    signal(SIGUSR1, talk_request);
}

static int room_list_refresh(struct _select_def *conf)
{
    clear();
    docmdtitle("[ÓÎÏ·ÊÒÑ¡µ¥]",
              "  ÍË³ö[\x1b[1;32m¡û\x1b[0;37m,\x1b[1;32me\x1b[0;37m] ½øÈë[\x1b[1;32mEnter\x1b[0;37m] Ñ¡Ôñ[\x1b[1;32m¡ü\x1b[0;37m,\x1b[1;32m¡ý\x1b[0;37m] Ìí¼Ó[\x1b[1;32ma\x1b[0;37m] ¼ÓÈë[\x1b[1;32mJ\x1b[0;37m] \x1b[m");
    move(2, 0);
    prints("[0;1;37;44m    %4s %-40s %-12s %4s %4s", "±àºÅ", "ÓÎÏ·ÊÒÃû³Æ", "´´½¨Õß", "ÈËÊý", "ÀàÐÍ");
    clrtoeol();
    resetcolor();
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
    struct room_struct * r = room_get(conf->pos-1), * r2;
    if((r2=find_room(r->name))==NULL) {
        move(0, 0);
        clrtoeol();
        prints(" ¸Ã·¿¼äÒÑ±»Ëø¶¨!");
        refresh(); sleep(1);
        return SHOW_REFRESH;
    }
    if(r2->people==r2->maxpeople) {
        move(0, 0);
        clrtoeol();
        prints(" ¸Ã·¿¼äÈËÊýÒÑÂú");
        refresh(); sleep(1);
        return SHOW_REFRESH;
    }
    join_room(find_room(r2->name));
    return SHOW_DIRCHANGE;
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

static int room_list_key(struct _select_def *conf, int key)
{
    struct room_struct r, *r2;
    char name[40];
    switch(key) {
    case 'a':
        strcpy(r.creator, currentuser->userid);
        getdata(0, 0, "·¿¼äÃû:", name, 12, 1, NULL, 1);
        if(!name[0]) return SHOW_REFRESH;
        strcpy(r.name, name);
        r.style = 1;
        r.flag = 0;
        r.people = 0;
        r.maxpeople = 100;
        if(add_room(&r)==-1) {
            move(0, 0);
            clrtoeol();
            prints(" ÓÐÒ»ÑùÃû×ÖµÄ·¿¼äÀ²!");
            refresh(); sleep(1);
            return SHOW_REFRESH;
        }
        clear_inroom(&r);
        join_room(find_room(r.name));
        return SHOW_DIRCHANGE;
    case 'J':
        getdata(0, 0, "·¿¼äÃû:", name, 12, 1, NULL, 1);
        if(!name[0]) return SHOW_REFRESH;
        if((r2=find_room(name))==NULL) {
            move(0, 0);
            clrtoeol();
            prints(" Ã»ÓÐÕÒµ½¸Ã·¿¼ä!");
            refresh(); sleep(1);
            return SHOW_REFRESH;
        }
        if(r2->people==r2->maxpeople) {
            move(0, 0);
            clrtoeol();
            prints(" ¸Ã·¿¼äÈËÊýÒÑÂú");
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
    rooms = shm+4;
    roomst = shm;
    if(i) (*roomst) = 0;
    oldmode = uinfo.mode;
    modify_user_mode(KILLER);
    choose_room();
    modify_user_mode(oldmode);
}

