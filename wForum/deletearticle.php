<?php
$needlogin=1;

require("inc/funcs.php");
require("inc/user.inc.php");
require("inc/board.inc.php");

global $boardArr;
global $boardID;
global $boardName;
global $reID;
global $reArticles;

preprocess();

setStat("删除文章");

show_nav();

if (isErrFounded()) {
	html_error_quit() ;
} else {
	?>
	<br>
	<TABLE cellSpacing=0 cellPadding=0 width=97% border=0 align=center>
	<?php

	if ($loginok==1) {
		showUserMailbox();
?>
</table>
<?php
	}

	board_head_var($boardArr['DESC'],$boardName,$boardArr['SECNUM']);

	doPostAritcles($boardID,$boardName,$boardArr,$reID,$reArticles);

	if (isErrFounded()) {
		html_error_quit() ;
	}
}

//showBoardSampleIcons();
show_footer();

function preprocess(){
	global $boardID;
	global $boardName;
	global $currentuser;
	global $boardArr;
	global $loginok;
	global $reID;
	global $reArticles;
	if ($loginok!=1) {
		foundErr("游客不能删除文章。");
		return false;
	}
	if (!isset($_GET['board'])) {
		foundErr("未指定版面。");
		return false;
	}
	$boardName=$_GET['board'];
	$brdArr=array();
	$boardID= bbs_getboard($boardName,$brdArr);
	$boardArr=$brdArr;
	if ($boardID==0) {
		foundErr("指定的版面不存在");
		return false;
	}
	$usernum = $currentuser["index"];
	if (bbs_checkreadperm($usernum, $boardID) == 0) {
		foundErr("您无权阅读本版");
		return false;
	}
	if (bbs_is_readonly_board($boardArr)) {
			foundErr("本版为只读讨论区！");
			return false;
	}
	if (bbs_checkpostperm($usernum, $boardID) == 0) {
		foundErr("您无权阅读本版");
		return false;
	}
	if (isset($_GET["ID"])) {
		$reID = $_GET["ID"];
	}else {
		foundErr("未指定编辑的文章.");
		return false;
	}
	settype($reID, "integer");
	$articles = array();
	if ($reID > 0)	{
	$num = bbs_get_records_from_id($boardName, $reID,$dir_modes["NORMAL"],$articles);
		if ($num == 0)	{
			foundErr("错误的文章文编号");
			return false;
		}
	}
	$reArticles=$articles;
	return true;
}

function 	doPostAritcles($boardID,$boardName,$boardArr,$reID,$reArticles){
	$ret=bbs_delfile($boardName,$reArticles[1]['FILENAME']);
	switch ($ret) {
		case -1:
			foundErr("您无权删除该文。");
			return false;
		case -2:
			foundErr("错误的版名或者文件名!");
			return false;

	}
?>
<table cellpadding=3 cellspacing=1 align=center class=tableborder1>
<tr align=center><th width="100%">文章删除成功</td>
</tr><tr><td width="100%" class=tablebody1>
本页面将在3秒后自动返回版面文章列表<meta HTTP-EQUIV=REFRESH CONTENT='3; URL=board.php?name=<?php echo $boardName; ?>' >，<b>您可以选择以下操作：</b><br><ul>
<li><a href="index.php">返回首页</a></li>
<li><a href="board.php?name=<?php   echo $boardName; ?>">返回<?php   echo $boardArr['DESC']; ?></a></li>
</ul></td></tr></table>
<?php
}
?>