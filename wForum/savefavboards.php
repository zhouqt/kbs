<?php
require("inc/funcs.php");

require("inc/usermanage.inc.php");

require("inc/user.inc.php");

setStat("保存收藏版面");

show_nav();

if ($loginok==1) {
	showUserMailbox();
	head_var($userid."的控制面板","usermanagemenu.php",0);
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
	if (isset($_GET["select"]))
		$select = $_GET["select"];
	else
		$select = 0;
	settype($select, "integer");

/*
	这个地方先用最丑陋的代码，需要调用PHP函数n次，保存fav文件n次。
	下面这个是理想化的代码，但是PHP函数传参array的用法太繁，懒得写... - atppp
	$boards = array();
	foreach($_POST as $board => $value) {
		if ($value == 1) $boards[] = $board;
	}
	bbs_set_favboards($select, $boards);
*/
	if (bbs_load_favboard($select) == -1) {
		foundErr("无法读取收藏夹");
		return false;
	}
	$boards = bbs_fav_boards($select, 1);
	if ($boards == FALSE) {
		foundErr("无法读取收藏夹");
		return false;
	}
	$brd_flag= $boards["FLAG"];
	$brd_npos= $boards["NPOS"];
	$rows = sizeof($brd_flag);
	for ($i = 0; $i < $rows; $i++) {
		if ($brd_flag[$i] == -1 ) continue;
		bbs_del_favboard($select,$brd_npos[$i]);
	}
	foreach($_POST as $board => $value) {
		if ($value == 1) bbs_add_favboard($board);
	}
	setSucMsg("修改成功！");
	return html_success_quit('返回收藏版面', 'favboard.php?select='.$select);
}
?>