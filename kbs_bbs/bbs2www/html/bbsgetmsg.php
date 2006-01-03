<?php
	require("www2-funcs.php");
	login_init();
	page_header("讯息", FALSE);
?>
<body class="msgs">
<?php
	if (strcmp($currentuser["userid"], "guest")) {
		$ret=bbs_getwebmsg($srcid,$msgbuf,$srcutmpnum,$sndtime);
		if ($ret) {
?>
<bgsound src="/sound/msg.wav">
<div id="msgs">
<?php echo $srcid; ?> (<?php echo strftime("%b %e %H:%M", $sndtime); ?>): <?php echo htmlspecialchars($msgbuf); ?> 
(<a target="f3" href="bbssendmsg.php?destid=<?php echo $srcid; ?>&destutmp=<?php echo $srcutmpnum; ?>">[回讯息]</a> 
<a href="bbsgetmsg.php?refresh">[忽略]</a>)
</div>
<?php
			$frameheight = 25;
			$timeout = 540;
		} else {
			//no msg
			$frameheight = 0;
			$timeout = (isset($_GET["refresh"])) ? 60 : 540;
		}
	} else {
		$frameheight = 0;
		$timeout = 0;
	}
?>
<script>
var ff = top.document.getElementById("viewfrm");
if (ff) ff.rows = "<?php echo $frameheight; ?>,*,20";
<?php if ($timeout > 0) { ?>
setTimeout("location.reload()", <?php echo $timeout*1000 ?>);
<?php } ?>
</script>
</body>
</html>