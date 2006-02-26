<?php
	require("www2-funcs.php");
	login_init();
	assert_login();

	$p = @$HTTP_RAW_POST_DATA;
	if ($p) {
		bbs2_brcsync($p);
	}
	ob_start("ob_gzhandler");
	echo bbs2_brcdump();
?>
