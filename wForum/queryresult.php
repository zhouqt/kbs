<?php
require("inc/funcs.php");

require("inc/board.inc.php");


global $boardName;
global $boardArr;
global $boardID;

setStat("搜索结果");

show_nav();

preprocess();

if ($loginok==1) {
?>
<table border="0" width="97%">
<?php
	showUserMailbox();
?>
</table>
<?php
} else {
	echo "<br><br>";
}

if ($boardName!='') 
	board_head_var($boardArr['DESC'],$boardName,$boardArr['SECNUM']);
else {
	head_var("论坛搜索",'',0);
}

if (isErrFounded()) {
		html_error_quit();
} else {
	doSearch($boardName);
}

show_footer();

function preprocess(){
	global $boardID;
	global $boardName;
	global $currentuser;
	global $boardArr;
	if (!isset($_GET['boardName'])) {
		foundErr("未指定版面。");
		return false;
	}
	$boardName=$_GET['boardName'];
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
	return true;
}

function showSearchMenu(){

}
?>
