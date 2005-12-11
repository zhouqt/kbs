<?php
    require("www2-funcs.php");
    login_init();
	page_header("隐身状态");
	assert_login();

    if (!($currentuser["userlevel"]&BBS_PERM_CLOAK)) {
		html_error_quit("错误的参数");
    }

	bbs_update_uinfo("invisible", !$currentuinfo["invisible"]);
    if (!$currentuinfo["invisible"])
        html_success_quit("开始隐身!");
    else
        html_success_quit("隐身已经停止!");
?>
