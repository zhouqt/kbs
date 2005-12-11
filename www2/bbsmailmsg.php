<?php
    require("www2-funcs.php");
    login_init();
    page_header("讯息回寄");
	assert_login();

	if (bbs_mailwebmsgs() === false)
		html_error_quit("讯息备份寄回信箱失败");
	else
		html_success_quit("讯息备份已经寄回您的信箱");
?>
