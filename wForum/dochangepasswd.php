<?php
require("inc/funcs.php");

require("inc/usermanage.inc.php");

require("inc/user.inc.php");

setStat("修改密码");

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
	main();
}else {
	foundErr("本页需要您以正式用户身份登陆之后才能访问！");
}

if (isErrFounded()) {
		html_error_quit();
} else {
		showUserManageMenu();
		html_success_quit();
}

show_footer();

function main() {
	global $currentuser;
	$pw1=trim($_POST['oldpsw']);
	$pw2=trim($_POST['psw']);
	$pw3=trim($_POST['psw2']);
    if (strcmp($pw2, $pw3)) {
		foundErr("两次密码不相同");
		return false;
	}
    if (strlen($pw2) < 2) {
        foundErr("新密码太短");
		return false;
	}
    if (bbs_checkuserpasswd($currentuser['userid'], $pw1)) {
        foundErr("旧密码不正确");
		return false;
	}
	$ret=bbs_setuserpasswd($currentuser['userid'], $pw2);
	if ($ret!=0) {
		foundErr("更新密码失败！");
		return 0;
	}
?>

<?php

	return setSucMsg("更新密码成功！");
}

?>