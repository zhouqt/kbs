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
?>
<html>
<body>
<script language="javascript" type="text/javascript" src="inc/browser.js"></script>
<script src="inc/loadThread.js"></script>
<?php
	showSecsJS($secNum,0,$isFold,$fav, false);
?>
<script language="javascript">
<!--
	parent.boards<?php echo $secNum; ?> = boards;
	parent.loadBoardFollow(<?php echo $secNum ?>, <?php echo ($fav ? "true" : "false"); ?>, false, false, <?php echo ($isFold?"true":"false"); ?>);
	targetTip = getParentRawObject("followTip<?php echo $secNum; ?>");
	targetTip.style.display = 'none';
	parent.foldflag<?php echo $secNum; ?> = <?php echo ($isFold ? 2 : 1); ?>;
//-->
</script>
</body>
</html>