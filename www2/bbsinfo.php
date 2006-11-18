<?php
	require("www2-funcs.php");
	login_init();
	bbs_session_modify_user_mode(BBS_MODE_EDITUFILE);
	toolbox_header("个人资料修改");
	assert_login();
	
	if (isset($_GET["submit"]) || isset($_POST["submit"]))
	{
		if( isset($_POST["username"]) ){
			$username = trim($_POST["username"]);
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
			html_success_quit($currentuser["userid"] . "个人资料修改成功");
		}
	}
?>
<form method="post" action="bbsinfo.php" class="medium">
<fieldset><legend>个人资料</legend><div class="inputs">
<label>您的帐号:</label><?php echo $currentuser["userid"];?><br/>
<label>您的昵称:</label><input type="text" name="username" value="<?php echo htmlspecialchars($currentuser["username"],ENT_QUOTES);?> " size="24" maxlength="39"><br/>
<label>发表大作:</label><?php echo $currentuser["numposts"];?> 篇<br/>
<label>上站次数:</label><?php echo $currentuser["numlogins"];?> 次<br/>
<label>上站时间:</label><?php echo (int)($currentuser["stay"]/60);?> 分钟<br/>
<label>真实姓名:</label><input type="text" name="realname" value="<?php echo $currentuser["realname"];?>" size="16" maxlength="39"><br/>
<label>居住地址:</label><input type="text" name="address" value="<?php echo $currentuser["address"];?>" size="40" maxlength="79"><br/>
<label>帐号建立:</label><?php echo date("D M j H:i:s Y",$currentuser["firstlogin"]);?><br/>
<label>最近光临:</label><?php echo date("D M j H:i:s Y",$currentuser["lastlogin"]);?><br/>
<?php if (isset($currentuser["score_user"])) { ?>
<label>用户积分:</label><?php echo $currentuser["score_user"];?><br/>
<?php } ?>
<label>来源地址:</label><?php echo $currentuser["lasthost"];?><br/>
<label>电子邮件:</label><input type="text" name="email" value="<?php echo $currentuser["email"];?>" size="32" maxlength="79"><br/>
</div></fieldset>
<div class="oper">
<input type="submit" name="submit" value="确定" /> <input type="reset" value="复原" />
</div>
</form>
<?php
	page_footer();
?>
