<?php
	require("funcs.php");
	if ($loginok != 1)
		html_nologin();
	else
	{
		html_init("gb2312");
?>
<body>
<center><?php echo BBS_FULL_NAME; ?> -- 删除好友 [使用者: <?php echo $currentuser["userid"]; ?>]<hr color="green">
<?php
		if (isset($_GET["userid"]))
		{
			$duserid = $_GET["userid"];
			$ret = bbs_delete_friend( $duserid );
			if($ret == 1){
				html_error_quit("您没有设定任何好友");
			}else if($ret == 2){
				html_error_quit("此人本来就不在你的好友名单中");
			}else if($ret == 3){
				html_error_quit("删除失败");
			}else{
?>
[<?php echo $duserid; ?>]已从您的好友名单中删除.<br>
<a href="/bbsfall.php">返回好友名单</a>
<?php
				html_normal_quit();
			}
		}else{
?>
<form action="/bbsfdel.php">
请输入欲删除的好友帐号:<input name="userid" type="text"><br>
<input type="submit" value="确定">
</form>
<a href="/bbsfall.php">返回好友名单</a>
<?php
			html_normal_quit();
		}
	}
?>
