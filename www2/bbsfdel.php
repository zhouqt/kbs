<?php
    require("www2-funcs.php");
    login_init();
	toolbox_header("好友编辑");
	assert_login();
	if (isset($_GET["userid"])) {
		$duserid = $_GET["userid"];
		$ret = bbs_delete_friend( $duserid );
		if($ret == 1){
			html_error_quit("您没有设定任何好友");
		}else if($ret == 2){
			html_error_quit("此人本来就不在你的好友名单中");
		}else if($ret == 3){
			html_error_quit("删除失败");
		}else{
			html_success_quit($duserid . "已从您的好友名单中删除.");
		}
	}
?>