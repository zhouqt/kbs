<?php

require("inc/funcs.php");
require("inc/usermanage.inc.php");
require("inc/user.inc.php");

setStat("个人参数修改");

requireLoginok();

show_nav();

showUserMailbox();
head_var($userid."的控制面板","usermanagemenu.php",0);
showUserManageMenu();
main();

show_footer();

function getOptions($var_name, $oldvalue) {
	global $$var_name;
	$userdefine = $$var_name;
	$ccc = count($userdefine);
	$flags = $oldvalue;
	for ($i = 0; $i < $ccc; $i++) {
		if (isset($_POST[$var_name.$i])) {
			if ($_POST[$var_name.$i] == 1) {
				$flags |= (1<<$i);
			} else {
				$flags &= ~(1<<$i);
			}
		}
	}
	return $flags;
}

function main(){
	global $currentuser;
	global $currentuinfo;
	$userdefine0 = getOptions("user_define", $currentuser['userdefine0']);
	$userdefine1 = getOptions("user_define1", $currentuser['userdefine1']);
	$mailbox_prop = getOptions("mailbox_prop", $currentuinfo['mailbox_prop']);
	bbs_setuserparam($userdefine0, $userdefine1, $mailbox_prop);
	setSucMsg("修改成功！");
	return html_success_quit('返回控制面板', 'usermanagemenu.php');
}
?>