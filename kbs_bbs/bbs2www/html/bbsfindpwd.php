<?php
	/**
	 * 丢失密码的用户取回新密码
	 * by binxun 2003.5
	 */
	require("funcs.php");
	html_init("gb2312");

	@$userid=$_POST["userid"];
	@$realname=$_POST["realname"];
	@$email=$_POST["email"];

	if(strlen($userid)<2 || strlen($realname) < 2 || strlen($email) < 4)
     	html_error_quit("所填写的某一项太短了");

	$ret=bbs_findpwd_check($userid,$realname,$email);
    //settype($ret,"integer");
	echo " ret is ".$ret;
	settype($ret,"string");
	if(strlen($ret) > 1)
	{
        echo "密码是 ".$ret;
		//todo 发送密码到指定的邮箱
	}
	else
	    html_error_quit("取回密码失败!,您所填写的资料和注册资料不合!");

?>
<body>
取回密码成功,请到指定的email里取回该ID的密码</body>
</html>
