#include "bbs.h"

#undef printf
#undef perror
#undef time
#undef fprintf

int login(struct userec *user)
{
    char buf[80];
    int tmp;
    struct user_info *u;
    struct user_info ui;
    int utmpent;

    memset(&ui, 0, sizeof(struct user_info));
    ui.active = true;

    /* Bigman 2000.8.29 智囊团能够隐身 */
    if ((HAS_PERM(user, PERM_CHATCLOAK) || HAS_PERM(user, PERM_CLOAK)) && (user->flags & CLOAK_FLAG))
        ui.invisible = true;
    ui.pager = 0;
    if (DEFINE(user, DEF_FRIENDCALL)) {
        ui.pager |= FRIEND_PAGER;
    }
    if (user->flags & PAGER_FLAG) {
        ui.pager |= ALL_PAGER;
        ui.pager |= FRIEND_PAGER;
    }
    if (DEFINE(user, DEF_FRIENDMSG)) {
        ui.pager |= FRIENDMSG_PAGER;
    }
    if (DEFINE(user, DEF_ALLMSG)) {
        ui.pager |= ALLMSG_PAGER;
        ui.pager |= FRIENDMSG_PAGER;
    }
    ui.uid = searchuser(user->userid);
    strncpy(ui.from, getSession()->fromhost, IPLEN);
    ui.logintime = time(0);     /* for counting user's stay time */
    /* refer to bbsfoot.c for details */
	ui.freshtime = time(0);
    ui.mode = WEBEXPLORE;
    strncpy(ui.userid, user->userid, 20);
	{
		struct userdata ud;

		read_userdata(user->userid, &ud);
    	strncpy(ui.realname, ud.realname, 20);
	}
    strncpy(ui.username, user->username, 40);
    utmpent = getnewutmpent2(&ui, 1);
    if (utmpent == -1)
	{
        fprintf(stderr, "抱歉，目前在线用户数已达上限，无法登录。");
		exit(-1);
	}
    /*u = get_user_info(utmpent);*/
	u = &(utmpshm->uinfo[utmpent-1]);
    u->pid = 1;
    tmp = rand() % 100000000;
    u->utmpkey = tmp;
    getfriendstr(user,u, getSession());
    /*setcurruinfo(u);*/
	/*u_info = ui;*/
    /*if (addto_msglist(get_utmpent_num(getcurruinfo()), getcurruserid()) < 0)
        http_fatal("无法添加当前用户到消息列表中");*/
    sprintf(buf, "%d", utmpent);
    printf("UTMPNUM = %s\n", buf);
    sprintf(buf, "%d", tmp);
    printf("UTMPKEY = %s\n", buf);
    printf("UTMPUSERID = %s\n", user->userid);
    return 0;
}

int main(int argc, char **argv)
{
	struct userec *x = NULL;

	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s <username>\n", argv[0]);
		exit(-1);
	}

    if (init_all()) {
        printf("init data fail\n");
        return -1;
    }

	getuser(argv[1], &x);
	if (x == NULL)
	{
		fprintf(stderr, "%s not found.\n", argv[1]);
		exit(-1);
	}
	setCurrentUser(x);
	login(x);

	return 0;
}

