<?php


if ( (!isset($_COOKIE['iscookies'])) || ($_COOKIE['iscookies']==''))
{
	setcookie('iscookies','0',time()+3650*24*3600);
	print '<META http-equiv=Content-Type content=text/html; charset=gb2312><meta HTTP-EQUIV=REFRESH CONTENT=3>正在登陆论坛……<br><br>本系统要求使用COOKIES，假如您的浏览器禁用COOKIES，您将不能登录本系统……';
	exit();
} 

$needlogin=1;

require("inc/funcs.php");
require("inc/user.inc.php");
require("inc/board.inc.php");
require("inc/conn.php");

global $boardArr;
global $boardID;
global $boardName;
global $page;

preprocess();



setStat("文章列表");

show_nav($boardName);

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
?>
	<TABLE cellSpacing=0 cellPadding=0 width=97% border=0 align=center>
<?php
	showAnnounce(); 
?>
</TABLE>
<script src="inc/loadThread.js"></script>
<iframe width=0 height=0 src="" id="hiddenframe"></iframe>

<?php
	showBoardStaticsTop($boardArr);
?>
<TABLE cellPadding=1 cellSpacing=1 class=tableborder1 align=center>
<?php
	showBroadcast($boardID,$boardName);

	showBoardContents($boardID,$boardName,$page);

	boardSearchAndJump($boardName, $boardID);

	showBoardSampleIcons();
?>
</table>
<?php
}

//showBoardSampleIcons();
show_footer();

CloseDatabase();

function preprocess(){
	global $boardID;
	global $boardName;
	global $currentuser;
	global $boardArr;
	global $page;
	if (!isset($_GET['name'])) {
		foundErr("未指定版面。");
		return false;
	}
	$boardName=$_GET['name'];
	$brdArr=array();
	$boardID= bbs_getboard($boardName, $brdArr);
	$boardArr=$brdArr;
	$boardName=$brdArr['NAME'];
	if ($boardID==0) {
		foundErr("指定的版面不存在");
		return false;
	}
	$usernum = $currentuser["index"];
	if (bbs_checkreadperm($usernum, $boardID) == 0) {
		foundErr("您无权阅读本版");
		return false;
	}
	if (!isset($_GET['page'])) {
		$page=-1;
	} else {
		$page=intval($_GET['page']);
	}
	bbs_set_onboard($boardID,1);
	return true;
}
?>
