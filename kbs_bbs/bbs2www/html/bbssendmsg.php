<?php
	/**
	 * This file send message for current user.
	 * $Id$
	 */
    require("funcs.php");
    if ($loginok != 1)
		html_nologin();
    else
    {
        html_init("gb2312");
        if ($currentuser["userid"]=="guest")
			html_error_quit("匆匆过客不能发送讯息，请先登录");
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
<body onload="document.form0.msg.focus()">
<form name="form0" action="<?php echo $_SERVER["PHP_SELF"]; ?>" method="post">
<input type="hidden" name="destutmp" value="<?php echo $destutmp; ?>"/>
送讯息给: <input name="destid" maxlength="12" value="<?php echo $destid; ?>" size="12"/><br/>
讯息内容: <input name="msg" maxlength="50" size="50" value="<?php echo $msg; ?>"/><br/>
<input type="submit" value="确认" width="6"/>
</form> 
<?php
			html_normal_quit();
		}
		bbs_sendwebmsg($destid, $msg, $destutmp, $errmsg);
?>
<body onload="document.form1.b1.focus()">
<?php echo $errmsg; ?>
<script>top.fmsg.location="bbsgetmsg.php"</script>
<br/>
<form name="form1">
<input name="b1" type="button" onclick="history.go(-2)" value="[返回]"/>
</form>
<?php
		html_normal_quit();
    }
?>
