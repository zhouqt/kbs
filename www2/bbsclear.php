<?php
require("www2-funcs.php");
login_init();

if (isset($_GET['board']))
	$board = $_GET['board'];
elseif (isset($_POST['board']))
	$board = $_POST['board'];
else
	die;

$brdarr = array();
$isnormalboard = bbs_safe_getboard(0, $board, $brdarr);
if (is_null($isnormalboard)) {
	die;
}
if ($brdarr["FLAG"]&BBS_BOARD_GROUP)
	die;
$board = $brdarr["NAME"];
$bid = $brdarr["BID"];

$guest = (!strcmp($currentuser['userid'],'guest'));

if ($isnormalboard && (isset($_GET["kbsrc"]))) {
	// this could be cacheable... - atppp
} else {
	if (!$guest) bbs_brcclear($board);
}

$id = $brdarr["LASTPOST"];
$page_title = "<a href=\"bbsdoc.php?board=".$board."\">".htmlspecialchars($brdarr["DESC"])."</a>";
$metas =  "<meta http-equiv='refresh' content='0; url=bbsdoc.php?board=".urlencode($board)."'/>";
page_header("Çå³ıÎ´¶Á¼ÇÂ¼", $page_title, $metas);
echo "<div id='kbsrcInfo'>con,$bid,$id,f</div>";
page_footer();
?>