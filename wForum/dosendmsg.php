<?php
require("inc/funcs.php");

require("inc/usermanage.inc.php");

require("inc/user.inc.php");

html_init();

if ($loginok==1) {
	main();
}else {
	foundErr("本页需要您以正式用户身份登陆之后才能访问！");
}

if (isErrFounded()) {
	echo "<body>";
	html_error_quit();
} 

show_footer();


function main(){
	global $_POST;
	if (isset($_POST["destid"]))
		$destid = $_POST["destid"];
	else
		$destid = "";
	if (isset($_POST["msg"]))
		$msg = $_POST["msg"];
	else
		$msg = "";
	if (isset($_POST["destutmp"]))
		$destutmp = $_POST["destutmp"];
	else
		$destutmp = 0;
	settype($destutmp, "integer");
	if (strlen($destid) == 0 || strlen($msg) == 0)	{
		foundErr("短信参数错误");
		return false;
	}
	if (bbs_sendwebmsg($destid, $msg, $destutmp, $errmsg)==FALSE){
		foundErr($errmsg);
		return false;
	}
	setSucMsg("消息已成功发送！");
	return html_success_quit('返回聊天记录', 'showmsgs.php');
}


?>