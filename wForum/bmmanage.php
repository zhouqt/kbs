<?php
/* 还不能工作，先放个基本模版 - atppp */
/* maybe this array should be put into .inc later... - atppp */
$bbsman_modes = array(
    "DEL"   => 1,
    "MARK"  => 2,
    "DIGEST"=> 3,
    "NOREPLY" => 4,
    "ZHIDING" => 5
    );

require("inc/funcs.php");
require("inc/user.inc.php");
require("inc/board.inc.php");

global $boardArr;
global $boardID;
global $boardName;
global $reID;
global $reArticles;

setStat("管理文章");

requireLoginok("游客不能切换文章。");

preprocess();

show_nav();

showUserMailBoxOrBR();
board_head_var($boardArr['DESC'],$boardName,$boardArr['SECNUM']);
doSwitchAritcles($boardID,$boardName,$boardArr,$reID,$reArticles);

show_footer();

function preprocess(){
	global $boardID;
	global $boardName;
	global $currentuser;
	global $boardArr;
	global $loginok;
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
	if (isset($_GET["ID"])) {
		$reID = $_GET["ID"];
	}else {
		foundErr("未指定切换的文章.");
	}
	settype($reID, "integer");
	$articles = array();
	if ($reID > 0)	{
	$num = bbs_get_records_from_id($boardName, $reID,$dir_modes["NORMAL"],$articles);
		if ($num == 0)	{
			foundErr("错误的文章编号");
		}
	}
	$reArticles=$articles;
	return true;
}

function doSwitchAritcles($boardID,$boardName,$boardArr,$reID,$reArticles){
	global $bbsman_modes;
	$ret=bbs_bmmanage($boardName,$reID,$bbsman_modes["MARK"],0);
	switch ($ret) {
        case -2:
            foundErr('您无权切换精华');
        case -1:
        case -3:
        case -9:
            foundErr('系统错误'.$ret);
        case -4:
            foundErr('文章ID错误');
        default:  

	}
?>
<table cellpadding=3 cellspacing=1 align=center class=TableBorder1>
<tr align=center><th width="100%">文章切换成功</td>
</tr><tr><td width="100%" class=TableBody1>
本页面将在3秒后自动返回该文章<meta HTTP-EQUIV=REFRESH CONTENT='3; URL=disparticle.php?boardName=<?php echo $boardName; ?>&ID=<?php echo $reID; ?>' >，<b>您可以选择以下操作：</b><br><ul>
<li><a href="index.php">返回首页</a></li>
<li><a href="board.php?name=<?php   echo $boardName; ?>">返回<?php   echo $boardArr['DESC']; ?></a></li>
<li><a href="disparticle.php?boardName=<?php echo $boardName; ?>&ID=<?php echo $reID; ?>">返回该文章</a></li>
</ul></td></tr></table>
<?php
}
?>
