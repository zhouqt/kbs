/*
 * $Id$
 */
#include "bbslib.h"
void
setflags(mask, value)
int mask, value;
{
	struct userec *u;

	u = getcurrusr();
    if (((u->flags[0] & mask) && 1) != value) {
        if (value) u->flags[0] |= mask;
        else u->flags[0] &= ~mask;
    }
}

void
u_exit()
{
	uinfo_t *ui;

	ui = getcurruinfo();
/*---	Added by period		2000-11-19	sure of this	---*/
    if(!ui->active) return;
/*---		---*/
    setflags(PAGER_FLAG, (ui->pager&ALL_PAGER));
/*    if (HAS_PERM(currentuser,PERM_LOGINCLOAK)&&HAS_PERM(currentuser,PERM_SEECLOAK))*/

   /* Bigman 2000.8.29 智囊团能够隐身 */
	if((HAS_PERM(currentuser,PERM_CHATCLOAK) || HAS_PERM(currentuser,PERM_CLOAK)))
        setflags(CLOAK_FLAG, ui->invisible);

	/*if (delfrom_msglist(get_utmpent_num(ui), getcurruserid()) < 0)
		http_fatal("无法从消息列表中删除当前用户");*/

    clear_utmp(get_utmpent_num(ui),ui->uid);
}

void abort_program()
{
	int stay=0;
	struct userec *x = NULL;

	stay = abs(time(0) - u_info->logintime);
	/* 上站时间超过 2 小时按 2 小时计 */
	if(stay>7200)
		stay = 7200;
	/*getuser(getcurruserid(), &x);*/
	x = getcurrusr();
	if(x)
	{
		x->stay += stay;
		record_exit_time();
		bbslog( "1system", "EXIT: Stay:%3ld (%s)[%d %d]", stay / 60, 
			x->username, get_curr_utmpent(), getusernum(x->userid));
		u_exit();
		/*save_user_data(x);*/
	}
}

/*void debug_abort(int signo)
{
	abort();
}*/

int main() {
	int stay, pid;

	init_all();
	if(!loginok) http_fatal("你没有登录");
	abort_program();
	/*
	setcookie("UTMPKEY", "");
	setcookie("UTMPNUM", "");
	setcookie("UTMPUSERID", "");
	*/
	setcookie("my_t_lines", "");
	setcookie("my_link_mode", "");
	setcookie("my_def_mode", "");
	redirect(FIRST_PAGE); 
}
