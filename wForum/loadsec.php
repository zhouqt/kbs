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

if (SECTION_DEF_CLOSE) {
	$cn = "HideSecBoards";
	if (!isset($_COOKIE[$cn])) {
		$ssb = ~0;
		/* 默认关闭的分区被打开时需要设置这个 cookie 的默认值，因为客户端默认的是 0 */
		setcookie($cn, $ssb ,time() + 604800);
	}
}
html_init();
?>
<body>
<script src="inc/loadThread.js"></script>
<?php
	showSecsJS($secNum,0,$isFold,$fav, false);
?>
<script language="javascript">
<!--
	parent.boards<?php echo $secNum; ?> = boards;
	parent.loadBoardFollow(<?php echo $secNum ?>, <?php echo ($fav ? 1 : 0); ?>, false, false, <?php echo ($isFold ? 1 : 0); ?>);
	parent.foldflag<?php echo $secNum; ?> = <?php echo ($isFold ? 2 : 1); ?>;
//-->
</script>
</body>
</html>