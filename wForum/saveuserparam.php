<?php
require("inc/funcs.php");

require("inc/usermanage.inc.php");

require("inc/user.inc.php");

setStat("个人参数修改");

show_nav();

if ($loginok==1) {
?>
<table border="0" width="97%">
<?php
	showUserMailbox();
?>
</table>
<?php
}

head_var($userid."的控制面板","usermanagemenu.php",0);

if ($loginok==1) {
	showUserManageMenu();
	main();
}else {
	foundErr("本页需要您以正式用户身份登陆之后才能访问！");
}

if (isErrFounded()) {
		html_error_quit();
} 

show_footer();

function main(){
	global $currentuser;
	global $user_define_num;
	$flags=0;
	for ($i=0;$i<$user_define_num;$i++) {
		if ($_POST['param'.$i]==1) {
			$flags|= (1<<$i);
		}
	}
	bbs_setuserparam($flags);
	setSucMsg("修改成功！");
	return html_success_quit('返回控制面板', 'usermanagemenu.php');
}
?>