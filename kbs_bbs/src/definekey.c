#include "bbs.h"

extern struct key_struct *keymem;
extern int keymem_total;
int tmpnum = 0;

#define MAX_KEY_DEFINE 100

int release_key()
{
    if(keymem) {
        free(keymem);
        keymem=NULL;
    }
    keymem_total=0;
    return 0;
}

int load_key()
{
    FILE* fp;
    char fname[STRLEN];
    struct stat st;
    release_key();
    sethomefile(fname, currentuser->userid, "definekey");
    stat(fname, &st);
    fp=fopen(fname, "rb");
    if(fp==NULL) return -1;
    keymem_total = st.st_size/sizeof(struct key_struct);
    keymem = malloc(MAX_KEY_DEFINE*sizeof(struct key_struct));
    fread(keymem, st.st_size, 1, fp);
    fclose(fp);
    return 0;
}

int save_key()
{
    FILE* fp;
    char fname[STRLEN];
    struct stat st;
    sethomefile(fname, currentuser->userid, "definekey");
    fp=fopen(fname, "wb");
    if(fp==NULL) return -1;
    fwrite(keymem, keymem_total*sizeof(struct key_struct), 1, fp);
    fclose(fp);
    truncate(fname, keymem_total*sizeof(struct key_struct));
    return 0;
}

int add_key(struct key_struct *s)
{
    if(keymem_total>=MAX_KEY_DEFINE) return -1;
    if(!keymem)
        keymem = malloc(MAX_KEY_DEFINE*sizeof(struct key_struct));
    memcpy(keymem+keymem_total*sizeof(struct key_struct), s, sizeof(struct key_struct));
    keymem_total++;
    return 0;
}

int remove_key(int i)
{
    int j;
    if(keymem_total<=0) return -1;
    for(j=i;j<keymem_total-1;j++)
        memcpy(keymem+j*sizeof(struct key_struct), keymem+(j+1)*sizeof(struct key_struct), sizeof(struct key_struct));
    keymem_total--;
    return 0;
}

int get_key_name(int key, char* s)
{
    if(key>=1&&key<=26)
        sprintf(s, "Ctrl+%c", key+64);
    else if(key>32&&key<128)
        sprintf(s, "'%c'", key);
    else
    switch(key) {
        case 32:
            strcpy(s, "Space");
            break;
        case KEY_TAB:
            strcpy(s, "Tab");
            break;
            break;
        case KEY_LEFT:
            strcpy(s, "Left");
            break;
        case KEY_RIGHT:
            strcpy(s, "Right");
            break;
        case KEY_UP:
            strcpy(s, "Up");
            break;
        case KEY_DOWN:
            strcpy(s, "Down");
            break;
        case KEY_HOME:
            strcpy(s, "Home");
            break;
        case KEY_INS:
            strcpy(s, "Insert");
            break;
        case KEY_DEL:
            strcpy(s, "Delete");
            break;
        case KEY_END:
            strcpy(s, "End");
            break;
        case KEY_PGUP:
            strcpy(s, "PageUp");
            break;
        case KEY_PGDN:
            strcpy(s, "PageDn");
            break;
        default:
            s[0]=0;
            break;
    }
}

int get_keys_name(struct key_struct* key, char* buf)
{
    int i=0;
    buf[0]=0;
    while(i<10&&key->mapped[i]) {
        get_key_name(key->mapped[i], buf+strlen(buf));
        strcat(buf, " ");
        i++;
    }
    buf[strlen(buf)-1] = 0;
}

static int set_keydefine_show(struct _select_def *conf, int i)
{
    char buf[120], buf2[20];
    get_key_name(keymem[i].key, buf2);
    get_keys_name(keymem+i, buf);
    prints(" %6s  %-36s  %-32s", buf2, buf, "");
    return SHOW_CONTINUE;
}

static int set_keydefine_prekey(struct _select_def *conf, int *key)
{
    if ((*key == '\r' || *key == '\n') && (tmpnum != 0)) {
        conf->new_pos = tmpnum;
        tmpnum = 0;
        return SHOW_SELCHANGE;
    }

    if (!isdigit(*key))
        tmpnum = 0;

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

static int set_keydefine_key(struct _select_def *conf, int key)
{
    int oldmode;

    if (key >= '0' && key <= '9') {
        tmpnum = tmpnum * 10 + (key - '0');
        return SHOW_CONTINUE;
    }
    switch (key) {
    case 'a':                  /* add new user */
        if (keymem_total < MAX_KEY_DEFINE) {
            int i,j;
            struct key_struct k;
            char buf[120];

            clear();
            move(1, 0);
            prints("Çë¼üÈëÒªÔö¼ÓµÄ×Ô¶¨Òå¼ü: ");
            do {
                i = igetkey();
                get_key_name(i, buf);
            }while(buf[0]||i==KEY_ESC);
            if(i==KEY_ESC) break;
            prints("%s\n", buf);
            k.key = i;
            move(2, 0);
            prints("ÇëÊäÈëÌæ»»ÐòÁÐ(×î¶à10¸ö)£¬°´Á½´ÎESC½áÊø: ");
            do{
                j=0;
                do {
                    i = igetkey();
                    get_key_name(i, buf);
                }while(buf[0]||i==KEY_ESC);
                if(i==KEY_ESC) break;
                prints("%s ", buf);
                k.mapped[j] = i;
                j++;
                if(j>=10) break;
            }while(1);
            if(j==0) break;

            k.status[0] = 0;
            add_key(&k);
            
            return SHOW_DIRCHANGE;
        }
        break;
    case 'd':                  /* delete existed user */
        if (keymem_total > 0) {
            char ans[3];

            getdata(t_lines - 1, 0, "È·ÊµÒªÉ¾³ý¸Ã¶¨Òå¼üÂð(Y/N)? [N]: ", ans, sizeof(ans), DOECHO, NULL, true);
            if (ans[0] == 'Y' || ans[0] == 'y') {
                remove_key(conf->pos-1);
                if(keymem_total==0) {
                    struct key_struct key;
                    key.key = 'A';
                    key.mapped[0] = 'A';
                    key.mapped[1] = 0;
                    key.status[0] = 0;
                    add_key(&key);
                }
            }
            return SHOW_DIRCHANGE;
        }
        break;
    case 'L':
    case 'l':
        oldmode = uinfo.mode;
        show_allmsgs();
        modify_user_mode(oldmode);
        return SHOW_REFRESH;
    case 'W':
    case 'w':
        oldmode = uinfo.mode;
        if (!HAS_PERM(currentuser, PERM_PAGE))
            break;
        s_msg();
        modify_user_mode(oldmode);
        return SHOW_REFRESH;
    case 'u':
        oldmode = uinfo.mode;
        clear();
        modify_user_mode(QUERY);
        t_query(NULL);
        modify_user_mode(oldmode);
        clear();
        return SHOW_REFRESH;
    }

    return SHOW_CONTINUE;
}

static int set_keydefine_refresh(struct _select_def *conf)
{
    clear();
    docmdtitle("[ÉèÖÃ×Ô¶¨Òå¼ü]",
               "ÍË³ö[\x1b[1;32m¡û\x1b[0;37m,\x1b[1;32me\x1b[0;37m] Ñ¡Ôñ[\x1b[1;32m¡ü\x1b[0;37m,\x1b[1;32m¡ý\x1b[0;37m] Ìí¼Ó[\x1b[1;32ma\x1b[0;37m] ÐÞ¸ÄÄ£Ê½[\x1b[1;32me\x1b[0;37m] ÐÞ¸Ä¼ü¶¨Òå[\x1b[1;32ms\x1b[0;37m] É¾³ý[\x1b[1;32md\x1b[0;37m]\x1b[m");
    move(2, 0);
    prints("[0;1;37;44m  %-6s  %-36s  %-32s[m", "°´¼ü", "Ìæ»»ÐòÁÐ", "Ä£Ê½");
    update_endline();
    return SHOW_CONTINUE;
}

static int set_keydefine_getdata(struct _select_def *conf, int pos, int len)
{
    conf->item_count = keymem_total;

    return SHOW_CONTINUE;
}


int define_key()
{
    struct _select_def group_conf;
    struct key_struct key;
    POINT *pts;
    int i;

    if(keymem_total==0) {
        key.key = 'A';
        key.mapped[0] = 'A';
        key.mapped[1] = 0;
        key.status[0] = 0;
        add_key(&key);
    }
    
    bzero(&group_conf, sizeof(struct _select_def));
    group_conf.item_count = keymem_total;

    pts = (POINT *) malloc(sizeof(POINT) * BBS_PAGESIZE);
    for (i = 0; i < BBS_PAGESIZE; i++) {
        pts[i].x = 2;
        pts[i].y = i + 3;
    }
    group_conf.item_per_page = BBS_PAGESIZE;
    /*
     * ¼ÓÉÏ LF_VSCROLL ²ÅÄÜÓÃ LEFT ¼üÍË³ö 
     */
    group_conf.flag = LF_VSCROLL | LF_BELL | LF_LOOP | LF_MULTIPAGE;
    group_conf.prompt = "¡ô";
    group_conf.item_pos = pts;
//    group_conf.arg = &arg;
    group_conf.title_pos.x = 0;
    group_conf.title_pos.y = 0;
    group_conf.pos = 1;         /* initialize cursor on the first mailgroup */
    group_conf.page_pos = 1;    /* initialize page to the first one */

    group_conf.show_data = set_keydefine_show;
    group_conf.pre_key_command = set_keydefine_prekey;
    group_conf.key_command = set_keydefine_key;
    group_conf.show_title = set_keydefine_refresh;
    group_conf.get_data = set_keydefine_getdata;

    list_select_loop(&group_conf);
    save_key();
    free(pts);

    return 0;
}

