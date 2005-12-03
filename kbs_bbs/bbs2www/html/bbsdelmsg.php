<?php
    require("www2-funcs.php");
    login_init();
    page_header("讯息回寄");
	assert_login();

	$filename = bbs_sethomefile($currentuser["userid"],"msgindex");
	if (file_exists($filename))
		unlink($filename);
	$filename = bbs_sethomefile($currentuser["userid"],"msgindex2");
	if (file_exists($filename))
		unlink($filename);
	$filename = bbs_sethomefile($currentuser["userid"], "msgcount");
	if (file_exists($filename))
		unlink($filename);
		
	html_success_quit("已经删除所有讯息备份");
?>
