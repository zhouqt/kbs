<?php

require("inc/funcs.php");
require("inc/user.inc.php");
require("inc/board.inc.php");

global $boardArr;
global $boardID;
global $boardName;
global $reID;
global $reArticles;

setStat("编辑文章");

requireLoginok("游客不能编辑文章。");

preprocess();

show_nav();

showUserMailBoxOrBR();
board_head_var($boardArr['DESC'],$boardName,$boardArr['SECNUM']);
doPostAritcles($boardID,$boardName,$boardArr,$reID,$reArticles);

show_footer();

function preprocess(){
	global $boardID;
	global $boardName;
	global $currentuser;
	global $boardArr;
	global $reID;
	global $reArticles;
	if (!isset($_POST['board'])) {
		foundErr("未指定版面。");
	}
	$boardName=$_POST['board'];
	$brdArr=array();
	$boardID= bbs_getboard($boardName,$brdArr);
	$boardArr=$brdArr;
	$boardName=$brdArr['NAME'];
	if ($boardID==0) {
		foundErr("指定的版面不存在");
	}
	$usernum = $currentuser["index"];
	if (bbs_checkreadperm($usernum, $boardID) == 0) {
		foundErr("您无权阅读本版");
	}
	if (bbs_is_readonly_board($boardArr)) {
		foundErr("本版为只读讨论区！");
	}
	if (bbs_checkpostperm($usernum, $boardID) == 0) {
		foundErr("您无权阅读本版");
	}
	if (!isset($_POST['Content'])) {
		foundErr("没有指定文章内容！");
	}
	if (isset($_POST["reID"])) {
		$reID = $_POST["reID"];
	} else {
		foundErr("未指定编辑的文章.");
	}
	settype($reID, "integer");
	$articles = array();
	if ($reID > 0)	{
	$num = bbs_get_records_from_id($boardName, $reID,$dir_modes["NORMAL"],$articles);
		if ($num == 0)	{
			foundErr("错误的文章文编号");
		}
	}
	$ret=bbs_caneditfile($boardName,$articles[1]['FILENAME']);
	switch ($ret) {
	case -1:
		foundErr("讨论区名称错误");
	case -2:
		foundErr("本版不能修改文章");
	case -3:
		foundErr("本版已被设置只读");
	case -4:
		foundErr("无法取得文件记录");
	case -5:
		foundErr("不能修改他人文章!");
	case -6:
		foundErr("同名ID不能修改老ID的文章");
	case -7:
		foundErr("您的POST权被封");
	}
	$reArticles=$articles;
	return true;
}

function doPostAritcles($boardID,$boardName,$boardArr,$reID,$reArticles){
	$ret=bbs_updatearticle($boardName,$reArticles[1]['FILENAME'],preg_replace("/\\\(['|\"|\\\])/","$1",$_POST['Content']));
	switch ($ret) {
		case -1:
			foundErr("修改文章失败，文章可能含有不恰当内容");
		case -10:
			foundErr("找不到文件!");
	}
?>
<table cellpadding=3 cellspacing=1 align=center class=TableBorder1>
<tr align=center><th width="100%">文章编辑成功</td>
</tr><tr><td width="100%" class=TableBody1>
本页面将在3秒后自动返回版面文章列表<meta HTTP-EQUIV=REFRESH CONTENT='3; URL=board.php?name=<?php echo $boardName; ?>' >，<b>您可以选择以下操作：</b><br><ul>
<li><a href="index.php">返回首页</a></li>
<li><a href="board.php?name=<?php   echo $boardName; ?>">返回<?php   echo $boardArr['DESC']; ?></a></li>
</ul></td></tr></table>
<?php
}
?>
