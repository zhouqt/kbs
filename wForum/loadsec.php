<?php
require("inc/funcs.php");
require("inc/user.inc.php");
//header("Expires: .0");

if (!isset($_GET['sec'])){
	exit(0);
}
$secNum=$_GET['sec'];
is_numeric($secNum) or exit(0);

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


setSecFoldCookie($fav ? -1 : $secNum, true);

?>
<html>
<body>
<script language="javascript" type="text/javascript" src="inc/browser.js"></script>
<script src="inc/loadThread.js"></script>
<?php
	showSecsJS($secNum,0,true,$fav);
?>
<script language="javascript">
	str = showSec(true, <?php echo ($fav ? "true" : "false"); ?>, boards, <?php echo $secNum ?>);
	parent.boards<?php echo $secNum; ?> = boards;
	oTd = getParentRawObject("followSpan<?php echo $secNum; ?>");
	oTip = getParentRawObject("followTip<?php echo $secNum; ?>");
	oTip.style.display = 'none';
	oTd.innerHTML = str;
</script>
</body>
</html>