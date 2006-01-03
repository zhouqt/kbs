<?php
	require("www2-funcs.php");
	login_init();
	bbs_session_modify_user_mode(BBS_MODE_GMENU);
	toolbox_header("临时昵称修改");
	assert_login();

	if (isset($_POST["username"])) {
		$username = $_POST["username"];
		if( strlen($username) < 2 )
			html_error_quit("昵称太短");
		$ret = bbs_modify_nick( $username );
		if($ret < 0){
			html_error_quit("系统错误");
		} else {
			html_success_quit("临时昵称修改成功");
		}
		exit;
	}
?>
<form action="bbsnick.php" method="post" class="medium">
	<fieldset><legend>临时改变昵称 (环顾四方有效)</legend>
		<div class="inputs">
			<label>新昵称:</label>
			<input id="sselect" type="text" name="username" value="<?php echo htmlspecialchars($currentuser["username"],ENT_QUOTES);?>" size="24" maxlength="39"/>
		</div>
	</fieldset>
	<div class="oper"><input type="submit" value="确定修改"></div>
</form>
<?php
	page_footer();
?>
