<?php
	require("funcs.php");
	if ($loginok != 1)
		html_nologin();
	else
	{
		html_init("gb2312");
        if ($currentuser["userid"]=="guest")
            html_error_quit("匆匆过客不能修改昵称");
?>
<body>
<center><?php echo BBS_FULL_NAME; ?> -- 临时改变昵称(环顾四方有效) [使用者: <?php echo $currentuser["userid"]; ?>]<hr color="green">
<?php
		if (isset($_GET["submit"]) || isset($_POST["submit"]))
		{
			if( isset($_POST["username"]) ){
				$username = $_POST["username"];
				if( strlen($username) < 2 )
					html_error_quit("昵称太短");
			}else
				html_error_quit("昵称必须填写");

			$ret = bbs_modify_nick( $username );

			if($ret < 0){
				html_error_quit("系统错误");
			}else{
?>
<?php echo $currentuser["userid"]; ?>临时昵称修改成功<br>
<?php
				html_normal_quit();
			}
		}else{
?>
<form action="/bbsnick.php" method=post>
新昵称: <input type="text" name="username" value="<?php echo $currentuser["username"];?>" size=24 maxlength=39><br>
<input type="submit" name="submit" value="确定">
</form>
<hr>
<?php
			html_normal_quit();
		}
	}
?>
