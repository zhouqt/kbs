<?php
require("inc/funcs.php");
require("inc/board.inc.php");
require("inc/user.inc.php");
require("inc/conn.php");

global $boardArr;
global $boardID;
global $boardName;

preprocess();

setStat("发布小字报");

show_nav($boardName);

if (isErrFounded()) {
	echo"<br><br>";
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
	main($boardID,$boardName);
}

show_footer();

CloseDatabase();

function preprocess(){
	global $boardID;
	global $boardName;
	global $currentuser;
	global $boardArr;
	global $loginok;
	global $title;
	global $Content;
	
	$title=trim($_POST["title"]);
	$Content=trim($_POST["Content"]);

	if ($title=="") {
	    foundErr("<br><li>主题不应为空。");
		return false;
	}
	if (strlen($title)>80) {
		foundErr("<br><li>主题长度不能超过80");
		return false;
	}
	if ($Content=="") {
		foundErr("<br><li>没有填写内容。");
		return false;
	}
	if (strlen($Content)>500) {
		foundErr("<br><li>发言内容不得大于500");
		return false;
	} 
	if ($loginok!=1) {
		foundErr("游客不能发表小字报。");
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
		foundErr("指定的版面不存在。");
		return false;
	}
	$usernum = $currentuser["index"];
	if (bbs_checkreadperm($usernum, $boardID) == 0) {
		foundErr("您无权阅读本版！");
		return false;
	}
	if (bbs_is_readonly_board($boardArr)) {
			foundErr("本版为只读讨论区！");
			return false;
	}
	if (bbs_checkpostperm($usernum, $boardID) == 0) {
		foundErr("您无权在本版发表小字报！");
		return false;
	}

	return true;
}



function main($boardID,$boardName) {
	global $conn;
	global $currentuser;
	global $title;
	global $Content;

    $sql="insert into smallpaper_tb (boardID,Owner,Title,Content,Addtime) values (".$boardID.",'". $currentuser['userid']."','". htmlspecialchars($title, ENT_QUOTES)."','". htmlspecialchars($Content,ENT_QUOTES)."',now())";
	$conn->query($sql);
	setSucMsg("您成功的发布了小字报。");
  	return html_success_quit('返回版面', 'board.php?name='.$boardName);
} 

?>