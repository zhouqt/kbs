#include "bbs.h"

#ifdef PERSONAL_CORP
#include "read.h"
//#include "mysql.h"

struct pc_users *pc_u = NULL;

/*******
 * pc_dirmode: 
 *   1: 公开区
 *   2: 好友
 *   3: 私人
 *   4: 收藏
 *   5: 删除
 */
int pc_dirmode = 0;

//收藏夹当前路径,0表示未进入收藏夹
unsigned long pc_fav_dir = 0;

static int pc_sec();
static int pc_selusr(char prefix);
static int pc_read_dir(int first);
static int pc_read_comment();


int pc_choose_user(void){
    pc_sec();
    return 0;
}

static int pc_sel_user()
{
    char ans[20];

    clear();
    getdata(5, 0, "你要查看谁的个人文集? [回车查看自己的个人文集]:", ans, 20, DOECHO, NULL, true);
    if (ans[0] == 0 || ans[0] == '\n' || ans[0] == '\r') {
        pc_read(getCurrentUser()->userid);
    } else
        pc_read(ans);

    return 0;

}

int pc_add_user(void){
    char ans[201];
    char sql[100];
    struct userec *lookupuser;
    struct pc_users pu;

    clear();
    move(0, 0);
    prints("[个人文集管理]");
    move(1, 0);
    prints("请输入待处理用户id:");
    usercomplete(NULL, ans);

    if (ans[0] == 0 || ans[0] == '\n' || ans[0] == '\r')
        return 0;

    if (getuser(ans, &lookupuser) == 0) {
        move(7, 0);
        prints("此用户不存在");
        pressanykey();
        return 0;
    }

    bzero(&pu, sizeof(pu));

    if (get_pc_users(&pu, lookupuser->userid)) {
        getdata(4, 0, "该用户已经拥有个人文集, [1]修改 [2]删除 [0] 返回, [0]:", ans, 3, DOECHO, NULL, true);
        if (ans[0] == '2') {
            move(6, 0);
            prints("\033[1;31m删除个人文集操作将丢失该用户所有个人文集，不可恢复\033[m");
            getdata(7, 0, "你确定要删除吗? (Y/N) [N]:", ans, 3, DOECHO, NULL, true);
            if (ans[0] != 'y' && ans[0] != 'Y')
                return 0;

            if (del_pc_users(&pu)) {
                char buf1[256];

                lookupuser->flags &= ~PCORP_FLAG;
                move(10, 0);
                prints("删除成功");
                pressanykey();
                sprintf(buf1, "%s 取消 %s 的个人文集", getCurrentUser()->userid, lookupuser->userid);
                securityreport(buf1, lookupuser, NULL, getSession());
                return 1;
            } else {
                move(10, 0);
                prints("删除失败");
                pressanykey();
                return 0;
            }

        } else if (ans[0] != '1')
            return 0;
    } else {
        if (!HAS_PERM(lookupuser, PERM_BASIC)) {
            move(7, 0);
            prints("此用户尚无基本权限，无法开启个人文集");
            pressanykey();
            return 0;
        }
        move(4, 0);
        prints("该用户尚没有个人文集,添加进行中,不输入个人文集名称自动取消");
    }

    strncpy(pu.username, lookupuser->userid, IDLEN + 2);
    pu.username[IDLEN + 1] = 0;

    if (pu.uid) {
        strncpy(ans, pu.corpusname, 40);
        ans[40] = 0;
    } else
        ans[0] = 0;
    getdata(5, 0, "请输入个人文集名称:", ans, 40, DOECHO, NULL, false);
    if (ans[0]) {
        strncpy(pu.corpusname, ans, 40);
        pu.corpusname[40] = 0;
    } else {
        if (!pu.uid)
            return 0;
    }

    if (pu.uid) {
        strncpy(ans, pu.description, 200);
        ans[200] = 0;
    } else
        ans[0] = 0;
    move(6, 0);
    prints("请输入个人文集描述:");
    multi_getdata(7, 0, 79, NULL, ans, 200, 4, false, 0);
    if (ans[0]) {
        strncpy(pu.description, ans, 200);
        pu.description[200] = 0;
    }

    if (pu.uid) {
        strncpy(ans, pu.theme, 10);
        ans[10] = 0;
    } else
        ans[0] = 0;
    getdata(12, 0, "请输入主题:", ans, 10, DOECHO, NULL, false);
    if (ans[0]) {
        strncpy(pu.theme, ans, 10);
        pu.theme[10] = 0;
    }

    if (pu.uid)
        sprintf(ans, "%d", pu.nodelimit);
    else
        sprintf(ans, "%d", PC_DEFAULT_NODELIMIT);
    sprintf(sql, "个人文集允许最多文章数 [%s]:", ans);
    getdata(13, 0, sql, ans, 5, DOECHO, NULL, false);
    if (ans[0]) {
        pu.nodelimit = atoi(ans);
        if (pu.nodelimit <= 0)
            pu.nodelimit = PC_DEFAULT_NODELIMIT;
    }

    if (pu.uid)
        sprintf(ans, "%d", pu.dirlimit);
    else
        sprintf(ans, "%d", PC_DEFAULT_DIRLIMIT);
    sprintf(sql, "个人文集允许最多目录数 [%s]:", ans);
    getdata(14, 0, sql, ans, 5, DOECHO, NULL, false);
    if (ans[0]) {
        pu.dirlimit = atoi(ans);
        if (pu.dirlimit <= 0)
            pu.dirlimit = PC_DEFAULT_DIRLIMIT;
    }

    if (pu.uid)
        getdata(15, 0, "确定修改? (Y/N) [N]:", ans, 3, DOECHO, NULL, true);
    else
        getdata(15, 0, "确定开启? (Y/N) [N]:", ans, 3, DOECHO, NULL, true);
    if (ans[0] != 'y' && ans[0] != 'Y')
        return 0;

    if (pu.uid == 0)
        pu.createtime = time(0);

    if (add_pc_users(&pu)) {
        char buf1[256];

        lookupuser->flags |= PCORP_FLAG;
        move(18, 0);
        if (pu.uid)
            prints("修改成功", lookupuser->userid);
        else
            prints("添加成功,用户%s已经拥有个人文集", lookupuser->userid);
        pressanykey();
        sprintf(buf1, "%s 增加 %s 的个人文集", getCurrentUser()->userid, lookupuser->userid);
        securityreport(buf1, lookupuser, NULL, getSession());
        return 1;
    } else {
        move(18, 0);
        prints("系统错误......", lookupuser->userid);
        pressanykey();
        return 0;
    }
}

static int pc_add_friend(char *uident, char *fpath, int echo)
{
    int seek;
    int id;
    struct userec *lookupuser;

    if (!(id = getuser(uident, &lookupuser))) {
        if (echo) {
            move(3, 0);
            prints("Invalid User Id");
            clrtoeol();
            pressreturn();
            clear();
        }
        return 0;
    }
    strcpy(uident, lookupuser->userid);

    seek = seek_in_file(fpath, uident);
    if (seek) {
        if (echo) {
            move(2, 0);
            prints("输入的ID 已经存在!");
            pressreturn();
        }
        return -1;
    }

    seek = addtofile(fpath, uident);;

    return 1;
}

static int pc_del_friend(char *uident, char *fpath)
{
    struct userec *lookupuser;
    int id;

    if (!(id = getuser(uident, &lookupuser))) {
        move(3, 0);
        prints("Invalid User Id");
        clrtoeol();
        pressreturn();
        clear();
        return 0;
    }
    strcpy(uident, lookupuser->userid);

    del_from_file(fpath, uident);;

    return 1;
}

static int pc_change_friend()
{
    char buf[STRLEN];
    int count;
    char ans[20];
    char uident[STRLEN];

    sethomefile(buf, getCurrentUser()->userid, "pc_friend");

    while (1) {
        clear();
        prints("设定个人文集好友名单\n");
        count = listfilecontent(buf);
        if (count)
            getdata(1, 0, "(A)增加 (D)删除or (E)离开[E]", ans, 7, DOECHO, NULL, true);
        else
            getdata(1, 0, "(A)增加 or (E)离开 [E]: ", ans, 7, DOECHO, NULL, true);
        if (*ans == 'A' || *ans == 'a') {
            move(1, 0);
            usercomplete("增加个人文集好友成员: ", uident);
            if (*uident != '\0') {
                pc_add_friend(uident, buf, 1);
            }
        } else if ((*ans == 'D' || *ans == 'd') && count) {
            move(1, 0);
            namecomplete("删除个人文集好友成员: ", uident);
            if (uident[0] != '\0') {
                pc_del_friend(uident, buf);
            }
        } else
            break;
    }

    return 1;
}

/******************************
 *
 *
 * 选择分区
 *
 *
 **********************/

static int pc_sec_prekey(struct _select_def *conf, int *key)
{
    if (*key >= 'a' && *key <= 'z') {
        conf->new_pos = *key - 'a' + 3;
        return SHOW_SELCHANGE;
    }
    if (*key >= 'A' && *key <= 'Z') {
        conf->new_pos = *key - 'A' + 3;
        return SHOW_SELCHANGE;
    }
    switch (*key) {
    case '*':
        conf->new_pos = 1;
        return SHOW_SELCHANGE;
    case KEY_RIGHT:
        if (conf->pos < 15) {
            conf->new_pos += 14;
            return SHOW_SELCHANGE;
        } else {
            *key = '\n';
        }
        break;
    case KEY_LEFT:
        if (conf->pos > 14) {
            conf->new_pos -= 14;
            return SHOW_SELCHANGE;
        }
        break;
    case 'q':
        *key = KEY_LEFT;
        break;
    case 'p':
    case 'k':
        *key = KEY_UP;
        break;
    case 'n':
    case 'j':
        *key = KEY_DOWN;
        break;
    }
    return SHOW_CONTINUE;
}

static int pc_sec_title(struct _select_def *conf)
{
    clear();
    move(1, 0);
    prints(" 请选择个人文集");
    return SHOW_CONTINUE;
}

static int pc_sec_show(struct _select_def *conf, int i)
{
    if (i > 2)
        prints("%c", 'A' + i - 3);
    else if (i == 1)
        prints("*)自己的个人文集");
    else if (i == 2)
        prints("直接选择个人文集");

    return SHOW_CONTINUE;
}

static int pc_sec_select(struct _select_def *conf)
{

    if (conf->pos > 2)
        pc_selusr('A' + conf->pos - 3);
    else if (conf->pos == 1)
        pc_read(getCurrentUser()->userid);
    else if (conf->pos == 2)
        pc_sel_user();

    return SHOW_REFRESH;

}

static int pc_sec()
{
    struct _select_def group_conf;
    POINT *pts;
    int i;
    
    modify_user_mode(PC);
    clear();

    bzero(&group_conf, sizeof(struct _select_def));

    pts = (POINT *) malloc(sizeof(POINT) * 28);
    for (i = 0; i < 28; i++) {
        pts[i].x = 5 + (i > 13 ? 30 : 0);
        pts[i].y = i % 14 + 3;
    }
    group_conf.item_per_page = 28;
    group_conf.flag = LF_VSCROLL | LF_BELL | LF_LOOP;
    group_conf.prompt = "◆";
    group_conf.item_pos = pts;
    group_conf.title_pos.x = 0;
    group_conf.title_pos.y = 0;
    group_conf.pos = 1;
    group_conf.page_pos = 1;

    group_conf.item_count = 28;
    group_conf.show_data = pc_sec_show;
    group_conf.show_title = pc_sec_title;
    group_conf.pre_key_command = pc_sec_prekey;
    group_conf.on_select = pc_sec_select;

    list_select_loop(&group_conf);

    free(pts);

    return 0;
}

/***************************************************
 *
 *
 *
 *
 * 选择具体用户
 *
 *
 *
 *
 ***************************************************/

struct _pc_selusr *pc_s;

static int pc_seldir_prekey(struct _select_def *conf, int *key)
{
    switch (*key) {
    case KEY_RIGHT:
        *key = '\n';
        break;
    case 'q':
        *key = KEY_LEFT;
        break;
    case 'p':
    case 'k':
        *key = KEY_UP;
        break;
    case 'n':
    case 'j':
        *key = KEY_DOWN;
        break;
    }
    return SHOW_CONTINUE;
}

static int pc_selusr_title(struct _select_def *conf)
{
    clear();
    docmdtitle("[个人文集选择]", "退出[\033[1;32mq\033[m]");
    move(2, 0);
    prints("\033[0;1;44m  %-4s %-13s %-40s %-15s\033[m", "序号", "用户", "个人文集名字", "开启时间");
    update_endline();
    return SHOW_CONTINUE;
}

static int pc_selusr_show(struct _select_def *conf, int i)
{
    char newts[20];

    prints("%-4d %-13s %-40s %-15s", i, pc_s[i - 1].userid, pc_s[i - 1].corpusname, tt2timestamp(pc_s[i - 1].createtime, newts));

    return SHOW_CONTINUE;
}

static int pc_selusr_select(struct _select_def *conf)
{
    pc_read(pc_s[conf->pos - 1].userid);

    return SHOW_REFRESH;
}

static int pc_selusr(char prefix)
{
    struct _select_def group_conf;
    POINT *pts;
    int i, ret;

    clear();

    ret = pc_load_usr(&pc_s, prefix);

    if (ret <= 0) {
        move(3, 0);
        prints("数据错误或者尚未有该字母开头的用户个人文集");
        pressanykey();
        return -1;
    }

    bzero(&group_conf, sizeof(struct _select_def));

    //TODO: 窗口大小动态改变的情况？这里有bug
    pts = (POINT *) malloc(sizeof(POINT) * BBS_PAGESIZE);
    for (i = 0; i < BBS_PAGESIZE; i++) {
        pts[i].x = 3;
        pts[i].y = i + 3;
    }
    group_conf.item_per_page = BBS_PAGESIZE;
    group_conf.flag = LF_VSCROLL | LF_BELL | LF_LOOP;
    group_conf.prompt = "◆";
    group_conf.item_pos = pts;
    group_conf.title_pos.x = 0;
    group_conf.title_pos.y = 0;
    group_conf.pos = 1;
    group_conf.page_pos = 1;

    group_conf.item_count = ret;
    group_conf.show_title = pc_selusr_title;
    group_conf.show_data = pc_selusr_show;
    group_conf.pre_key_command = pc_seldir_prekey;
    group_conf.on_select = pc_selusr_select;

    list_select_loop(&group_conf);

    free(pts);
    free(pc_s);

    return 0;

}

/****************************************************
 *
 *
 *
 *
 * 个人文集分区选择的select
 *
 *
 *
 *
 *****************************************************/

/*******
 * 检查权限的函数
 * 返回 1: 普通用户
 *      2: 好友
 *      5: 所有权限
 */
char pc_select_user[IDLEN + 2];

static int pc_is_owner(char *userid)
{

    if (!strcasecmp(userid, getCurrentUser()->userid) && pc_u->createtime >= getCurrentUser()->firstlogin)
        return 1;
    return 0;
}

static int pc_is_admin(char *userid)
{

    if (HAS_PERM(getCurrentUser(), PERM_ADMIN) || pc_is_owner(userid))
        return 1;

    return 0;
}

static int pc_is_friend(char *userid)
{
    char fpath[STRLEN];

    sethomefile(fpath, userid, "pc_friend");
    if (seek_in_file(fpath, getCurrentUser()->userid))
        return 1;

    return 0;
}

static int pc_perm(char *userid)
{
    if (pc_is_owner(userid))
        return 5;
    if (pc_is_admin(userid))
        return 2;

    if (pc_is_friend(userid))
        return 2;

    return 1;

}

static int pc_seldir_show(struct _select_def *conf, int i)
{
    switch (i) {
    case 1:
        prints(" 公开区");
        break;
    case 2:
        prints(" 好友区");
        break;
    case 3:
        prints(" 私人区");
        break;
    case 4:
        prints(" 收藏区");
        break;
    case 5:
        prints(" 删除区");
        break;
    default:
        prints(" NULL ");
        break;
    }

    return SHOW_CONTINUE;
}

static int pc_seldir_title(struct _select_def *conf)
{
    clear();
    move(2, 0);
    prints("           \033[1;31m%s的个人文集 -- %s\033[m", pc_u->username, pc_u->description);
    return SHOW_CONTINUE;
}

static int pc_seldir_select(struct _select_def *conf)
{
    pc_dirmode = conf->pos;

    pc_read_dir(1);

    if (pc_select_user[0])
        return SHOW_QUIT;

    return SHOW_REFRESH;
}

int pc_read(char *userid)
{
    struct userec *lookupuser;
    struct _select_def group_conf;
    POINT *pts = NULL;
    int i;

    if (pc_u)
        return 0; //reentry
    
    pc_u = (struct pc_users *) malloc(sizeof(struct pc_users));
    if (pc_u == NULL)
        return 0;

    strncpy(pc_select_user, userid, IDLEN + 2);
    pc_select_user[IDLEN] = 0;

  startuser:
    clear();
    if (getuser(pc_select_user, &lookupuser) == 0) {
        clear();
        move(7, 0);
        prints("此用户不存在");
        pressanykey();
        free(pc_u);
        if (pts) free(pts);
        pc_u = NULL;
        return 0;
    }

    pc_select_user[0] = 0;
    bzero(pc_u, sizeof(struct pc_users));

    if (!get_pc_users(pc_u, lookupuser->userid)) {
        clear();
        move(7, 0);
        prints("没有此用户个人文集存在");
        free(pc_u);
        if (pts) free(pts);
        pc_u = NULL;
        pressanykey();
        return 0;
    }

    pc_fav_dir = 0;
    pc_dirmode = 1;

    bzero(&group_conf, sizeof(struct _select_def));

    if (pts == NULL) {
        pts = (POINT *) malloc(sizeof(POINT) * BBS_PAGESIZE);
        for (i = 0; i < BBS_PAGESIZE; i++) {
            pts[i].x = 30;
            pts[i].y = i + 10;
        }
    }

    group_conf.item_per_page = BBS_PAGESIZE;
    group_conf.flag = LF_VSCROLL | LF_BELL | LF_LOOP;
    group_conf.prompt = "◆";
    group_conf.item_pos = pts;
    group_conf.title_pos.x = 0;
    group_conf.title_pos.y = 0;
    group_conf.pos = 1;
    group_conf.page_pos = 1;

    group_conf.item_count = pc_perm(lookupuser->userid);
    group_conf.show_data = pc_seldir_show;
    group_conf.show_title = pc_seldir_title;
    group_conf.pre_key_command = pc_seldir_prekey;
    group_conf.on_select = pc_seldir_select;

    list_select_loop(&group_conf);

    if (pc_select_user[0])
        goto startuser;

    if (pts) free(pts);
    free(pc_u);
    pc_u = NULL;

    return 0;
}


/**************************************************
 *
 *
 * 以下是文章列表select
 *
 *
 *
 ***************************************************/



struct pc_nodes *pc_n = NULL;

//int pc_dir_start = 0;

/* 用于评论select,表示当前是哪个node,对应 pc_n[i] */
int pc_now_node_ent = 0;

/* 复制粘贴的东西，保存临时node nid */
unsigned long pc_pasteboard = 0;

static int pc_get_fav_root(unsigned long *nid)
{
    struct pc_nodes pn;
    int ret;

    ret = get_pc_nodes(&pn, pc_u->uid, 0, 1, 3, 0, 1, 0);
    if (ret <= 0)
        return ret;

    *nid = pn.nid;
    return 1;
}

static int pc_add_fav_root()
{
    struct pc_nodes pn;

    bzero(&pn, sizeof(pn));
    pn.pid = 0;
    pn.type = 1;

    strncpy(pn.hostname, uinfo.from, 20);
    pn.hostname[20] = 0;

    pn.created = time(0);
    pn.changed = pn.created;
    pn.uid = pc_u->uid;
    pn.access = 3;

    return add_pc_nodes(&pn);

}

static int pc_conv_body_to_file(char *body, char *fname)
{
    int fd;
    unsigned long size;
    unsigned long hadwrite = 0;
    int nd;
    int ret;

    if (!body)
        return 0;

    size = strlen(body);

    if ((fd = open(fname, O_WRONLY | O_CREAT, 0600)) < 0)
        return 0;

    do {
        if (size > 10240)
            nd = 10240;
        else
            nd = size;

        ret = write(fd, body + hadwrite, nd);
        if (ret <= 0) {
            close(fd);
            return 0;
        }
        size -= ret;
        hadwrite += ret;
    } while (size > 0);

    close(fd);
    return 1;
}

static int pc_conv_com_to_file(unsigned long nid, char *fname)
{
    int fd;
    struct pc_comments pn;
    unsigned long size;
    unsigned long hadwrite = 0;
    int nd;
    int ret;
    char buf[256];
    struct userec *lookupuser;

    ret = get_pc_a_com(&pn, nid);
    if (ret <= 0) {
        return 0;
    }

    if (getuser(pn.username, &lookupuser) == 0) {
        if (pn.body)
            free(pn.body);
        return 0;
    }

    if ((fd = open(fname, O_WRONLY | O_CREAT, 0600)) < 0) {
        if (pn.body)
            free(pn.body);
        return 0;
    }

    snprintf(buf, 255, "发信人: %s (%s), 个人文集\n", lookupuser->userid, lookupuser->username);
    write(fd, buf, strlen(buf));
    snprintf(buf, 255, "标  题: %s\n", pn.subject);
    write(fd, buf, strlen(buf));
    snprintf(buf, 255, "发信站: %s (%24.24s), 评论\n\n", BBS_FULL_NAME, ctime(&pn.created));
    write(fd, buf, strlen(buf));

    if (pn.body) {

        size = strlen(pn.body);

        do {
            if (size > 10240)
                nd = 10240;
            else
                nd = size;

            ret = write(fd, pn.body + hadwrite, nd);
            if (ret <= 0) {
                close(fd);
                free(pn.body);
                return 0;
            }
            size -= ret;
            hadwrite += ret;
        } while (size > 0);

        free(pn.body);
    }

    close(fd);
    return 1;
}
static const char TAG_NEWLN[11][8] = {
    "br",
    "/p",
    "/tr",
    "/div",
    "/h1",
    "/h2",
    "/h3",
    "/h4",
    "/h5",
    "/h6",
    "/li"
};

static const char TAG_SPACE[3][8] = {
    "/td",
    "/th",
    "/span"
};

static const char TAG_SPECL[5][2][8] = {
    {"nbsp", " "},
    {"amp", "&"},
    {"quot", "\""},
    {"gt", ">"},
    {"lt", "<"}
};

static const char TAG_LINK[6][2][8] = {
    {"a", "href="},
    {"img", "src="},
    {"applet", "src="},
    {"embed", "src="},
    {"bgsound", "src="},
    {"iframe", "src="}
};

#define PUTC(chr)  buf[hadwrite]=chr; hadwrite++; if (hadwrite==256) { ret = write(fd, buf, 256); hadwrite=0; }
#define PUTS(str)  temp=str; while(*temp) { PUTC(*temp); temp++; }
#define GET_TAG(condi) tagp=0; tag[0]=0; while ((condi)&&(*ptr)&&(tagp<255)) { tag[tagp] = *ptr; tagp++; ptr++; } tag[tagp]=0;

static int pc_conv_node_to_file(unsigned long nid, char *fname)
{
    char *ptr, *temp;
    char tag[256];
    int tagp;


    int fd;
    struct pc_nodes pn;
    unsigned long size;
    unsigned long hadwrite = 0;
    int nd;
    int ret;
    char buf[256];
    struct userec *lookupuser;

    if (getuser(pc_u->username, &lookupuser) == 0)
        return 0;

    if ((fd = open(fname, O_WRONLY | O_CREAT, 0600)) < 0)
        return 0;

    ret = get_pc_a_node(&pn, nid);
    if (ret <= 0) {
        close(fd);
        return 0;
    }

    snprintf(buf, 255, "发信人: %s (%s), 个人文集\n", lookupuser->userid, lookupuser->username);
    write(fd, buf, strlen(buf));
    snprintf(buf, 255, "标  题: %s\n", pn.subject);
    write(fd, buf, strlen(buf));
    snprintf(buf, 255, "发信站: %s (%24.24s), 文集\n\n", BBS_FULL_NAME, ctime(&pn.created));
    write(fd, buf, strlen(buf));

    if (pn.body) {
        if (pn.htmltag) {//need to parse html
            ptr = pn.body;
            PUTS("(本文包含HTML标记，终端模式下可能无法正确浏览)\n");
            do {
                switch (*ptr) {
                case '<':
                    ptr++;
                    GET_TAG((*ptr != '>') && (*ptr != ' '));
                    for (nd = 0; nd < 11; nd++)
                        if (!strcasecmp(tag, TAG_NEWLN[nd])) {
                            PUTS("\n");
                            break;
                        }
                    for (nd = 0; nd < 3; nd++)
                        if (!strcasecmp(tag, TAG_SPACE[nd])) {
                            PUTC(' ');
                            break;
                        }
                    for (nd = 0; nd < 6; nd++) {
                        if (!strcasecmp(tag, TAG_LINK[nd][0]) && (*ptr == ' ')) {
                            do {
                                while (*ptr == ' ')
                                    ptr++;
                                GET_TAG((*ptr != '\"') && (*ptr != '/') && (*ptr != '>'));
                                if (*ptr != '\"') {
                                    if (*ptr)
                                        ptr++;
                                    continue;
                                }
                                ptr++;
                                if (!strcasecmp(tag, TAG_LINK[nd][1])) {
                                    GET_TAG(*ptr != '\"');
                                    PUTS(" [ ");
                                    PUTS(tag);
                                    PUTS(" ] ");
                                    if (*ptr)
                                        ptr++;
                                } else {
                                    GET_TAG(*ptr != '\"');
                                    if (*ptr)
                                        ptr++;
                                }
                            } while (*ptr == ' ');
                            break;
                        }
                    }
                    while ((*ptr) && (*ptr != '>'))
                        ptr++;
                    if (*ptr == '>')
                        ptr++;
                    break;
                case '&':
                    ptr++;
                    GET_TAG((*ptr >= 'a') && (*ptr <= 'z'));
                    for (tagp = 0; tagp < 5; tagp++)
                        if (!strcasecmp(tag, TAG_SPECL[tagp][0])) {
                            PUTC(TAG_SPECL[tagp][1][0]);
                            break;
                        }
                    ptr++;
                    break;
                default:
                    PUTC(*ptr);
                    ptr++;
                }
                if (ret <= 0) {
                    close(fd);
                    free(pn.body);
                    return 0;
                }
            } while (*ptr);
            if (hadwrite)
                ret = write(fd, buf, hadwrite);
        } else {//normal article
            size = strlen(pn.body);

            do {
                if (size > 10240)
                    nd = 10240;
                else
                    nd = size;

                ret = write(fd, pn.body + hadwrite, nd);
                if (ret <= 0) {
                    close(fd);
                    free(pn.body);
                    return 0;
                }
                size -= ret;
                hadwrite += ret;
            } while (size > 0);
        }

        free(pn.body);
    }

    close(fd);
    return 1;
}

#undef PUTS
#undef PUTC
#undef GET_TAG

/*******
 * num==-1: 增加
 * num >=0: 修改
 *          num为序号
 */
static int pc_add_a_dir(unsigned long nid)
{
    struct pc_nodes pn;
    char ans[201];
    int ret;

    bzero(&pn, sizeof(pn));
    if (nid) {
        if (get_pc_a_node(&pn, nid) <= 0)
            return 0;
    }

    move(t_lines - 1, 0);
    clrtoeol();

    if (nid) {
        strncpy(ans, pn.subject, 200);
        ans[200] = 0;
    } else
        ans[0] = 0;
    getdata(t_lines - 1, 0, "标题:", ans, 200, DOECHO, NULL, false);
    if (!ans[0])
        return 0;
    strncpy(pn.subject, ans, 200);
    pn.subject[200] = 0;

    if (pn.body)
        free(pn.body);

    pn.body = NULL;

    strncpy(pn.hostname, uinfo.from, 20);
    pn.hostname[20] = 0;

    pn.created = time(0);
    pn.changed = pn.created;
    pn.uid = pc_u->uid;
    pn.pid = pc_fav_dir;
    pn.type = 1;
    pn.comment = 1;
    pn.access = 3;

    ret = add_pc_nodes(&pn);

    return ret;
}


/*******
 * nid==-1: 增加
 * nid >=0: 修改
 *          num为序号
 */
static int pc_add_a_node(unsigned long nid)
{
    struct pc_nodes pn;
    char ans[201];
    char fpath[STRLEN];
    int ret;

    bzero(&pn, sizeof(pn));
    if (nid) {
        if (get_pc_a_node(&pn, nid) <= 0)
            return 0;
    }

    move(t_lines - 1, 0);
    clrtoeol();

    if (nid) {
        strncpy(ans, pn.subject, 200);
        ans[200] = 0;
    } else
        ans[0] = 0;
    getdata(t_lines - 1, 0, "标题:", ans, 200, DOECHO, NULL, false);
    if (!ans[0])
        return 0;
    strncpy(pn.subject, ans, 200);
    pn.subject[200] = 0;

    gettmpfilename(fpath, "pc.node");
    unlink(fpath);

    if (nid && pn.body) {
        pc_conv_body_to_file(pn.body, fpath);
        free(pn.body);
        pn.body = NULL;
    } else
        unlink(fpath);

    if (vedit(fpath, 0, NULL, NULL, 0) == -1)
        return 0;

    pn.body = NULL;
    if (!pc_conv_file_to_body(&(pn.body), fpath)) {
        unlink(fpath);
        return 0;
    }
    unlink(fpath);

    strncpy(pn.hostname, uinfo.from, 20);
    pn.hostname[20] = 0;

    pn.created = time(0);
    pn.changed = pn.created;
    pn.uid = pc_u->uid;
    pn.pid = pc_fav_dir;
    pn.comment = 1;
    pn.access = pc_dirmode - 1;

    ret = add_pc_nodes(&pn);

    if (pn.body)
        free(pn.body);

    return ret;
}

static int pc_dir_title(struct _select_def *conf)
{
    int chkmailflag;

    chkmailflag = chkmail();

    clear();
    move(0, 0);

    if (chkmailflag == 2) {
        prints("\033[0;1;5;44m                         [您的信箱超过容量,不能再收信!]                       \033[m");
    } else if (chkmailflag) {
        prints("\033[0;1;5;44m                                   [您有信件]                                 \033[m");
    } else {
        prints("\033[0;1;44m  %s的个人文集 -- %-44s ", pc_u->username, pc_u->corpusname);
        switch (pc_dirmode) {
        case 2:
            prints("[好友区]\033[m");
            break;
        case 3:
            prints("[私人区]\033[m");
            break;
        case 4:
            prints("[收藏区]\033[m");
            break;
        case 5:
            prints("[删除区]\033[m");
            break;
        default:
            prints("[公开区]\033[m");
            break;
        }
    }

    move(1, 0);
    if (pc_dirmode != 2)
        prints("               退出[\033[1;32mq\033[m] 增加[\033[1;32ma\033[m] 删除[\033[1;32md\033[m] 修改[\033[1;32me\033[m] 拷贝[\033[1;32mc\033[m] 粘贴[\033[1;32mp\033[m]");
    else
        prints
            ("退出[\033[1;32mq\033[m] 增加[\033[1;32ma\033[m] 删除[\033[1;32md\033[m] 修改[\033[1;32me\033[m] 拷贝[\033[1;32mc\033[m] 粘贴[\033[1;32mp\033[m] 修改好友[\033[1;32mo\033[m] 导入好友[\033[1;32mi\033[m]");
    move(2, 0);
    prints("\033[0;1;44m  %-4s %-6s %-38s %-4s %-4s %-12s\033[m", "序号", "类别", "标题", "评论", "访问", "文章发表时间");
    update_endline();
    return SHOW_CONTINUE;
}

static int pc_dir_select(struct _select_def *conf)
{
    char fpath[STRLEN];
    int ch;

    clear();

    //pc_add_visitcount(pc_n[conf->pos - conf->page_pos].nid);
    pc_n[conf->pos - conf->page_pos].visitcount++;
        /**如果是目录***/
    if (pc_dirmode == 4 && pc_n[conf->pos - conf->page_pos].type == 1) {
        unsigned long old_fav_dir = pc_fav_dir;

        pc_fav_dir = pc_n[conf->pos - conf->page_pos].nid;

        pc_read_dir(0);

        pc_fav_dir = old_fav_dir;

        if (pc_select_user[0])
            return SHOW_QUIT;

        return SHOW_DIRCHANGE;
    }

        /***先显示文章正文*****/
    gettmpfilename(fpath, "pc.node");
    unlink(fpath);
    if (!pc_conv_node_to_file(pc_n[conf->pos - conf->page_pos].nid, fpath)) {
        move(3, 0);
        prints("没有内容");
        pressanykey();
        return SHOW_REFRESH;
    }
    ch = mmap_more(fpath, 1, "r", "PersonalCorp");
    if (ch == 0) {
        move(t_lines - 1, 0);
        if (pc_n[conf->pos - conf->page_pos].comment)
            prints("\033[0;1;44m r 查看所有评论(共%d条)                                                         \033[m", pc_n[conf->pos - conf->page_pos].commentcount);
        else
            prints("\033[0;1;44m 本文不许评论 \033[m");
        ch = igetkey();
    }

    switch (ch) {
    case 'r':
        pc_now_node_ent = conf->pos - conf->page_pos;
        pc_read_comment();
        return SHOW_DIRCHANGE;
    default:
        break;
    }

    unlink(fpath);

    return SHOW_REFRESH;
}

static int pc_dir_show(struct _select_def *conf, int i)
{

    char newts[20];

    prints(" %-3d %s %-38s %-3d %-3d %-12s", i, pc_n[i - conf->page_pos].type == 0 ? "[文章]" : "\033[1;33m[目录]\033[m", pc_n[i - conf->page_pos].subject, pc_n[i - conf->page_pos].commentcount,
           pc_n[i - conf->page_pos].visitcount, tt2timestamp(pc_n[i - conf->page_pos].created, newts));

    return SHOW_CONTINUE;
}

static int pc_dir_key(struct _select_def *conf, int key)
{
    char ans[4];
    switch (key) {
    case 'a':
        if (!pc_is_owner(pc_u->username))
            return SHOW_CONTINUE;
        if (conf->item_count > pc_u->nodelimit) {
            clear();
            prints("文章数目达到限制");
            pressreturn();
            return SHOW_REFRESH;
        }
        if (pc_add_a_node(0))
            return SHOW_DIRCHANGE;
        return SHOW_REFRESH;
        break;
    case 'i':
        {
            char ans[4];

            if (!pc_is_owner(pc_u->username) || pc_dirmode != 2)
                return SHOW_CONTINUE;

            clear();
            getdata(3, 0, "确实要导入好友名单? (Y/N) [N]: ", ans, 3, DOECHO, NULL, true);
            if (ans[0] != 'y' && ans[0] != 'Y') {
                return SHOW_REFRESH;
            } else {
                char fpath[STRLEN];
                char buf[STRLEN];
                struct friends fh;
                FILE *fp;
                int suc = 0;

                sethomefile(fpath, getCurrentUser()->userid, "friends");
                sethomefile(buf, getCurrentUser()->userid, "pc_friend");

                if ((fp = fopen(fpath, "r")) == NULL)
                    return SHOW_REFRESH;

                while (fread(&fh, sizeof(fh), 1, fp)) {

                    if (pc_add_friend(fh.id, buf, 0) > 0)
                        suc++;

                }
                fclose(fp);

                move(10, 0);
                clrtoeol();
                prints("成功导入 %d 个好友", suc);
                pressanykey();

                return SHOW_REFRESH;
            }
        }
    case 'o':
        if (!pc_is_owner(pc_u->username) || pc_dirmode != 2)
            return SHOW_CONTINUE;
        pc_change_friend();
        return SHOW_REFRESH;
        break;
    case 'g':
        if (!pc_is_owner(pc_u->username) || pc_dirmode != 4 || pc_fav_dir == 0)
            return SHOW_CONTINUE;
        if (conf->item_count > pc_u->dirlimit) {
            clear();
            prints("目录数目达到限制");
            pressreturn();
            return SHOW_REFRESH;
        }
        if (pc_add_a_dir(0))
            return SHOW_DIRCHANGE;
        return SHOW_REFRESH;
        break;
    case 'd':
        if (!pc_is_admin(pc_u->username))
            return SHOW_CONTINUE;

        move(2,0);
        clrtoeol();
        ans[0]=0;
        getdata(2, 0, "删除这条记录(Y/N) [N]: ", ans, 3, DOECHO, NULL, true);
        if (ans[0] != 'y' && ans[0]!='Y') {
            // KCN 普度众生
        } else if (pc_dirmode == 5) {
            if (del_pc_nodes(pc_n[conf->pos - conf->page_pos].nid, pc_n[conf->pos - conf->page_pos].access, pc_u->uid)) {
                return SHOW_DIRCHANGE;
            }
        } else if (pc_dirmode == 4) {
            struct pc_nodes pn;

            if (get_pc_nodes(&pn, pc_u->uid, pc_n[conf->pos - conf->page_pos].nid, -1, 3, 0, 1, 0) > 0) {
                move(t_lines - 1, 0);
                clrtoeol();
                prints("不能删除非空目录,按任意键继续");
                igetkey();
                update_endline();
                return SHOW_CONTINUE;
            }
            if (del_pc_node_junk(pc_n[conf->pos - conf->page_pos].nid, pc_n[conf->pos - conf->page_pos].access, pc_u->uid))
                return SHOW_DIRCHANGE;
        } else {
            if (del_pc_node_junk(pc_n[conf->pos - conf->page_pos].nid, pc_n[conf->pos - conf->page_pos].access, pc_u->uid))
                return SHOW_DIRCHANGE;
        }
        return SHOW_REFRESH;
        break;
    case 'D':
        {
            char ans[4];

            if (!pc_is_owner(pc_u->username))
                return SHOW_CONTINUE;
            if (pc_dirmode != 5)
                return SHOW_CONTINUE;
            clear();
            getdata(1, 0, "确实要清空垃圾箱吗? (Y/N) [N]:", ans, 3, DOECHO, NULL, true);
            if (ans[0] != 'y' && ans[0] != 'Y')
                return SHOW_REFRESH;

            pc_del_junk(pc_u->uid);

            return SHOW_DIRCHANGE;
        }
    case 'e':
        if (!pc_is_owner(pc_u->username))
            return SHOW_CONTINUE;
        if (pc_dirmode == 4 && pc_n[conf->pos - conf->page_pos].type == 1) {
            if (pc_add_a_dir(pc_n[conf->pos - conf->page_pos].nid))
                return SHOW_DIRCHANGE;
            return SHOW_REFRESH;
        }
        if (pc_add_a_node(pc_n[conf->pos - conf->page_pos].nid))
            return SHOW_DIRCHANGE;
        return SHOW_REFRESH;
        break;
    case 'c':
        if (pc_n[conf->pos - conf->page_pos].type == 1) {
            move(t_lines - 1, 0);
            clrtoeol();
            prints("不能复制目录,按任意键继续");
            igetkey();
            update_endline();
            return SHOW_CONTINUE;
        }
        pc_pasteboard = pc_n[conf->pos - conf->page_pos].nid;
        move(t_lines - 1, 0);
        clrtoeol();
        prints("已经复制该条目到剪贴板中,按任意键继续");
        igetkey();
        update_endline();
        return SHOW_CONTINUE;
        break;
    case 'p':
        if (!pc_is_owner(pc_u->username))
            return SHOW_CONTINUE;
        if (conf->item_count > pc_u->nodelimit) {
            clear();
            prints("文章数目达到限制");
            pressreturn();
            return SHOW_REFRESH;
        }
        if (pc_pasteboard <= 0) {
            move(t_lines - 1, 0);
            clrtoeol();
            prints("剪贴板没有内容,按任意键继续");
            igetkey();
            update_endline();
            return SHOW_DIRCHANGE;
        }
        if (pc_paste_node(pc_pasteboard, pc_u->uid, pc_dirmode - 1, pc_dirmode == 4 ? pc_fav_dir : 0)) {
            move(t_lines - 1, 0);
            clrtoeol();
            prints("粘贴成功,按任意键继续");
            igetkey();
            update_endline();
            return SHOW_DIRCHANGE;
        }
        move(t_lines - 1, 0);
        clrtoeol();
        prints("粘贴失败,按任意键继续");
        igetkey();
        update_endline();
        return SHOW_CONTINUE;
        break;
    case 's':
        {
            char ans[20];
            struct userec *lookupuser;

            move(0, 0);
            clrtoeol();
            prints("选择别的用户的个人文集");
            move(1, 0);
            clrtoeol();
            prints("输入用户id:");
            usercomplete(NULL, ans);

            if (ans[0] == 0 || ans[0] == '\n' || ans[0] == '\r')
                return SHOW_REFRESH;

            if (getuser(ans, &lookupuser) == 0)
                return SHOW_REFRESH;

            strncpy(pc_select_user, ans, IDLEN + 2);
            pc_select_user[IDLEN + 1] = 0;
            return SHOW_QUIT;

            break;
        }
    case 'v':
        i_read_mail();
        return SHOW_REFRESH;
        break;
    }

    return SHOW_CONTINUE;
}

static int pc_dir_getdata(struct _select_def *conf, int pos, int len)
{

    int i;

    for (i = 0; i < BBS_PAGESIZE; i++) {
        if (pc_n[i].body)
            free(pc_n[i].body);
    }

    bzero(pc_n, sizeof(struct pc_nodes) * BBS_PAGESIZE);

    if (conf->item_count - conf->page_pos < BBS_PAGESIZE)
        conf->item_count = count_pc_nodes(pc_u->uid, pc_fav_dir, -1, pc_dirmode - 1);

    if (pos <= 0) {
        clear();
        move(3, 0);
        prints("pos:%d\n", pos);
        pressanykey();
    }

    i = get_pc_nodes(pc_n, pc_u->uid, pc_fav_dir, -1, pc_dirmode - 1, pos - 1, BBS_PAGESIZE, 0);

    if (i < 0)
        return SHOW_QUIT;

    if (i == 0) {

        conf->page_pos = 1;

        i = get_pc_nodes(pc_n, pc_u->uid, pc_fav_dir, -1, pc_dirmode - 1, 0, BBS_PAGESIZE, 0);

        if (i <= 0)
            return SHOW_QUIT;
    }

    return SHOW_CONTINUE;
}

static int pc_dir_prekey(struct _select_def *conf, int *key)
{
    switch (*key) {
    case 'q':
        *key = KEY_LEFT;
        break;
    case 'k':
        *key = KEY_UP;
        break;
    case 'j':
        *key = KEY_DOWN;
        break;
    }
    return SHOW_CONTINUE;
}

static int pc_read_dir(int first)
{
    struct _select_def group_conf;
    POINT *pts;
    int i;

    if (pc_dirmode <= 0 || pc_dirmode > 5)
        return 0;

//      pc_dir_start = 0;

    if (pc_dirmode == 4) {
        int ret;
        unsigned long retnid;

        if (pc_fav_dir == 0) {
            ret = pc_get_fav_root(&retnid);
            if (ret < 0)
                return 0;
            if (ret == 0) {
                pc_add_fav_root();
                ret = pc_get_fav_root(&retnid);
                if (ret <= 0)
                    return 0;
            }
            pc_fav_dir = retnid;
        }
    }

    if (first) {
        pc_n = (struct pc_nodes *) malloc(sizeof(struct pc_nodes) * BBS_PAGESIZE);
        if (pc_n == NULL)
            return 0;
    }

    bzero(&group_conf, sizeof(struct _select_def));

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
    group_conf.pos = 1;
    group_conf.page_pos = 1;

    group_conf.show_data = pc_dir_show;
    group_conf.show_title = pc_dir_title;
    group_conf.pre_key_command = pc_dir_prekey;
    group_conf.on_select = pc_dir_select;
    group_conf.get_data = pc_dir_getdata;
    group_conf.key_command = pc_dir_key;

    bzero(pc_n, sizeof(struct pc_nodes) * BBS_PAGESIZE);

    group_conf.item_count = count_pc_nodes(pc_u->uid, pc_fav_dir, -1, pc_dirmode - 1);
    i = get_pc_nodes(pc_n, pc_u->uid, pc_fav_dir, -1, pc_dirmode - 1, 0, BBS_PAGESIZE, 0);

    if (i < 0) {
        free(pts);
        if (first)
            free(pc_n);
        if (pc_dirmode == 4)
            pc_fav_dir = 0;
        return -1;
    }
    if (i == 0) {
        if (!pc_is_owner(pc_u->username)) {
            clear();
            move(7, 0);
            prints("暂时没有文章");
            pressanykey();
            free(pts);
            if (first)
                free(pc_n);
            if (pc_dirmode == 4)
                pc_fav_dir = 0;
            return -1;
        }
        clear();
        move(7, 0);
        prints("本区暂时没有文章,增加新文章");
        if (!pc_add_a_node(0)) {
            free(pts);
            if (first)
                free(pc_n);
            if (pc_dirmode == 4)
                pc_fav_dir = 0;
            return -1;
        }
        i = get_pc_nodes(pc_n, pc_u->uid, pc_fav_dir, -1, pc_dirmode - 1, 0, BBS_PAGESIZE, 0);
        group_conf.item_count = i;
    }

    if (i <= 0) {
        free(pts);
        if (first)
            free(pc_n);
        if (pc_dirmode == 4)
            pc_fav_dir = 0;
        return -1;
    }

    clear();
    list_select_loop(&group_conf);

    free(pts);
    if (first) {
        free(pc_n);
        pc_n = NULL;
    }
    //pc_dir_start=0;

    if (pc_dirmode == 4) {
        pc_fav_dir = 0;
    }

    return 0;
}

/***************************************************************
 *
 *
 *
 *
 * 以下是评论的select
 *
 *
 *
 *
 *
 ***************************************************************/

struct pc_comments *pc_c = NULL;

//int pc_com_start=0;

static int pc_can_com(int comlevel , unsigned long pcuid )
{
    if (comlevel == 0)
        return 0;
    if (comlevel == 1 && !strcmp(getCurrentUser()->userid, "guest"))
        return 0;
    if (pc_in_blacklist( getCurrentUser()->userid , pcuid ) )
    	return 0;
    return 1;
}

/*******
 * nid==0 : 增加
 * nid > 0: 修改
 *          nid为序号
 */
static int pc_add_a_com(unsigned long nid)
{
    struct pc_comments pn;
    char ans[201];
    char fpath[STRLEN];
    int ret;

    bzero(&pn, sizeof(pn));
    if (nid) {
        if (get_pc_a_com(&pn, nid) <= 0)
            return 0;
    }

    move(t_lines - 1, 0);
    clrtoeol();

    if (nid) {
        strncpy(ans, pn.subject, 200);
        ans[200] = 0;
    } else
        ans[0] = 0;
    getdata(t_lines - 1, 0, "标题:", ans, 200, DOECHO, NULL, false);
    if (!ans[0])
        return 0;
    strncpy(pn.subject, ans, 200);
    pn.subject[200] = 0;

    gettmpfilename(fpath, "pc.comments");
    unlink(fpath);

    if (nid && pn.body) {
        pc_conv_body_to_file(pn.body, fpath);
        free(pn.body);
        pn.body = NULL;
    } else
        unlink(fpath);

    if (vedit(fpath, 0, NULL, NULL, 0) == -1)
        return 0;

    pn.body = NULL;
    if (!pc_conv_file_to_body(&(pn.body), fpath)) {
        unlink(fpath);
        return 0;
    }
    unlink(fpath);

    strncpy(pn.hostname, uinfo.from, 20);
    pn.hostname[20] = 0;

    pn.changed = time(0);
    if (!nid)
        pn.created = pn.changed;
    pn.uid = pc_u->uid;
    pn.nid = pc_n[pc_now_node_ent].nid;
    strncpy(pn.username, getCurrentUser()->userid, 20);
    pn.username[20] = 0;

    ret = add_pc_comments(&pn);

    if (pn.body)
        free(pn.body);
    return ret;
}

static int pc_com_title(struct _select_def *conf)
{
    int chkmailflag;

    chkmailflag = chkmail();

    clear();
    move(0, 0);

    if (chkmailflag == 2) {
        prints("\033[0;1;5;44m                         [您的信箱超过容量,不能再收信!]                       \033[m");
    } else if (chkmailflag) {
        prints("\033[0;1;5;44m                                   [您有信件]                                 \033[m");
    } else {
        prints("\033[0;1;44m  %s的个人文集评论 -- %-42s ", pc_u->username, pc_u->corpusname);
        switch (pc_dirmode) {
        case 2:
            prints("[好友区]\033[m");
            break;
        case 3:
            prints("[私人区]\033[m");
            break;
        case 4:
            prints("[收藏区]\033[m");
            break;
        case 5:
            prints("[删除区]\033[m");
            break;
        default:
            prints("[公开区]\033[m");
            break;
        }
    }
    move(1, 0);
    prints("                       退出[\033[1;32mq\033[m] 增加[\033[1;32ma\033[m] 删除[\033[1;32md\033[m] 修改[\033[1;32me\033[m]");
    move(2, 0);
    prints("\033[0;1;44m  %-4s %-13s %-40s %-15s\033[m", "序号", "作者", "标题", "时间");
    update_endline();
    return SHOW_CONTINUE;
}

static int pc_com_show(struct _select_def *conf, int i)
{

    char newts[20];

    prints(" %-3d %-13s %-40s %-15s", i, pc_c[i - conf->page_pos].username, pc_c[i - conf->page_pos].subject, tt2timestamp(pc_c[i - conf->page_pos].created, newts));

    return SHOW_CONTINUE;
}

static int pc_com_prekey(struct _select_def *conf, int *key)
{
    switch (*key) {
    case 'q':
        *key = KEY_LEFT;
        break;
    case 'p':
    case 'k':
        *key = KEY_UP;
        break;
    case 'n':
    case 'j':
        *key = KEY_DOWN;
        break;
    }
    return SHOW_CONTINUE;
}

static int pc_com_key(struct _select_def *conf, int key)
{
    char ans[4];

    switch (key) {
    case 'a':
        if (!pc_can_com(pc_n[pc_now_node_ent].comment , pc_n[pc_now_node_ent].uid))
            return SHOW_CONTINUE;
        if (pc_add_a_com(0))
            return SHOW_DIRCHANGE;
        return SHOW_REFRESH;
        break;
    case 'd':
        if (!pc_is_admin(pc_u->username) && 
            ((strcmp(getCurrentUser()->userid, "guest")==0) ||
            strcasecmp(getCurrentUser()->userid, pc_c[conf->pos - conf->page_pos].username)))
            return SHOW_CONTINUE;

        move(2,0);
		clrtoeol();
		ans[0]=0;
        getdata(2, 0, "删除这条记录(Y/N) [N]: ", ans, 3, DOECHO, NULL, true);
		if(ans[0] != 'y' && ans[0]!='Y') {
			
        } else if (del_pc_comments(pc_n[pc_now_node_ent].nid, pc_c[conf->pos - conf->page_pos].cid)) {
            return SHOW_DIRCHANGE;
        }
        return SHOW_REFRESH;
        break;
    case 'e':
        if ((strcmp(getCurrentUser()->userid, "guest")==0) ||
            strcasecmp(getCurrentUser()->userid, pc_c[conf->pos - conf->page_pos].username))
            return SHOW_CONTINUE;
        if (pc_add_a_com(pc_c[conf->pos - conf->page_pos].cid))
            return SHOW_DIRCHANGE;
        return SHOW_REFRESH;
        break;
    case 'v':
        i_read_mail();
        return SHOW_REFRESH;
        break;
    }

    return SHOW_CONTINUE;
}

static int pc_com_getdata(struct _select_def *conf, int pos, int len)
{

    int i;

    for (i = 0; i < BBS_PAGESIZE; i++) {
        if (pc_c[i].body)
            free(pc_c[i].body);
    }

    bzero(pc_c, sizeof(struct pc_comments) * BBS_PAGESIZE);

    if (conf->item_count - conf->page_pos < BBS_PAGESIZE)
        conf->item_count = count_pc_comments(pc_n[pc_now_node_ent].nid);

    i = get_pc_comments(pc_c, pc_n[pc_now_node_ent].nid, conf->page_pos - 1, BBS_PAGESIZE, 0);

    if (i < 0)
        return SHOW_QUIT;

    if (i == 0) {

        conf->pos = 1;

        i = get_pc_comments(pc_c, pc_n[pc_now_node_ent].nid, 0, BBS_PAGESIZE, 0);

        if (i <= 0)
            return SHOW_QUIT;
    }

    return SHOW_CONTINUE;
}

static int pc_com_select(struct _select_def *conf)
{
    char fpath[STRLEN];

    clear();

        /***先显示文章正文*****/
    gettmpfilename(fpath, "pc.comments");
    unlink(fpath);
    if (!pc_conv_com_to_file(pc_c[conf->pos - conf->page_pos].cid, fpath)) {
        move(3, 0);
        prints("评论没有内容");
        pressanykey();
        return SHOW_REFRESH;
    }

    ansimore(fpath, true);

    unlink(fpath);

    return SHOW_REFRESH;
}

static int pc_read_comment()
{
    struct _select_def group_conf;
    POINT *pts;
    int i;

    if (pc_now_node_ent < 0 || pc_now_node_ent >= BBS_PAGESIZE)
        return 0;

    pc_c = (struct pc_comments *) malloc(sizeof(struct pc_comments) * BBS_PAGESIZE);
    if (pc_c == NULL)
        return 0;

    bzero(&group_conf, sizeof(struct _select_def));

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
    group_conf.pos = 1;
    group_conf.page_pos = 1;

    group_conf.show_data = pc_com_show;
    group_conf.key_command = pc_com_key;
    group_conf.get_data = pc_com_getdata;
    group_conf.show_title = pc_com_title;
    group_conf.pre_key_command = pc_com_prekey;
    group_conf.on_select = pc_com_select;

    bzero(pc_c, sizeof(struct pc_comments) * BBS_PAGESIZE);

    group_conf.item_count = count_pc_comments(pc_n[pc_now_node_ent].nid);
    i = get_pc_comments(pc_c, pc_n[pc_now_node_ent].nid, 0, BBS_PAGESIZE, 0);

    if (i < 0) {
        free(pts);
        free(pc_c);
        return -1;
    }
    if (i == 0) {
        char ans[3];

        if (!pc_can_com(pc_n[pc_now_node_ent].comment , pc_n[pc_now_node_ent].uid)) {
            clear();
            move(7, 0);
            prints("暂时没有评论");
            pressanykey();
            free(pts);
            free(pc_c);
            return -1;
        }
        clear();
        getdata(7, 0, "本文暂时没有评论,增加新评论吗? (Y/N) [N]:", ans, 3, DOECHO, NULL, true);
        if ((ans[0] != 'y' && ans[0] != 'Y') || !pc_add_a_com(0)) {
            free(pts);
            free(pc_c);
            return -1;
        }
        i = get_pc_comments(pc_c, pc_n[pc_now_node_ent].nid, 0, BBS_PAGESIZE, 0);
        group_conf.item_count = i;
    }

    if (i <= 0) {
        free(pts);
        free(pc_c);
        return -1;
    }

    clear();
    list_select_loop(&group_conf);

    free(pts);
    free(pc_c);
    pc_c = NULL;

    return 0;
}

int import_to_pc(int ent, struct fileheader *fileinfo, char *direct)
{
    struct pc_users pu;
    struct pc_nodes pn;
    char fpath[STRLEN];
    int ret;
    char buf1[512];
    char ans[4];
    char *t;

    if (!(getCurrentUser()->flags & PCORP_FLAG))
        return DONOTHING;

    bzero(&pu, sizeof(pu));
    if (get_pc_users(&pu, getCurrentUser()->userid) <= 0)
        return FULLUPDATE;

    move(t_lines - 1, 0);
    clrtoeol();
    getdata(t_lines - 1, 0, "收录到个人文集私人区，确认？(y/n) [N]", ans, 3, DOECHO, NULL, true);
    if (toupper(ans[0]) != 'Y') {
        return FULLUPDATE;
    }

    bzero(&pn, sizeof(pn));

    strncpy(pn.subject, fileinfo->title, ARTICLE_TITLE_LEN - 1);
    pn.subject[ARTICLE_TITLE_LEN - 1] = 0;

    strcpy(buf1, direct);
    if ((t = strrchr(buf1, '/')) != NULL)
        *t = '\0';
    snprintf(fpath, 512, "%s/%s", buf1, fileinfo->filename);

    pn.body = NULL;
    if (!pc_conv_file_to_body(&(pn.body), fpath)) {
        return DONOTHING;
    }

    strncpy(pn.hostname, uinfo.from, 20);
    pn.hostname[20] = 0;

    pn.created = time(0);
    pn.changed = pn.created;
    pn.uid = pu.uid;
    pn.comment = 1;
    pn.access = 3 - 1;

    ret = add_pc_nodes(&pn);

    if (pn.body)
        free(pn.body);

    move(t_lines - 1, 0);
    clrtoeol();
    if (ret)
        getdata(t_lines - 1, 0, "收录到个人文集成功，按回车继续<<", ans, 3, NOECHO, NULL, true);
    else
        getdata(t_lines - 1, 0, "收录到个人文集失败，按回车继续<<", ans, 3, NOECHO, NULL, true);

    return FULLUPDATE;
}

#endif
