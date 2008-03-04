<?php

require_once('www2-funcs.php');
require_once('www2-board.php');
login_init();
bbs_session_modify_user_mode(BBS_MODE_CSIE_ANNOUNCE);

if (isset($_GET["p"])) {
	$numpath = $_GET["p"];

	$brdarr = array();
	$bid = $numpath;
	settype($bid,"integer");
	if( $bid == 0 ){
		html_error_quit("错误的版面");
	}

	$board = bbs_getbname($bid);
	if( !$board ){
		html_error_quit("错误的讨论区");
	}
	if( $bid != bbs_getboard($board, $brdarr) ){
		html_error_quit("错误的讨论区");
	}

	$board = $brdarr['NAME'];

	$path = bbs_ann_num2path($numpath,$currentuser["userid"]);
	if($path==false){
		html_error_quit("错误的文章");
	}
	$path = substr($path, 10, strlen($path) - 9);
}
else if (isset($_GET['path']))
	$path = trim($_GET['path']);
else 
	$path = "";

if (strstr($path, '.Names') || strstr($path, '..') || strstr($path, 'SYSHome'))
	html_error_quit('不存在该目录');

if($path=="")
	$filename="0Announce/";
else if($path[0]=='/')
	$filename="0Announce".$path;
else
	$filename="0Announce/".$path;
$has_perm_boards = false;
if(bbs_ann_traverse_check($filename, $currentuser["userid"]) == 1)
	$has_perm_boards = true;
if(!$has_perm_boards)
	html_error_quit("没有权限。");
?>
