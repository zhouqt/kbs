<?php
	/**
	 * This file registry a new id, work with bbsreg.html
	 * by binxun 2003.5
	 */
	$needlogin=false;
	require("funcs.php");
	require("reg.inc.php");

	@$num_auth=$_POST["num_auth"];
	@$userid=$_POST["userid"];
	@$nickname=$_POST["username"];
	/*
	@$realname=$_POST["realname"];
	@$dept=$_POST["dept"];
	@$address=$_POST["address"];
	@$year=$_POST["year"];
	@$month=$_POST["month"];
	@$day=$_POST["day"];
	*/
	@$reg_email=$_POST["reg_email"];
	/*
	@$phone=$_POST["phone"];
	@$gender=$_POST["gender"];
	@$m_register=$_POST["m_register"];
	@$mobile_phone=$_POST["mobile_phone"];
	*/
	@$password = $_POST["pass1"];
	@$re_password = $_POST["pass2"];
	
	session_start();
	if(!isset($_SESSION['num_auth']))
  	    html_error_quit("请等待识别的图片显示完毕!");
	if(strcasecmp($_SESSION['num_auth'],$num_auth))
	    html_error_quit("图片上的字符串识别错误!难道你是机器人？");

	if(!strchr($reg_email,'@'))
	    html_error_quit("错误的注册 email 地址!");
	
	if($password != $re_password)
	    html_error_quit("密码与确认密码不一致! ");
	    
	if(strlen($password) < 4 || strlen($password) > 39)
	    html_error_quit("密码不规范, 密码长度应为 4-39 位! ");
	
	//generate activation code
	if(!($activation=bbs_create_activation()))
		html_error_quit("生成激活码错误，请联系管理员!");
	
	//create new id
	$ret=bbs_createnewid($userid,$password,$nickname);

	switch($ret)
	{
	case 0:
			break;
	case 1:
			html_error_quit("用户名有非数字字母字符或者首字符不是字母!");
			break;
	case 2:
			html_error_quit("用户名至少为两个字母!");
			break;
	case 3:
			html_error_quit("系统用字或不雅用字!");
			break;
	case 4:
			html_error_quit("该用户名已经被使用!");
			break;
	case 5:
			html_error_quit("用户名太长,最长12个字符!");
			break;
	case 6:
			html_error_quit("密码太长,最长39个字符!");
			break;
	case 10:
			html_error_quit("系统错误,请与系统管理员SYSOP联系.");
			break;
	default:
			html_error_quit("注册ID时发生未知的错误!");
			break;
	}
	
	$ret = bbs_setactivation($userid,"0".$activation.$reg_email);
	if($ret != 0)
		html_error_quit("设置激活码错误");
	
$mailbody="
<?xml version=\"1.0\" encoding=\"gb2312\">
<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">
<html>
<body><P class=MsoNormal><FONT size=2><SPAN 
style=\"FONT-FAMILY: 宋体; mso-ascii-font-family: 'Times New Roman'; mso-hansi-font-family: 'Times New Roman'\">  " . $realname  . "欢迎您来到</SPAN><SPAN 
style=\"FONT-FAMILY: 宋体; mso-ascii-font-family: 'Times New Roman'; mso-hansi-font-family: 'Times New Roman'\">" . BBS_FULL_NAME . "。</SPAN></FONT></P>
<P class=MsoNormal><FONT size=2><SPAN 
style=\"FONT-FAMILY: 宋体; mso-ascii-font-family: 'Times New Roman'; mso-hansi-font-family: 'Times New Roman'\">您的注册信息是：</SPAN></FONT></P>
<P class=MsoNormal><FONT size=2><SPAN 
style=\"FONT-FAMILY: 宋体; mso-ascii-font-family: 'Times New Roman'; mso-hansi-font-family: 'Times New Roman'\">用户名：" . $userid . "</SPAN></FONT></P>
<P class=MsoNormal><FONT size=2><SPAN 
style=\"FONT-FAMILY: 宋体; mso-ascii-font-family: 'Times New Roman'; mso-hansi-font-family: 'Times New Roman'\">昵称：" . $nickname . "<SPAN></FONT></P>
<P class=MsoNormal><FONT size=2><SPAN 
style=\"FONT-FAMILY: 宋体; mso-ascii-font-family: 'Times New Roman'; mso-hansi-font-family: 'Times New Roman'\">密码：" . $password . "</SPAN></FONT></P>
<P class=MsoNormal><FONT size=2><SPAN lang=EN-US>email</SPAN><SPAN 
style=\"FONT-FAMILY: 宋体; mso-ascii-font-family: 'Times New Roman'; mso-hansi-font-family: 'Times New Roman'\">：" . $reg_email . "</SPAN></FONT></P>
<P class=MsoNormal><FONT size=2><A 
href=\"https://www.smth.edu.cn/bbsact.php?userid=".$userid."&acode=".$activation."<SPAN 
style=\"FONT-FAMILY: 宋体; mso-ascii-font-family: 'Times New Roman'; mso-hansi-font-family: 'Times New Roman'\">点击这里激活您在" . BBS_FULL_NAME . "的新帐号</SPAN>
<br /><br />
<P class=MsoNormal><FONT size=2><SPAN 
style=\"FONT-FAMILY: 宋体; mso-ascii-font-family: 'Times New Roman'; mso-hansi-font-family: 'Times New Roman'\"></SPAN></FONT></P>
</body>
</html>
";
/* To send HTML mail, you can set the Content-type header. */
$headers  = "MIME-Version: 1.0\r\n";
$headers .= "Content-type: text/html; charset=gb2312\r\n";

/* additional headers */
$headers .= "From: BBS水木清华站 <https://www.smth.edu.cn>\r\n";

html_init("gb2312");
if(!mail($reg_email, "welcome to " . BBS_FULL_NAME, $mailbody,$headers))
{
?>
<body>
申请BBS水木清华ID成功,<font color=red>注册码发送到您的注册Email失败!登录后请确认您的Email地址并重新发送注册码</font><br>
激活并通过审核后,你才将获得合法用户权限！<br/><a href="https://www.smth.edu.cn">现在登录进站</a>
</body>
<?php
}   
else
{	
?>
<body>
申请BBS水木清华ID成功,你现在还没有通过身份认证,只有最基本的权限,不能发文,发信,聊天等,请查收您收到的注册确认Email，点击里面的激活链接激活您在本站的帐号.<br>
激活并通过审核后,你将获得合法用户权限！<br/><a href="https://www.smth.edu.cn">现在登录进站</a>
</body>
<?php
}
?>
</html>
