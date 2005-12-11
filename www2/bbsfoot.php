<?php
    $setboard=0;
    require("www2-funcs.php");
	login_init();
	page_header("状态", FALSE);
	if (isset($_GET["total"]))
		$oldtotal = $_GET["total"];
	else
		$oldtotal = 0;
	settype($oldtotal,"integer");

	if (isset($_GET["unread"]))
		$oldunread = $_GET["unread"];
	else
		$oldunread = 0;
	settype($oldunread,"integer");
	
	if (strcmp($currentuser["userid"], "guest")) {
		if (!bbs_getmailnum($currentuser["userid"],$total,$unread, $oldtotal, $oldunread)) {
			$unread = $total = 0;
		}
	} else {
		$unread = false;
	}
?>
<script type="text/javascript">
<!--
	addBootFn(footerStart);
	var stayTime = <?php echo (time()-$currentuinfo["logintime"]); ?>;
	var serverTime = <?php echo (time() + intval(date("Z"))); ?>;
	var hasMail = <?php echo $unread ? "1" : "0"; ?>;
//-->
</script>
<body><div class="footer">时间[<span id="divTime"></span>] 在线[<?php echo bbs_getonlinenumber(); ?>]
帐号[<a href="bbsqry.php?userid=<?php echo $currentuser["userid"]; ?>" target="f3"><?php echo $currentuser["userid"]; ?></a>]
<?php
	if ($unread !== false) {
echo "信箱[<a href=\"bbsmailbox.php?path=.DIR&title=收件箱\" target=\"f3\">";
		if ($unread) {
			echo $total . "封(新信" . $unread . ")</a>] <bgsound src='sound/newmail.mp3'>";
		} else {
			echo $total . "封</a>] ";
		}
	}
?>
停留[<span id="divStay"></span>]
</div></body>
</html>
