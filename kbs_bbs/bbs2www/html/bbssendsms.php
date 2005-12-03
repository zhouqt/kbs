<?php
	require("www2-funcs.php");
	login_init();
	assert_login();
	page_header("发送手机短信");
	
	if (isset($_POST["dest"])){
		$dest = $_POST["dest"];

		if(!isset($_POST["msgstr"]))
			html_error_quit("请输入信息");

		$msgstr = $_POST["msgstr"];

		$ret = bbs_send_sms($dest, $msgstr);

		if( $ret == 0 )
			html_success_quit("发送成功");
		else
			html_error_quit("发送失败".$ret);
	} else{
?>
<form action="bbssendsms.php" method="post" class="small">
	<fieldset><legend>发送手机短信</legend>
		<div class="inputs">
			<label>对方手机号:</label><input type="text" name="dest" maxlength="11" id="sfocus"/><br/>
			<label>信息:</label><input type="text" name="msgstr"/>
		</div>
	</fieldset>
	<div class="oper"><input type="submit" value="发送"/></div>
</form>
<?php
		page_footer();
	}
?>
