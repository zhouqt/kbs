<?php

require("inc/funcs.php");

global $boardName;

setStat("清除未读");

requireLoginok();

preprocess();

bbs_brcclear($boardName);

if (!isset($_SERVER["HTTP_REFERER"]) || ( $_SERVER["HTTP_REFERER"]=="") )
{
	header("Location: index.php");
} else {
	header("Location: ".$_SERVER["HTTP_REFERER"]);
} 

function preprocess() {
	global $boardName;
	global $currentuser;
	if (!isset($_GET['boardName'])) {
		foundErr("未指定版面。");
	}
	$boardName = $_GET['boardName'];
	$brdArr = array();
	$boardID = bbs_getboard($boardName, $brdArr);
	$boardArr = $brdArr;
	$boardName = $brdArr['NAME'];
	if ($boardID == 0) {
		foundErr("指定的版面不存在。");
	}
	$usernum = $currentuser["index"];
	if (bbs_checkreadperm($usernum, $boardID) == 0) {
		foundErr("您无权阅读本版。");
	}
	if ($brdArr["FLAG"] & BBS_BOARD_GROUP) {
		foundErr("这是目录版面。");
    }
}
?>