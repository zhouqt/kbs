#include "bbslib.h"

int main() {
	int stay, pid;
	uinfo_t *ui;

	init_all();
	if(!loginok) http_fatal("ÄãÃ»ÓÐµÇÂ¼");
	ui = getcurruinfo();
	pid = ui->pid;
	if(pid>0) kill(pid, SIGHUP);
	setcookie("utmpkey", "");
	setcookie("utmpnum", "");
	setcookie("utmpuserid", "");
	setcookie("my_t_lines", "");
	setcookie("my_link_mode", "");
	setcookie("my_def_mode", "");
	redirect(FIRST_PAGE); 
}
