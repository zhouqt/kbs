<?php
require("inc/funcs.php");
require("inc/user.inc.php");
//header("Expires: .0");

if (!isset($_GET['sec'])){
	exit(0);
}
$secNum=$_GET['sec'];
is_numeric($secNum) or exit(0);

if (isset($_GET['fold'])) $isFold = true;
else $isFold = false;

if ( isset($_GET['fav']) ) {
	$fav = true;
	if ($loginok != 1) exit(0);
	if (bbs_load_favboard($secNum) == -1) {
		exit(0);
	}
} else {
	$fav = false;
	if ( ($secNum < 0)  && ($secNum >= $sectionCount)) exit(0);
}


setSecFoldCookie($fav ? -1 : $secNum, true, true);
setSecFoldCookie($fav ? -1 : $secNum, false, false);

?>
<html>
<body>
<script language="javascript" type="text/javascript" src="inc/browser.js"></script>
<script src="inc/loadThread.js"></script>
<?php
	showSecsJS($secNum,0,$isFold,$fav, false);
?>
<script language="javascript">
	var siteconf_BOARDS_PER_ROW = <?php echo BOARDS_PER_ROW; ?>;
	loadBoardFollow_Internal(<?php echo $secNum ?>, <?php echo ($fav ? "true" : "false"); ?>, false, false, <?php echo ($isFold?"true":"false"); ?>, true);
</script>
</body>
</html>