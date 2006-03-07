<?php
	require("www2-funcs.php");
	login_init();
	cache_header("nocache");

	if(!strcmp($currentuser["userid"],"guest")) die;

	$p = @$HTTP_RAW_POST_DATA;
	if ($p) {
		$check_userid = @$_GET["userid"];
		if (!$check_userid) die;
		if (strcmp($currentuser["userid"], $check_userid) != 0) die;
		bbs2_brcsync($p);
	}
	if (!isset($_GET["logout"])) {
		ob_start("ob_gzhandler");
		echo bbs2_brcdump();
	}
?>
