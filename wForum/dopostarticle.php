<?php

require("inc/funcs.php");
require("inc/user.inc.php");
require("inc/board.inc.php");

global $boardArr;
global $boardID;
global $boardName;
global $reID;

setStat("发表文章");

requireLoginok("游客不能发表文章。");

preprocess();

show_nav(false);
showUserMailBoxOrBR();
board_head_var($boardArr['DESC'],$boardName,$boardArr['SECNUM']);
doPostAritcles($boardID,$boardName,$boardArr,$reID);

show_footer();

function preprocess(){
	global $boardID;
	global $boardName;
	global $currentuser;
	global $boardArr;
	global $loginok;
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
	if (!isset($_POST['subject'])) {
		foundErr("没有指定文章标题！");
	}
	if (!isset($_POST['Content'])) {
		foundErr("没有指定文章内容！");
	}
	if (isset($_POST["reID"])) {
		$reID = $_POST["reID"];
	} else {
		$reID = 0;
	}
	settype($reID, "integer");
	$articles = array();
	if ($reID > 0)	{
	$num = bbs_get_records_from_id($boardName, $reID,$dir_modes["NORMAL"],$articles);
		if ($num == 0)	{
			foundErr("错误的 Re 文编号");
		}
		if ($articles[1]["FLAGS"][2] == 'y') {
			foundErr("该文不可回复!");
		}
	}
	return true;
}

function doPostAritcles($boardID,$boardName,$boardArr,$reID){
	global $_POST;
	if (bbs_is_outgo_board($boardArr)) $outgo = intval($_POST["outgo"]);
	else $outgo = 0;
	$ret=bbs_postarticle($boardName,preg_replace("/\\\(['|\"|\\\])/","$1",trim($_POST['subject'])),
	                     preg_replace("/\\\(['|\"|\\\])/","$1",$_POST['Content']),
	                     intval($_POST['signature']), $reID,$outgo,intval($_POST['anonymous']),
	                     ($reID>0)?0:intval($_POST['emailflag']),SUPPORT_TEX?intval($_POST['texflag']):0);
	switch ($ret) {
		case -1:
			foundErr("错误的讨论区名称。");
		case -2:
			foundErr("本版为二级目录版！");
		case -3:
			foundErr("标题为空。");
		case -4:
			foundErr("此讨论区是唯读的, 或是您尚无权限在此发表文章。");
		case -5:
			foundErr("很抱歉, 你被版务人员停止了本版的post权力。");
		case -6:
			foundErr("两次发文间隔过密, 请休息几秒后再试。");
		case -7:
			foundErr("无法读取索引文件！请迅速通知站务人员，谢谢！");
		case -8:
			foundErr("本文不可回！");
		case -9:
			foundErr("系统内部错误！请迅速通知站务人员，谢谢！");
	}
?>
<table cellpadding=3 cellspacing=1 align=center class=TableBorder1 style="width: 75%;">
<tr align=center><th width="100%">发文成功！</td>
</tr><tr><td width="100%" class=TableBody1>
本页面将在3秒后自动返回版面文章列表<meta HTTP-EQUIV=REFRESH CONTENT='3; URL=board.php?name=<?php echo $boardName; ?>' >，<b>您可以选择以下操作：</b><br><ul>
<li><a href="index.php">返回首页</a></li>
<li><a href="board.php?name=<?php   echo $boardName; ?>">返回<?php   echo $boardArr['DESC']; ?></a></li>
</ul></td></tr></table>
<?php
}
?>
