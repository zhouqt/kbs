<?php

require("inc/funcs.php");
require("inc/usermanage.inc.php");
require("inc/user.inc.php");

setStat("用户短信服务");

requireLoginok();

show_nav();

showUserMailbox();
head_var($userid."的控制面板","usermanagemenu.php",0);
main();

show_footer();

function main() {
	bbs_mailwebmsgs();
	setSucMsg("讯息备份已经寄回您的信箱！");
	return html_success_quit();
}
?>