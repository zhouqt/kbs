<?php

require("inc/funcs.php");
require("inc/user.inc.php");
require("inc/board.inc.php");

global $boardArr;
global $boardID;
global $boardName;
global $reID;
global $reArticles;

setStat("删除文章");

requireLoginok("游客不能删除文章。");

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
	if (!isset($_GET['board'])) {
		foundErr("未指定版面。");
	}
	$boardName=$_GET['board'];
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
	if (isset($_GET["ID"])) {
		$reID = $_GET["ID"];
	}else {
		foundErr("未指定删除的文章.");
	}
	settype($reID, "integer");
	$articles = array();
	if ($reID > 0)	{
	$num = bbs_get_records_from_id($boardName, $reID,$dir_modes["NORMAL"],$articles);
		if ($num == 0)	{
			foundErr("错误的文章文编号");
		}
	}
	$reArticles=$articles;
	return true;
}

function doPostAritcles($boardID,$boardName,$boardArr,$reID,$reArticles){
	$ret=bbs_delfile($boardName,$reArticles[1]['FILENAME']);
	switch ($ret) {
		case -1:
			foundErr("您无权删除该文。");
		case -2:
			foundErr("错误的版名或者文件名!");
	}
?>
<table cellpadding=3 cellspacing=1 align=center class=TableBorder1>
<tr align=center><th width="100%">文章删除成功</td>
</tr><tr><td width="100%" class=TableBody1>
本页面将在3秒后自动返回版面文章列表<meta HTTP-EQUIV=REFRESH CONTENT='3; URL=board.php?name=<?php echo $boardName; ?>' >，<b>您可以选择以下操作：</b><br><ul>
<li><a href="index.php">返回首页</a></li>
<li><a href="board.php?name=<?php   echo $boardName; ?>">返回<?php   echo $boardArr['DESC']; ?></a></li>
</ul></td></tr></table>
<?php
}
?>
