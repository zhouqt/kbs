<?php

require("inc/funcs.php");
require("inc/usermanage.inc.php");
require("inc/user.inc.php");

setStat("撰写新邮件");

requireLoginok("游客不能写信。");

preprocess();

show_nav();

echo "<br>";
head_var($userid."的发件箱","usermailbox.php?boxname=sendbox",0);
showUserManageMenu();
showmailBoxes();
main();

show_footer();

//showBoardSampleIcons();
function preprocess(){
	global $currentuser;
	global $loginok;
    if (!bbs_can_send_mail()) {
		foundErr("您没有写信权力!");
	}
	if (strchr($_POST['destid'], '@') || strchr($_POST['destid'], '|')
        || strchr($_POST['destid'], '&') || strchr($_POST['destid'], ';')) {
        foundErr("错误的收信人帐号");
    }
	return true;
}

function main() {
	global $_POST;
	$ret=bbs_postmail($_POST['destid'],preg_replace("/\\\(['|\"|\\\])/","$1",$_POST['title']),preg_replace("/\\\(['|\"|\\\])/","$1",$_POST['content']),intval($_POST['signature']), isset($_POST['backup'])?1:0);
	switch ($ret) {
		case -1:
			foundErr("无法创建临时文件");
		case -2:
			foundErr("发信失败:无法创建文件！");
		case -3:
			foundErr("对方拒收你的邮件。");
		case -4:
			foundErr("对方信箱满。");
		case -5:
			foundErr("发信失败。");
		case -100:
			foundErr("收信人账号错误。");
	}
	setSucMsg("信件已成功发送！");
	return html_success_quit('返回收件箱', 'usermailbox.php?boxname=inbox');
}
?>
