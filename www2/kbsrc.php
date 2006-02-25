<?php
        require("www2-funcs.php");
        login_init();
        assert_login();

	ob_start("ob_gzhandler");
	echo bbs2_brcdump();
?>
