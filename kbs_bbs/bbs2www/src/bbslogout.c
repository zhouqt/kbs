/*
 * $Id$
 */
#include "bbslib.h"

int main()
{
    int stay, pid;

    init_all();
    if (!loginok)
        http_fatal("ÄãÃ»ÓÐµÇÂ¼");
    if (getcurrusr())
        www_user_logoff(getcurrusr(), getusernum(getcurrusr()->userid), u_info, get_curr_utmpent());
    /*
       abort_program();
       setcookie("UTMPKEY", "");
       setcookie("UTMPNUM", "");
       setcookie("UTMPUSERID", "");
     */
    setcookie("my_t_lines", "");
    setcookie("my_link_mode", "");
    setcookie("my_def_mode", "");
    redirect(FIRST_PAGE);
}
