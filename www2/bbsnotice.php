<?php
	require("www2-funcs.php");
	login_init();
	
	if (isset($_GET["file"])) {
		if($_GET["file"] == "hotinfo") {
			$title = "近期热点";
			$fname = "0Announce/hotinfo";
		}
		else if($_GET["file"] == "systeminfo") {
			$title = "系统热点";
			$fname = "0Announce/systeminfo";
		}
		else {
			exit;
		}

		page_header($title);
?>
<p> &nbsp; </p>
<div class="main smaller">
<span class="c36">
<?php
		$s = bbs2_readfile($fname);
		if (is_string($s)) {
?>
<link rel="stylesheet" type="text/css" href="static/www2-ansi.css"/>
<script type="text/javascript" src="static/www2-addons.js"></script>
<div id="divPlan" class="AnsiArticleBW"><div id="dp1">
<script type="text/javascript"><!--
triggerAnsiDiv('divPlan','dp1');
<?php
				echo $s;
?>
//-->
</script></div></div>
<?php
			}
?>
</div>
<?php
		page_footer();
	}
?>

