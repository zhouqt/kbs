<?php
	require("www2-funcs.php");
	login_init();
	toolbox_header("好友编辑");
	assert_login();
	if (isset($_POST["userid"]))
	{
		$duserid = $_POST["userid"];
		@$expp = $_POST["exp"];
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
			html_success_quit($duserid . "已增加到您的好友名单中.");
		}
	}else{
?>
<form action="bbsfadd.php" method="post" class="medium">
	<fieldset><legend>增加好友</legend>
		<div class="inputs">
			<label>请输入欲增加的好友帐号:</label>
				<input maxlength=12 name="userid" type="text" id="sfocus"/><br/>
			<label>请输入欲增加的好友备注:</label>
				<input maxlength=14 name="exp" type="text"/>
		</div>
	</fieldset>
	<div class="oper"><input type="submit" value="添加好友"></div>
</form>
<a href="bbsfall.php">返回好友名单</a>
<?php
		page_footer();
	}
?>
