<?php
	require("funcs.php");
	if ($loginok != 1)
		html_nologin();
	else
	{
		html_init("gb2312");
        if ($currentuser["userid"]=="guest")
            html_error_quit("匆匆过客不能设定好友");
?>
<body>
<center><?php echo BBS_FULL_NAME; ?> -- 增加好友 [使用者: <?php echo $currentuser["userid"]; ?>]<hr color="green">
<?php
		if (isset($_GET["userid"]))
		{
			$duserid = $_GET["userid"];
			if( isset($_GET["exp"]))
				$expp = $_GET["exp"];
			else
				$expp="";
			$ret = bbs_add_friend( $duserid ,$expp );
			if($ret == -1){
				html_error_quit("您没有权限设定好友或者好友个数超出限制");
			}else if($ret == -2){
				html_error_quit("此人本来就在你的好友名单中");
			}else if($ret == -3){
				html_error_quit("系统出错");
			}else if($ret == -4){
				html_error_quit("用户不存在");
			}else{
?>
[<?php echo $duserid; ?>]已增加到您的好友名单中.<br>
<a href="/bbsfall.php">返回好友名单</a>
<?php
				html_normal_quit();
			}
		}else{
?>
<form action="/bbsfadd.php">
请输入欲增加的好友帐号:<input maxlength=12 name="userid" type="text"><br>
请输入欲增加的好友备注:<input maxlength=14 name="exp" type="text"><br>
<input type="submit" value="确定">
</form>
<a href="/bbsfall.php">返回好友名单</a>
<?php
			html_normal_quit();
		}
	}
?>
