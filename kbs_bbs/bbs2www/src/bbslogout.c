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
/*    if (HAS_PERM(PERM_LOGINCLOAK)&&HAS_PERM(PERM_SEECLOAK))*/

   /* Bigman 2000.8.29 智囊团能够隐身 */
	if((has_perm(PERM_CHATCLOAK) || has_perm(PERM_CLOAK)))
        setflags(CLOAK_FLAG, ui->invisible);

    clear_utmp2(ui);
}

void abort_program() {
	int stay=0;
	struct userec *x = NULL;

	stay=abs(time(0) - *(int*)(u_info->from+32));
	if(stay>7200) stay = 7200;
	getuser(getcurruserid(), &x);
	if(x) {
			x->stay+=stay;
			record_exit_time();
			u_exit();
			save_user_data(x);
	}
}

int main() {
	int stay, pid;
//	uinfo_t *ui;

	init_all();
	if(!loginok) http_fatal("你没有登录");
//	ui = getcurruinfo();
//	pid = ui->pid;
//	if(pid>1) kill(pid, SIGHUP);
	abort_program();
	setcookie("utmpkey", "");
	setcookie("utmpnum", "");
	setcookie("utmpuserid", "");
	setcookie("my_t_lines", "");
	setcookie("my_link_mode", "");
	setcookie("my_def_mode", "");
	redirect(FIRST_PAGE); 
}
