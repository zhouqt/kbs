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

setStat("编辑文章");

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
		foundErr("游客不能发表文章。");
		return false;
	}
	if (!isset($_POST['board'])) {
		foundErr("未指定版面。");
		return false;
	}
	$boardName=$_POST['board'];
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
	if (!isset($_POST['Content'])) {
		foundErr("没有指定文章内容！");
		return false;
	}
	if (isset($_POST["reID"])) {
		$reID = $_POST["reID"];
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
	$ret=bbs_caneditfile($boardName,$articles[1]['FILENAME']);
	switch ($ret) {
	case -1:
		foundErr("讨论区名称错误");
		return false;
	case -2:
		foundErr("本版不能修改文章");
		return false;
	case -3:
		foundErr("本版已被设置只读");
		return false;
	case -4:
		foundErr("无法取得文件记录");
		return false;
	case -5:
		foundErr("不能修改他人文章!");
		return false;
	case -6:
		foundErr("同名ID不能修改老ID的文章");
		return false;
	case -7:
		foundErr("您的POST权被封");
		return false;
	}
	$reArticles=$articles;
	return true;
}

function 	doPostAritcles($boardID,$boardName,$boardArr,$reID,$reArticles){
	global $_POST;
	$ret=bbs_updatearticle($boardName,$reArticles[1]['FILENAME'],preg_replace("/\\\(['|\"|\\\])/","$1",$_POST['Content']));
	switch ($ret) {
		case -1:
			foundErr("修改文章失败，文章可能含有不恰当内容");
			return false;
		case -10:
			foundErr("找不到文件!");
			return false;

	}
?>
<table cellpadding=3 cellspacing=1 align=center class=tableborder1>
<tr align=center><th width="100%">文章编辑成功</td>
</tr><tr><td width="100%" class=tablebody1>
本页面将在3秒后自动返回版面文章列表<meta HTTP-EQUIV=REFRESH CONTENT='3; URL=board.php?name=<?php echo $boardName; ?>' >，<b>您可以选择以下操作：</b><br><ul>
<li><a href="index.php">返回首页</a></li>
<li><a href="board.php?name=<?php   echo $boardName; ?>">返回<?php   echo $boardArr['DESC']; ?></a></li>
</ul></td></tr></table>
<?php
}
?>