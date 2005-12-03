<?php
function bbs_clear_donothing() {
	header('Location: /'); /* TODO */
}

require("www2-funcs.php");
login_init();

if (!strcmp($currentuser['userid'],'guest') )
	bbs_clear_donothing();

if (isset($_GET['board']))
	$board = $_GET['board'];
elseif (isset($_POST['board']))
	$board = $_POST['board'];
else
	bbs_clear_donothing();
		
$brdarr = array();
$bid = bbs_getboard($board,$brdarr);
if ($bid == 0)
	bbs_clear_donothing();
$board = $brdarr['NAME'];
$usernum = $currentuser["index"];
if (bbs_checkreadperm($usernum, $bid) == 0)
	bbs_clear_donothing();
if ($brdarr["FLAG"]&BBS_BOARD_GROUP)
	bbs_clear_donothing();

bbs_brcclear($board);
header('Location: bbsdoc.php?board='.urlencode($board));
?>