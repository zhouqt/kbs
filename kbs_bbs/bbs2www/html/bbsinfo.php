<?php
	require("funcs.php");
	if ($loginok != 1)
		html_nologin();
	else
	{
		html_init("gb2312");
        if ($currentuser["userid"]=="guest")
            html_error_quit("匆匆过客不能修改资料");
?>
<body>
<center><?php echo BBS_FULL_NAME; ?> -- 用户个人资料 [使用者: <?php echo $currentuser["userid"]; ?>]<hr color="green">
<?php
		if (isset($_GET["submit"]) || isset($_POST["submit"]))
		{
			if( isset($_POST["username"]) ){
				$username = $_POST["username"];
				if( strlen($username) < 2 )
					html_error_quit("昵称太短");
			}else
				html_error_quit("昵称必须填写");

			if( isset($_POST["realname"]) ){
				$realname = $_POST["realname"];
				if( strlen($realname) < 2 )
					html_error_quit("真实姓名太短");
			}else
				html_error_quit("真实姓名必须填写");

			if( isset($_POST["address"]) ){
				$address = $_POST["address"];
				if( strlen($address) < 8 )
					html_error_quit("居住地址太短");
			}else
				html_error_quit("居住地址必须填写");

			if( isset($_POST["email"]) ){
				$email = $_POST["email"];
				if( strlen($email) < 8 )
					html_error_quit("email不合法");
			}else
				html_error_quit("email必须填写");

			$ret = bbs_modify_info( $username, $realname, $address, $email );

			if($ret < 0){
				html_error_quit("系统错误");
			}else{
?>
<?php echo $currentuser["userid"]; ?>个人资料修改成功<br>
<?php
				html_normal_quit();
			}
		}else{
?>
<form action="/bbsinfo.php" method=post>
您的帐号: <?php echo $currentuser["userid"];?><br>
您的昵称: <input type="text" name="username" value="<?php echo $currentuser["username"];?>" size=24 maxlength=39><br>
发表大作: <?php echo $currentuser["numposts"];?> 篇<br>
上站次数: <?php echo $currentuser["numlogins"];?> 次<br>
上站时间: <?php echo $currentuser["stay"]/60;?> 分钟<br>
真实姓名: <input type="text" name="realname" value="<?php echo $currentuser["realname"];?>" size=16 maxlength=39><br>
居住地址: <input type="text" name="address" value="<?php echo $currentuser["address"];?>" size=40 maxlength=79><br>
帐号建立: <?php echo date("D M j H:i:s Y",$currentuser["firstlogin"]);?><br>
最近光临: <?php echo date("D M j H:i:s Y",$currentuser["lastlogin"]);?><br>
来源地址: <?php echo $currentuser["lasthost"];?><br>
电子邮件: <input type="text" name="email" value="<?php echo $currentuser["email"];?>" size=32 maxlength=79><br>
<input type="submit" name="submit" value="确定"><input type="reset" value="复原">
</form>
<hr>
<?php
			html_normal_quit();
		}
	}
?>
