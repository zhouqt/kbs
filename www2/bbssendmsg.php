<?php
	require("www2-funcs.php");
	login_init();
	bbs_session_modify_user_mode(BBS_MODE_MSGING);
	page_header("发讯息");
	assert_login();

	// get user input
	if (isset($_POST["destid"]))
		$destid = $_POST["destid"];
	elseif (isset($_GET["destid"]))
		$destid = $_GET["destid"];
	else
		$destid = "";
	if (isset($_POST["msg"]))
		$msg = $_POST["msg"];
	else
		$msg = "";
	if (isset($_POST["destutmp"]))
		$destutmp = $_POST["destutmp"];
	elseif (isset($_GET["destutmp"]))
		$destutmp = $_GET["destutmp"];
	else
		$destutmp = 0;
	settype($destutmp, "integer");
	if (strlen($destid) == 0 || strlen($msg) == 0)
	{
?>
<form action="<?php echo $_SERVER["PHP_SELF"]; ?>" method="post" class="large">
<fieldset><legend>发送讯息</legend><div class="inputs">
<input type="hidden" name="destutmp" value="<?php echo $destutmp; ?>"/>
<label>送讯息给:</label><input <?php if (!$destid) echo 'id="sfocus"'; ?> type="text" name="destid" maxlength="12" value="<?php echo $destid; ?>" size="12"/><br/>
<label>讯息内容:</label><input <?php if ($destid) echo 'id="sfocus"'; ?> type="text" name="msg" maxlength="50" size="50" value="<?php echo $msg; ?>"/>
</div></fieldset>
<div class="oper"><input type="submit" value="确认"/></div>
</form>
<?php
		page_footer();
	} else {
		$ret = bbs_sendwebmsg($destid, $msg, $destutmp, $errmsg);
?>
<script>top.fmsg.location="bbsgetmsg.php?refresh";</script>
<?php
		if($ret)
            html_success_quit($errmsg);
        else
            html_error_quit($errmsg);
	}
?>
