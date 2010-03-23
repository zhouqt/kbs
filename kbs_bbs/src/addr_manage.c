#include "bbs.h"
#if HAVE_MYSQL_SMTH == 1
#include "mysql.h"

/**********************************************************
 * mysql结构:
 *
 * mysql> desc addr;
 * +-------------+--------------+------+-----+---------+----------------+
 * | Field       | Type         | Null | Key | Default | Extra          |
 * +-------------+--------------+------+-----+---------+----------------+
 * | id          | int(10)      |      | PRI | NULL    | auto_increment |
 * | userid      | varchar(13)  |      | MUL |         |                |
 * | name        | varchar(15)  |      | MUL |         |                |
 * | bbsid       | varchar(15)  | YES  | MUL | NULL    |                |
 * | school      | varchar(100) | YES  |     | NULL    |                |
 * | zipcode     | varchar(6)   | YES  |     | NULL    |                |
 * | homeaddr    | varchar(100) | YES  |     | NULL    |                |
 * | companyaddr | varchar(100) | YES  |     | NULL    |                |
 * | tel_o       | varchar(20)  | YES  |     | NULL    |                |
 * | tel_h       | varchar(20)  | YES  |     | NULL    |                |
 * | mobile      | varchar(15)  | YES  | MUL | NULL    |                |
 * | email       | varchar(30)  | YES  |     | NULL    |                |
 * | qq          | varchar(10)  | YES  |     | NULL    |                |
 * | birthday    | date         | YES  |     | NULL    |                |
 * | memo        | text         | YES  |     | NULL    |                |
 * | groupname   | varchar(10)  | YES  | MUL | NULL    |                |
 * +-------------+--------------+------+-----+---------+----------------+
 *
 *                                                    stiger
 **********************************************************/

struct addresslist * a_l;
int al_num=0;
//int al_start=0;
char al_dest[15];
int al_order=0;
char al_group[10];
/*
int sm_type=-1;
*/
char al_msgtxt[30];

int add_addresslist(struct addresslist * oldal)
{

    struct addresslist al;
    char ans[110];
    char anss[4];

    bzero(&al,sizeof(al));
    if (oldal) {
        memcpy(&al, oldal, sizeof(al));
    }
    clear();
    move(0,0);
    ans[0]=0;
    prints("                                 增加通讯录条目\n");
    prints("\033[1;31m------------------------------------------------------------------------\033[m\n");

    if (oldal)
        strcpy(ans, oldal->name);
    else
        ans[0]=0;
    getdata(2, 0, "请输入姓名(不可空):", ans, 16, DOECHO, NULL, false);
    if (ans[0] == '\n' || (ans[0] == 0 && oldal == NULL) || ans[0]=='\r' || !isprint2(ans[0])) {
        prints("\n取消...");
        pressanykey();
        return 0;
    }
    if (ans[0] == 0)
        strcpy(al.name, oldal->name);
    else
        strncpy(al.name, ans, 15);
    al.name[14]=0;

    if (oldal)
        strcpy(ans, oldal->bbsid);
    else
        ans[0]=0;
    getdata(3, 0, "请输入bbsid:", ans, 15, DOECHO, NULL, false);
    strncpy(al.bbsid, ans, 15);
    al.bbsid[14]=0;

    if (oldal)
        strcpy(ans, oldal->mobile);
    else
        ans[0]=0;
    getdata(4, 0, "请输入移动电话:", ans, 15, DOECHO, NULL, false);
    strncpy(al.mobile, ans, 15);
    al.mobile[14]=0;

    if (oldal)
        strcpy(ans, oldal->group);
    else
        ans[0]=0;
    getdata(5, 0, "请输入分组名称:", ans, 10, DOECHO, NULL, false);
    strncpy(al.group, ans, 10);
    al.group[9]=0;

    if (oldal)
        strcpy(ans, oldal->school);
    else
        ans[0]=0;
    getdata(6, 0, "请输入学校:", ans, 100, DOECHO, NULL, false);
    strncpy(al.school, ans, 100);
    al.school[99]=0;

    if (oldal)
        strcpy(ans, oldal->zipcode);
    else
        ans[0]=0;
    getdata(7, 0, "请输入邮政编码:", ans, 7, DOECHO, NULL, false);
    strncpy(al.zipcode, ans, 7);
    al.zipcode[6]=0;

    if (oldal)
        strcpy(ans, oldal->homeaddr);
    else
        ans[0]=0;
    getdata(8, 0, "请输入家庭地址:", ans, 100, DOECHO, NULL, false);
    strncpy(al.homeaddr, ans, 100);
    al.homeaddr[99]=0;

    if (oldal)
        strcpy(ans, oldal->companyaddr);
    else
        ans[0]=0;
    getdata(9, 0, "请输入工作地址:", ans, 100, DOECHO, NULL, false);
    strncpy(al.companyaddr, ans, 100);
    al.companyaddr[99]=0;

    if (oldal)
        strcpy(ans, oldal->tel_h);
    else
        ans[0]=0;
    getdata(10, 0, "请输入家庭电话:", ans, 20, DOECHO, NULL, false);
    strncpy(al.tel_h, ans, 20);
    al.tel_h[19]=0;

    if (oldal)
        strcpy(ans, oldal->tel_o);
    else
        ans[0]=0;
    getdata(11, 0, "请输入工作电话:", ans, 20, DOECHO, NULL, false);
    strncpy(al.tel_o, ans, 20);
    al.tel_o[19]=0;

    if (oldal)
        strcpy(ans, oldal->email);
    else
        ans[0]=0;
    getdata(12, 0, "请输入电子邮箱:", ans, 30, DOECHO, NULL, false);
    strncpy(al.email, ans, 30);
    al.email[29]=0;

    if (oldal)
        strcpy(ans, oldal->qq);
    else
        ans[0]=0;
    getdata(13, 0, "请输入qq:", ans, 10, DOECHO, NULL, false);
    strncpy(al.qq, ans, 10);
    al.qq[9]=0;

    if (oldal)
        sprintf(ans,"%d", oldal->birth_year);
    else
        ans[0]=0;
    getdata(14, 0, "请输入生日年份:", ans, 5, DOECHO, NULL, false);
    al.birth_year = atoi(ans);
    if (al.birth_year <= 0)
        al.birth_year = 1;
    if (al.birth_year < 1000)
        al.birth_year += 1000;
    if (al.birth_year > 9000)
        al.birth_year = 9000;

    if (oldal)
        sprintf(ans,"%d", oldal->birth_month);
    else
        ans[0]=0;
    getdata(15, 0, "请输入生日月份:", ans, 3, DOECHO, NULL, false);
    al.birth_month = atoi(ans);
    if (al.birth_month <= 0)
        al.birth_month = 1;
    if (al.birth_month >12)
        al.birth_month = 12;

    if (oldal)
        sprintf(ans,"%d", oldal->birth_day);
    else
        ans[0]=0;
    getdata(16, 0, "请输入生日日期:", ans, 3, DOECHO, NULL, false);
    al.birth_day = atoi(ans);
    if (al.birth_day <= 0)
        al.birth_day = 1;
    if (al.birth_day >31)
        al.birth_day = 31;

    if (oldal)
        strcpy(ans, oldal->memo);
    else
        ans[0]=0;
    move(17,0);
    prints("请输入备注:\n");
    multi_getdata(18, 0, 79, NULL, ans, 100, 4, false, 0);
    ans[99]=0;

    if (oldal)
        getdata(t_lines-1, 0, "确定修改? (Y/N) [Y]:", anss, 3, DOECHO, NULL, true);
    else
        getdata(t_lines-1, 0, "确定增加? (Y/N) [Y]:", anss, 3, DOECHO, NULL, true);

    if (anss[0] != 'N' && anss[0] != 'n')
        return add_sql_al(getCurrentUser()->userid, &al, ans);

    return 0;
}

static int set_al_select(struct _select_def *conf)
{
    clear();
    move(0,0);
    prints("姓名:     %s\n", a_l[conf->pos-conf->page_pos].name);
    prints("bbsid:    %s\n", a_l[conf->pos-conf->page_pos].bbsid);
    prints("学校:     %s\n", a_l[conf->pos-conf->page_pos].school);
    prints("邮政编码: %s\n", a_l[conf->pos-conf->page_pos].zipcode);
    prints("家庭住址: %s\n", a_l[conf->pos-conf->page_pos].homeaddr);
    prints("公司地址: %s\n", a_l[conf->pos-conf->page_pos].companyaddr);
    prints("家庭电话: %s\n", a_l[conf->pos-conf->page_pos].tel_h);
    prints("公司电话: %s\n", a_l[conf->pos-conf->page_pos].tel_o);
    prints("手机:     %s\n", a_l[conf->pos-conf->page_pos].mobile);
    prints("电子信箱: %s\n", a_l[conf->pos-conf->page_pos].email);
    prints("qq号:     %s\n", a_l[conf->pos-conf->page_pos].qq);
    prints("分组:     %s\n", a_l[conf->pos-conf->page_pos].group);
    prints("生日:     %d-%d-%d\n", a_l[conf->pos-conf->page_pos].birth_year,a_l[conf->pos-conf->page_pos].birth_month,a_l[conf->pos-conf->page_pos].birth_day);
    prints("备注:     %s", a_l[conf->pos-conf->page_pos].memo);
    pressanykey();
    return SHOW_REFRESH;
}

static int set_al_show(struct _select_def *conf, int i)
{
    char title[41];
    char *c;

    if (strlen(a_l[i-conf->page_pos].memo) > 40) {
        strncpy(title, a_l[i-conf->page_pos].memo, 37);
        title[37]='.';
        title[38]='.';
        title[39]='.';
    } else {
        strcpy(title, a_l[i-conf->page_pos].memo);
    }
    title[40]=0;

    if ((c = strchr(title, '\n'))!= NULL) *c=0;
    if ((c = strchr(title, '\r'))!= NULL) *c=0;

    prints(" %-3d %-8s %-10s %-10s %-12s %-28s", i, a_l[i-conf->page_pos].name, a_l[i-conf->page_pos].bbsid, a_l[i-conf->page_pos].group, a_l[i-conf->page_pos].mobile, title);
    return SHOW_CONTINUE;
}

static int set_al_prekey(struct _select_def *conf, int *key)
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

static int set_al_refresh(struct _select_def *conf)
{
    clear();
    docmdtitle("[通讯录管理]","退出[\033[1;32mq\033[m] 增加[\033[1;32ma\033[m] 修改[\033[1;32me\033[m] 删除[\033[1;32md\033[m] 选择[\033[1;32ms\033[m] 排序[\033[1;32mTAB\033[m] 导入[\033[1;32mi\033[m] 发送短信[\033[1;32mS\033[m]");
    move(2,0);
    prints("\033[0;1;44m  %-4s %s%-8s %s%-10s%s %-10s%s %-12s %-28s              \033[m","序号",al_order==AL_ORDER_NAME?"\033[1;32m":"",al_order==AL_ORDER_NAME?"{名字}":"名字",al_order==AL_ORDER_NAME?"\033[0;1;44m":(al_order==AL_ORDER_BBSID?"\033[1;32m":""),al_order==AL_ORDER_BBSID?"{id}":"id",al_order==AL_ORDER_BBSID?"\033[0;1;44m":(al_order==AL_ORDER_GROUPNAME?"\033[1;32m":""),al_order==AL_ORDER_GROUPNAME?"{分组}":"分组",al_order==AL_ORDER_GROUPNAME?"\033[0;1;44m":"", "手机","备注");
    update_endline();
    return SHOW_CONTINUE;
}

static int set_al_getdata(struct _select_def *conf,int pos,int len)
{
    int i;

    for (i=0; i<al_num; i++) {
        if (a_l[i].memo) free(a_l[i].memo);
    }
    bzero(a_l, sizeof(struct addresslist) * BBS_PAGESIZE);

    if (conf->item_count - conf->page_pos < BBS_PAGESIZE)
        conf->item_count = count_sql_al(getCurrentUser()->userid, al_dest, al_group, al_msgtxt);

    i = get_sql_al(a_l, getCurrentUser()->userid, al_dest, al_group, conf->page_pos-1, BBS_PAGESIZE,al_order, al_msgtxt);

    if (i <= 0) {

        conf->pos = 1;
        al_dest[0]=0;
        al_group[0]=0;
        //sm_type = -1;
        al_msgtxt[0]=0;

        i = get_sql_al(a_l, getCurrentUser()->userid, al_dest, al_group, 0, BBS_PAGESIZE,al_order, al_msgtxt);

        if (i <= 0)
            return SHOW_QUIT;
    }

    return SHOW_CONTINUE;
}


static int set_al_key(struct _select_def *conf, int key)
{
    switch (key) {
        case 'd': {
            char sql[100];
            MYSQL s;
            char ans[4];

            move(2,0);
            clrtoeol();
            ans[0]=0;
            getdata(2, 0, "删除这条记录(Y/N) [N]: ", ans, 3, DOECHO, NULL, true);
            if (ans[0] != 'y' && ans[0]!='Y')
                return SHOW_REFRESH;

            mysql_init(&s);
            if (! my_connect_mysql(&s)) {
                clear();
                prints("%s\n",mysql_error(&s));
                pressanykey();
                mysql_close(&s);
                return SHOW_REFRESH;
            }

            sprintf(sql,"DELETE FROM addr WHERE id=%d;",a_l[conf->pos-conf->page_pos].id);

            if (mysql_real_query(&s, sql, strlen(sql))) {
                clear();
                prints("%s\n",mysql_error(&s));
                pressanykey();
                mysql_close(&s);
                return SHOW_REFRESH;
            }

            mysql_close(&s);

            return SHOW_DIRCHANGE;
        }
        case 's': {
            char ans[40];

            clear();
            move(0,0);
            ans[0]=0;
            prints("                                 超级通讯录选择\n");
            prints("\033[1;31m------------------------------------------------------------------------\033[m\n");
            getdata(2, 0, "选择全部通讯录请按\033[1;32m1\033[m,输入条件选择请按\033[1;32m2\033[m,取消直接回车(1/2/0) [0]: ", ans, 3, DOECHO, NULL, true);
            if (ans[0] == '1') {
                conf->pos = 0;
                al_dest[0]=0;
                al_group[0]=0;
                //sm_type = -1;
                al_msgtxt[0]=0;
                return SHOW_DIRCHANGE;
            } else if (ans[0] == '2') {
                move(3,0);
                getdata(3,0,"请输入要选择的通讯录姓名(回车选择所有):",ans,15, DOECHO,NULL,true);
                strncpy(al_dest, ans, 15);
                al_dest[14]=0;

                move(4,0);
                getdata(4,0,"请输入要选择的通讯录组名(回车选择所有):",ans,10, DOECHO,NULL,true);
                strncpy(al_group, ans, 10);
                al_group[9]=0;

                move(5,0);
                getdata(5,0,"请输入开始显示的通讯录序号 [0]:",ans,5, DOECHO,NULL,true);
                conf->page_pos = atoi(ans);
                if (conf->page_pos<=0)
                    conf->page_pos=1;
                conf->pos=conf->page_pos;

                move(6,0);
                getdata(6,0,"请输入要通讯录备注内容包含文字(回车选择所有):",ans,21, DOECHO,NULL,true);
                strncpy(al_msgtxt, ans, 21);
                al_msgtxt[20]=0;

                return SHOW_DIRCHANGE;
            } else {
                return SHOW_REFRESH;
            }
            break;
        }
        case 'f': {
            conf->pos = 0;
            al_dest[0]=0;
            al_group[0]=0;
            //sm_type = -1;
            al_msgtxt[0]=0;
            return SHOW_DIRCHANGE;
        }
        case 'a': {
            if (conf->item_count > MAXFRIENDS) {
                clear();
                prints("您的通讯录已经达到最大容量");
                pressreturn();
                return SHOW_REFRESH;
            }
            if (add_addresslist(NULL)) {
                return SHOW_DIRCHANGE;
            }
            return SHOW_REFRESH;
        }
        case 'e': {
            if (add_addresslist(a_l+conf->pos-conf->page_pos)) {
                return SHOW_DIRCHANGE;
            }
            return SHOW_REFRESH;
        }
#ifdef SMS_SUPPORT
        case 'S': {
            if ((a_l+conf->pos - conf->page_pos)->mobile[0] && isdigit((a_l+conf->pos - conf->page_pos)->mobile[0])) {
                do_send_sms_func((a_l+conf->pos - conf->page_pos)->mobile, NULL);
                return SHOW_REFRESH;
            }
            break;
        }
#endif
        case KEY_TAB: {
            al_order ++;
            if (al_order > AL_ORDER_COUNT)
                al_order = 0;
            return SHOW_DIRCHANGE;
        }
        case 'i': {
            char ans[4];

            if (conf->item_count > MAXFRIENDS) {
                clear();
                prints("您的通讯录已经达到最大容量");
                pressreturn();
                return SHOW_REFRESH;
            }

            clear();
            getdata(3, 0, "确实要导入好友名单? (Y/N) [N]: ", ans, 3, DOECHO, NULL, true);
            if (ans[0] != 'y' && ans[0] != 'Y') {
                return SHOW_REFRESH;
            } else {
                char fpath[STRLEN];
                struct friends fh;
                struct addresslist al;
                FILE *fp;
                int suc=0;

                sethomefile(fpath, getCurrentUser()->userid, "friends");

                if ((fp=fopen(fpath, "r"))==NULL)
                    return SHOW_REFRESH;
                while (fread(&fh, sizeof(fh), 1, fp)) {
                    bzero(&al,sizeof(al));
                    if (fh.exp[0])
                        strncpy(al.name, fh.exp, 15);
                    else
                        strncpy(al.name, fh.id, 15);
                    al.name[14]=0;
                    strncpy(al.bbsid, fh.id, 15);
                    al.bbsid[14]=0;
                    strcpy(al.group,"bbs");

                    move(10,0);
                    clrtoeol();
                    prints("正在导入用户 %s......",fh.id);
                    if (add_sql_al(getCurrentUser()->userid, &al, fh.exp)) {
                        suc++;
                        prints("成功");
                    } else
                        prints("失败");
                }
                fclose(fp);

                move(10,0);
                clrtoeol();
                prints("成功导入 %d 个好友", suc);
                pressanykey();

                return SHOW_DIRCHANGE;
            }
        }
        default:
            break;
    }
    return SHOW_CONTINUE;
}

int al_read(void)
{
    struct _select_def group_conf;
    int i;
    POINT *pts;

    al_dest[0]=0;
    al_group[0]=0;

    a_l = (struct addresslist *) malloc(sizeof(struct addresslist) * BBS_PAGESIZE);
    if (a_l == NULL)
        return -1;

    bzero(&group_conf,sizeof(struct _select_def));

    //TODO: 窗口大小动态改变的情况？这里有bug
    pts = (POINT *) malloc(sizeof(POINT) * BBS_PAGESIZE);
    for (i = 0; i < BBS_PAGESIZE; i++) {
        pts[i].x = 2;
        pts[i].y = i + 3;
    }
    group_conf.item_per_page = BBS_PAGESIZE;
    group_conf.flag = LF_VSCROLL | LF_BELL | LF_MULTIPAGE | LF_LOOP;
    group_conf.prompt = "◆";
    group_conf.item_pos = pts;
    group_conf.title_pos.x = 0;
    group_conf.title_pos.y = 0;
    group_conf.pos=1;
    group_conf.page_pos=1;

    group_conf.on_select = set_al_select;
    group_conf.show_data = set_al_show;
    group_conf.pre_key_command = set_al_prekey;
    group_conf.show_title = set_al_refresh;
    group_conf.get_data = set_al_getdata;
    group_conf.key_command = set_al_key;


    bzero(a_l, sizeof(struct addresslist) * BBS_PAGESIZE);
    group_conf.item_count = count_sql_al(getCurrentUser()->userid, al_dest, al_group, NULL);
    i = get_sql_al(a_l, getCurrentUser()->userid, al_dest, al_group, 0, BBS_PAGESIZE, al_order, NULL);

    if (i < 0) {
        free(pts);
        free(a_l);
        return -1;
    }
    if (i == 0) {
        if (!add_addresslist(NULL)) {
            free(pts);
            free(a_l);
            return -1;
        }
        i = get_sql_al(a_l, getCurrentUser()->userid, al_dest, al_group, 0, BBS_PAGESIZE, al_order, NULL);
        group_conf.item_count = 1;
    }

    if (i <= 0) {
        free(pts);
        free(a_l);
        return -1;
    }

    clear();
    list_select_loop(&group_conf);

    for (i=0; i<al_num; i++) {
        if (a_l[i].memo) free(a_l[i].memo);
    }

    free(pts);
    free(a_l);
    a_l = NULL;

    return 1;
}
#endif
