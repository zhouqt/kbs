<?php
require("funcs.php");
login_init();
require("reg.inc.php");
html_init("gb2312");

if ($loginok != 1)
	html_nologin();
else
{
	$new_reg_mail = $_POST["newemail"];
	
	if(!strcmp($currentuser["userid"],"guest"))
	{
		html_error_quit("请先登录!");
		exit();
	}
	if($currentuser["userlevel"]&BBS_PERM_LOGINOK)
	{
		html_error_quit("您已通过注册");
		exit();
	}
	
	
	//检查激活码
	$activation = "";
	$userid = $currentuser["userid"];
	$ret = bbs_getactivation($userid,$activation);
	
	if($ret == -1 || $ret == -10)
		html_error_quit("系统错误，请联系管理员!");
		
	if($ret == 0)
	{
		if(bbs_reg_haveactivated($activation && !isset($_GET["react"])))
			html_error_quit("您的帐户已激活");	
		$reg_email = bbs_reg_getactivationemail($activation);
	}
	else
		$reg_email = "";
	
?>
<body>
<br /><br />
<?php
		if($new_reg_mail)
		{
			$new_activation = bbs_create_activation();
			$ret = bbs_setactivation($userid,bbs_reg_newactivation($new_activation,$new_reg_mail));
			switch ($ret)
			{
				case -1:
					html_error_quit("用户不存在");
					break;
				case -2:
					html_error_quit("您已经通过注册了");
					break;
				case -10:
					html_error_quit("系统错误");
					break;
				default:
			}
			
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
<P class=MsoNormal><FONT size=2><SPAN lang=EN-US>Email: </SPAN><SPAN 
style=\"FONT-FAMILY: 宋体; mso-ascii-font-family: 'Times New Roman'; mso-hansi-font-family: 'Times New Roman'\">" . $new_reg_mail . "</SPAN></FONT></P>
<P class=MsoNormal><FONT size=2><A 
href=\"http://".BBS_DOMAIN_NAME."/bbsact.php?userid=".$userid."&acode=".$new_activation."\"><SPAN 
style=\"FONT-FAMILY: 宋体; mso-ascii-font-family: 'Times New Roman'; mso-hansi-font-family: 'Times New Roman'\">点击这里激活您在" . BBS_FULL_NAME . "的新帐号</SPAN>
<br /><br />
<P class=MsoNormal><FONT size=2><SPAN 
style=\"FONT-FAMILY: 宋体; mso-ascii-font-family: 'Times New Roman'; mso-hansi-font-family: 'Times New Roman'\"></SPAN></FONT></P>
</body>
</html>
";
/* To send HTML mail, you can set the Content-type header. */
			$headers  = "MIME-Version: 1.0\n";
			$headers .= "Content-type: text/html; charset=gb2312\n";
			
			/* additional headers */
			$headers .= "From: ".BBS_FULL_NAME." <http://".BBS_DOMAIN_NAME.">\n";
			
			if(!mail($new_reg_mail, "welcome to " . BBS_FULL_NAME, $mailbody,$headers))
			{		
?>
<p align="center">
激活码发送失败！请重新设置您的注册Email。
</p><p align="center">
<a href="bbssendacode.php">[重新发送激活码]</a>
</p>
<?php
			}
			else
			{
?>
<p align="center">
激活码发送成功！请查收信件并激活您的帐号。
</p><p align="center">
<a href="bbsfillform.html">[填写注册单]</a>
</p>
<?php
			}
		}
		else
		{
?>
<form action="bbssendacode.php<?php if(isset($_GET["react"])) echo "?react=1" ?>" method="post">
Emai地址：
<input type="text" name="newemail" size="20" maxlength="100" value="<?php echo $reg_email; ?>" />
<input type="submit" value="发送激活码" />
</form>
<?php
		}
		html_normal_quit();
	}
?>
