<?php
require("inc/funcs.php");

require("inc/usermanage.inc.php");

require("inc/user.inc.php");

setStat("撰写新邮件");

preprocess();

show_nav();

echo "<br><br>";

if (!isErrFounded()) {
	head_var($userid."的发件箱","usermailbox.php?boxname=sendbox",0);
}

if ($loginok==1) {
	showUserManageMenu();
	showmailBoxes();
	main();
}else {
	foundErr("本页需要您以正式用户身份登陆之后才能访问！");
}


if (isErrFounded()) {
		html_error_quit();
} 
show_footer();

//showBoardSampleIcons();
function preprocess(){
	global $currentuser;
	global $loginok;
	if ($loginok!=1) {
		foundErr("游客不能写信。");
		return false;
	}
    if (!bbs_can_send_mail()) {
		foundErr("您没有写信权力!");
		return false;
	}
	if (strchr($_POST['destid'], '@') || strchr($_POST['destid'], '|')
        || strchr($_POST['destid'], '&') || strchr($_POST['destid'], ';')) {
        foundErr("错误的收信人帐号");
		return false;
    }
	return true;
}

function 	main(){
	global $_POST;
	$ret=bbs_postmail($_POST['destid'],preg_replace("/\\\(['|\"|\\\])/","$1",$_POST['title']),preg_replace("/\\\(['|\"|\\\])/","$1",$_POST['content']),intval($_POST['signature']), isset($_POST['backup'])?1:0);
	switch ($ret) {
		case -1:
			foundErr("无法创建临时文件");
			return false;
		case -2:
			foundErr("发信失败:无法创建文件！");
			return false;
		case -3:
			foundErr("对方拒收你的邮件。");
			return false;
		case -4:
			foundErr("对方信箱满。");
			return false;
		case -5:
			foundErr("发信失败。");
			return false;
		case -100:
			foundErr("收信人账号错误。");
			return false;
	}
	setSucMsg("信件已成功发送！");
	return html_success_quit('返回收件箱', 'usermailbox.php?boxname=inbox');
}
?>
