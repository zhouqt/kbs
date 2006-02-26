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

$id = $brdarr["LASTPOST"];
$page_title = "<a href=\"bbsdoc.php?board=".$brdarr["NAME"]."\">".htmlspecialchars($brdarr["DESC"])."</a>";
$metas =  "<meta http-equiv='refresh' content='0; url=bbsdoc.php?board=".urlencode($board)."'/>";
$metas .= "<meta name='kbsrc.con' content='$bid,$id,f'/>";
page_header("Çå³ýÎ´¶Á¼ÇÂ¼", $page_title, $metas);
page_footer();
?>