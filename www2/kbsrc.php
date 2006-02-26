<?php
	require("www2-funcs.php");
	login_init();

	if(!strcmp($currentuser["userid"],"guest")) die;

	$p = @$HTTP_RAW_POST_DATA;
	if ($p) {
		bbs2_brcsync($p);
	}
	ob_start("ob_gzhandler");
	echo bbs2_brcdump();
?>
