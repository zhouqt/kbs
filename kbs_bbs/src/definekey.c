#include "bbs.h"

extern struct key_struct *keymem;
extern int keymem_total;

#define MAX_KEY_DEFINE 100

int release_key()
{
    if (keymem) {
        free(keymem);
        keymem=NULL;
    }
    keymem_total=0;
    return 0;
}

int load_key(char * fn)
{
    FILE* fp;
    char fname[STRLEN];
    struct stat st;
    release_key();
    if (fn==NULL)
        sethomefile(fname, getCurrentUser()->userid, "definekey");
    else
        strcpy(fname, fn);
    stat(fname, &st);
    fp=fopen(fname, "rb");
    if (fp==NULL) return -1;
    keymem_total = st.st_size/sizeof(struct key_struct);
    keymem = malloc(MAX_KEY_DEFINE*sizeof(struct key_struct));
    fread(keymem, st.st_size, 1, fp);
    fclose(fp);
    return 0;
}

void ask_define()
{
    struct _select_item *sel;
    int j;
#ifdef FREE
    char sname[4][14]={"水木清华", "一塌糊涂", "北大未名", "本站默认"};
    char fname[4][30]={"service/definekey.smth", "service/definekey.ytht", "service/definekey.pku", "service/definekey.free" };
#else
    char sname[4][14]={"一塌糊涂", "北大未名", "南大小百合", "本站默认"};
    char fname[4][30]={"service/definekey.ytht", "service/definekey.pku", "service/definekey.nju", "service/definekey.smth"};
#endif
    clear();
    move(3, 3);
    prints("请选择你习惯的按键模式:");
    sel = (struct _select_item *) malloc(sizeof(struct _select_item) * 5);
    for (j=0; j<4; j++) {
        sel[j].x = 3;
        sel[j].y = 6+j;
        sel[j].hotkey = '0'+j;
        sel[j].type = SIT_SELECT;
        sel[j].data = sname[j];
    }
    j=4;
    sel[j].x = -1;
    sel[j].y = -1;
    sel[j].hotkey = -1;
    sel[j].type = 0;
    sel[j].data = NULL;
    j = simple_select_loop(sel, SIF_NUMBERKEY | SIF_SINGLE | SIF_ESCQUIT, 0, 6, NULL) - 1;
    free(sel);
    if (j>=0&&j<=3)
        load_key(fname[j]);
}

int save_key()
{
    FILE* fp;
    char fname[STRLEN];
    sethomefile(fname, getCurrentUser()->userid, "definekey");
    fp=fopen(fname, "wb");
    if (fp==NULL) return -1;
    fwrite(keymem, keymem_total*sizeof(struct key_struct), 1, fp);
    fclose(fp);
    truncate(fname, keymem_total*sizeof(struct key_struct));
    return 0;
}

int add_key(struct key_struct *s)
{
    if (keymem_total>=MAX_KEY_DEFINE) return -1;
    if (!keymem)
        keymem = malloc(MAX_KEY_DEFINE*sizeof(struct key_struct));
    memcpy(keymem+keymem_total, s, sizeof(struct key_struct));
    keymem_total++;
    return 0;
}

int remove_key(int i)
{
    int j;
    if (keymem_total<=0) return -1;
    for (j=i; j<keymem_total-1; j++)
        memcpy(keymem+j, keymem+j+1, sizeof(struct key_struct));
    keymem_total--;
    return 0;
}

void get_key_name(int key, char* s)
{
    if (key>=1&&key<=26)
        sprintf(s, "Ctrl+%c", key+64);
    else if (key>32&&key<128)
        sprintf(s, "'%c'", key);
    else
        switch (key) {
            case 32:
                strcpy(s, "Space");
                break;
            case KEY_F1:
                strcpy(s, "F1");
                break;
            case KEY_F2:
                strcpy(s, "F2");
                break;
            case KEY_F3:
                strcpy(s, "F3");
                break;
            case KEY_F4:
                strcpy(s, "F4");
                break;
            case KEY_TAB:
                strcpy(s, "Tab");
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

void get_keys_name(struct key_struct* key, char* buf)
{
    int i=0;
    buf[0]=0;
    while (i<10&&key->mapped[i]) {
        get_key_name(key->mapped[i], buf+strlen(buf));
        strcat(buf, " ");
        i++;
    }
    buf[strlen(buf)-1] = 0;
}

void get_modes_name(struct key_struct* key, char* buf)
{
    int i=0;
    buf[0]=0;
    if (key->status[0]==-1) {
        strcpy(buf, "无模式");
        return;
    }
    if (!key->status[0]) {
        strcpy(buf, "全部模式");
        return;
    }
    while (i<10&&key->status[i]) {
        strcat(buf, ModeType(key->status[i]));
        strcat(buf, " ");
        i++;
    }
    buf[strlen(buf)-1] = 0;
    return;
}

int modes[200][2], modest;

static int set_modes_show(struct _select_def *conf, int i)
{
    i--;
    prints("%s%s\x1b[m", modes[i][1]?"*":" ", modes[i][0]?ModeType(modes[i][0]):"全部");
    return SHOW_CONTINUE;
}

static int set_modes_prekey(struct _select_def *conf, int *key)
{
    switch (*key) {
        case 'e':
        case 'q':
        case 'Q':
            return SHOW_QUIT;
    }
    return SHOW_CONTINUE;
}

static int set_modes_key(struct _select_def *conf, int key)
{
    int i;
    switch (key) {
        case ' ':
            i=conf->pos-1;
            if (modes[i][1]) {
                modes[i][1]=0;
                if (modes[i][0]>0)
                    modest--;
            } else if (modest<10||modes[i][0]==0) {
                modes[i][1]=1;
                if (modes[i][0]>0)
                    modest++;
            }
            return SHOW_DIRCHANGE;
    }
    return SHOW_CONTINUE;
}

static int set_modes_refresh(struct _select_def *conf)
{
    clear();
    docmdtitle("[设置自定义键模式]",
               "退出[\x1b[1;32mq\x1b[0;37m,\x1b[1;32me\x1b[0;37m] 移动[\x1b[1;32m↑\x1b[0;37m,\x1b[1;32m↓\x1b[0;37m] 选择[\x1b[1;32m空格\x1b[0;37m]\x1b[m");
    update_endline();
    return SHOW_CONTINUE;
}

static int set_modes_getdata(struct _select_def *conf, int pos, int len)
{
    return SHOW_CONTINUE;
}


int set_modes(int *res)
{
    struct _select_def group_conf;
    POINT *pts;
    char *s;
    int i,j,n;

    n=1; modest=0;
    modes[0][0]=0;
    modes[0][1]=res[0]==0;
    for (i=0; i<200; i++) {
        s=ModeType(i);
        if (s[0]&&!strchr(s,'?')&&!strchr(s,'W')) {
            modes[n][0]=i;
            modes[n][1]=0;
            if (res[0]!=-1)
                for (j=0; j<10; j++) {
                    if (!res[j]) break;
                    if (res[j]==i) {
                        modes[n][1]=1;
                        modest++;
                        break;
                    }
                }
            n++;
        }
    }
    bzero(&group_conf, sizeof(struct _select_def));
    group_conf.item_count = n;

    //TODO: 窗口大小动态改变的情况？这里有bug
    pts = (POINT *) malloc(sizeof(POINT) * n);
    for (i = 0; i < n; i++) {
        pts[i].x = 2+20*(i/BBS_PAGESIZE);
        pts[i].y = i%BBS_PAGESIZE + 3;
    }
    group_conf.item_per_page = n;
    /*
     * 加上 LF_VSCROLL 才能用 LEFT 键退出
     */
    group_conf.flag = LF_BELL | LF_LOOP;
    group_conf.prompt = "◆";
    group_conf.item_pos = pts;
//    group_conf.arg = &arg;
    group_conf.title_pos.x = 0;
    group_conf.title_pos.y = 0;
    group_conf.pos = 1;         /* initialize cursor on the first mailgroup */
    group_conf.page_pos = 1;    /* initialize page to the first one */

    group_conf.show_data = set_modes_show;
    group_conf.pre_key_command = set_modes_prekey;
    group_conf.key_command = set_modes_key;
    group_conf.show_title = set_modes_refresh;
    group_conf.get_data = set_modes_getdata;

    list_select_loop(&group_conf);
    free(pts);
    if (modes[0][1]) res[0]=0;
    else if (modest==0) res[0]=-1;
    else {
        j=0;
        for (i=1; i<n; i++)
            if (modes[i][1]) {
                res[j]=modes[i][0];
                j++;
            }
        if (j<10) res[j]=0;
    }

    return 0;
}

static int set_keydefine_show(struct _select_def *conf, int i)
{
    char buf[120], buf2[20], buf3[200];
    get_key_name(keymem[i-1].key, buf2);
    get_keys_name(keymem+i-1, buf);
    get_modes_name(keymem+i-1, buf3);
    buf2[6]=0;
    buf[36]=0;
    buf3[32]=0;
    prints("%-6s  %-36s  %-32s", buf2, buf, buf3);
    return SHOW_CONTINUE;
}

static int set_keydefine_prekey(struct _select_def *conf, int *key)
{
    switch (*key) {
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

    switch (key) {
        case KEY_TAB:
            ask_define();
            return SHOW_DIRCHANGE;
        case 'a':                  /* add new user */
            if (keymem_total < MAX_KEY_DEFINE) {
                int i,j;
                struct key_struct k;
                char buf[120];

                clear();
                move(1, 0);
                prints("请键入自定义键: ");
                do {
                    i = igetkey();
                    get_key_name(i, buf);
                } while (!buf[0]&&i!=KEY_ESC);
                if (i==KEY_ESC) return SHOW_DIRCHANGE;
                prints("%s\n", buf);
                k.key = i;
                move(2, 0);
                prints("请输入替换序列(最多10个)，按一次ESC结束: ");
                j=0;
                do {
                    do {
                        i = igetkey();
                        get_key_name(i, buf);
                    } while (!buf[0]&&i!=KEY_ESC);
                    if (i==KEY_ESC) break;
                    prints("%s ", buf);
                    k.mapped[j] = i;
                    j++;
                    if (j>=10) break;
                } while (1);
                if (j<10) k.mapped[j]=0;
                if (j==0) return SHOW_DIRCHANGE;

                k.status[0] = -1;
                add_key(&k);

                return SHOW_DIRCHANGE;
            }
            break;
        case 'e':
            set_modes(keymem[conf->pos-1].status);
            return SHOW_DIRCHANGE;
        case 's': {
            int i,j,x,y;
            struct key_struct k;
            char buf[120];
            memmove(&k,keymem+conf->pos-1,sizeof(struct key_struct));
            clear();
            move(1, 0);
            prints("请键入自定义键: ");
            do {
                i = igetkey();
                get_key_name(i, buf);
            } while (!buf[0]&&i!=KEY_ESC);
            if (i==KEY_ESC) return SHOW_DIRCHANGE;
            prints("%s\n", buf);
            k.key = i;
            move(2, 0);
            prints("请输入替换序列(最多10个)，按一次ESC结束: ");
            j=0;
            do {
                do {
                    i = igetkey();
                    get_key_name(i, buf);
                } while (!buf[0]&&i!=KEY_ESC);
                if (i==KEY_ESC) break;
                getyx(&y,&x);
                prints((x+strlen(buf)<t_columns-1)?"%s ":"\n%s ",buf);
                k.mapped[j] = i;
                j++;
                if (j>=10) break;
            } while (1);
            if (j<10) k.mapped[j]=0;
            if (j==0) return SHOW_DIRCHANGE;

            memcpy(keymem+conf->pos-1, &k, sizeof(struct key_struct));

            return SHOW_DIRCHANGE;
        }
        break;
        case 'x': {
            struct key_struct k;
            int i;
            char buf[128];
            memmove(&k,keymem+conf->pos-1,sizeof(struct key_struct));
            clear();
            move(1,0);
            prints("请键入自定义键: ");
            do {
                i=igetkey();
                get_key_name(i,buf);
            } while (!buf[0]&&i!=KEY_ESC);
            if (i==KEY_ESC)
                return SHOW_DIRCHANGE;
            k.key=i;
            prints("%s",buf);
            memmove(keymem+conf->pos-1,&k,sizeof(struct key_struct));
            prints("\n\n\033[1;33m%s\033[0;33m<Enter>\033[m","已更新!");
            WAIT_RETURN;
            return SHOW_DIRCHANGE;
        }
        break;
        case 't': {
            struct key_struct k;
            int i,j,x,y;
            char buf[128];
            memmove(&k,keymem+conf->pos-1,sizeof(struct key_struct));
            clear();
            move(1,0);
            prints("请输入替换序列(最多10个), 按一次ESC结束: ");
            for (j=0; j<10; j++) {
                do {
                    i=igetkey();
                    get_key_name(i,buf);
                } while (!buf[0]&&i!=KEY_ESC);
                if (i==KEY_ESC)
                    break;
                getyx(&y,&x);
                prints((x+strlen(buf)<t_columns-1)?"%s ":"\n%s ",buf);
                k.mapped[j]=i;
            }
            if (!j)
                return SHOW_DIRCHANGE;
            if (j<10)
                k.mapped[j]=0;
            memmove(keymem+conf->pos-1,&k,sizeof(struct key_struct));
            prints("\n\n\033[1;33m%s\033[0;33m<Enter>\033[m","已更新!");
            WAIT_RETURN;
            return SHOW_DIRCHANGE;
        }
        break;
        case 'd':                  /* delete existed user */
            if (keymem_total > 0) {
                char ans[3];

                getdata(t_lines - 1, 0, "确实要删除该定义键吗(Y/N)? [N]: ", ans, sizeof(ans), DOECHO, NULL, true);
                if (ans[0] == 'Y' || ans[0] == 'y') {
                    remove_key(conf->pos-1);
                    if (keymem_total==0) {
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
            if (!HAS_PERM(getCurrentUser(), PERM_PAGE))
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
    docmdtitle("[设置自定义键]","预定义[\x1b[1;32mTab\x1b[0;37m] 添加[\x1b[1;32ma\x1b[0;37m] 删除[\x1b[1;32md\x1b[0;37m]\x1b[m "
               "修改 { 模式[\x1b[1;32me\x1b[0;37m] 键名[\x1b[1;32mx\x1b[0;37m] 序列[\x1b[1;32mt\x1b[0;37m] 键名及序列[\x1b[1;32ms\x1b[0;37m] }");
    move(2, 0);
    prints("\033[0;1;37;44m  %-6s  %-36s  %-32s", "按键", "替换序列", "模式");
    clrtoeol();
    update_endline();
    return SHOW_CONTINUE;
}

static int set_keydefine_getdata(struct _select_def *conf, int pos, int len)
{
    conf->item_count = keymem_total;

    return SHOW_CONTINUE;
}

static int set_keydefine_select(struct _select_def *conf)
{
    char buf[20],buf2[120],buf3[240];
    clear();
    get_key_name(keymem[conf->pos-1].key,buf);
    get_keys_name(keymem+conf->pos-1,buf2);
    get_modes_name(keymem+conf->pos-1,buf3);
    move(1,0);
    prints("自定义键: %s\n\n", buf);
    prints("替换序列: %s\n\n", buf2);
    prints("允许模式: %s\n\n", buf3);
    pressanykey();

    return SHOW_REFRESH;
}

extern int skip_key;

int define_key(void)
{
    struct _select_def group_conf;
    struct key_struct key;
    POINT *pts;
    int i;

    skip_key = 1;

    if (keymem_total==0) {
        key.key = 'A';
        key.mapped[0] = 'A';
        key.mapped[1] = 0;
        key.status[0] = 0;
        add_key(&key);
        ask_define();
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
     * 加上 LF_VSCROLL 才能用 LEFT 键退出
     */
    group_conf.flag = LF_VSCROLL | LF_BELL | LF_LOOP | LF_MULTIPAGE;
    group_conf.prompt = "◆";
    group_conf.item_pos = pts;
//    group_conf.arg = &arg;
    group_conf.title_pos.x = 0;
    group_conf.title_pos.y = 0;
    group_conf.pos = 1;         /* initialize cursor on the first mailgroup */
    group_conf.page_pos = 1;    /* initialize page to the first one */

    group_conf.on_select = set_keydefine_select;
    group_conf.show_data = set_keydefine_show;
    group_conf.pre_key_command = set_keydefine_prekey;
    group_conf.key_command = set_keydefine_key;
    group_conf.show_title = set_keydefine_refresh;
    group_conf.get_data = set_keydefine_getdata;

    list_select_loop(&group_conf);
    save_key();
    free(pts);
    skip_key = 0;

    return 0;
}

