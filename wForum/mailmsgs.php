<?php


require("inc/funcs.php");

require("inc/usermanage.inc.php");

require("inc/user.inc.php");

setStat("用户短信服务");

show_nav();

if ($loginok==1) {
?>
<table border="0" width="97%">
<?php
	showUserMailbox();
?>
</table>
<?php
	head_var($userid."的控制面板","usermanagemenu.php",0);
	main();
}else {
	foundErr("本页需要您以正式用户身份登陆之后才能访问！");
}

if (isErrFounded()) {
		html_error_quit();
}

show_footer();

function main() {
	bbs_mailwebmsgs();
	setSucMsg("讯息备份已经寄回您的信箱！");
	return html_success_quit();
}
?>