<?php
/*
** activate register user
** @author: windinsn Apr 12 , 2004
*/
require("funcs.php");
require("reg.inc.php");
$retry_time = intval($_COOKIE["ACTRETRYTIME"]);

if($retry_time > 9)
	html_error_quit("您的重试次数过多，请稍后再激活");
	
$userid = trim($_GET["userid"]);
$acode  = trim($_GET["acode"]);

if(!$userid || !$acode)
	html_error_quit("缺少用户名或激活码");

$lookupuser = array();
if(bbs_getuser($userid,$lookupuser)==0)
	html_error_quit("用户".$userid."不存在");
	
$userid = $lookupuser["userid"];
$ret = bbs_getactivation($userid,$activation);

switch($ret)
{
	case -1:
		html_error_quit("用户不存在");
		break;
	case -2:
		html_error_quit("激活码不存在");
		break;
	default:
}

if(bbs_reg_haveactivated($activation))
	html_error_quit("用户已激活");

if(bbs_reg_getactivationcode($activation)!=$acode)
{
	setcookie("ACTRETRYTIME",$retry_time+1,time()+360000);
	html_error_quit("激活码错误");
}

$ret = bbs_setactivation($userid,bbs_reg_successactivation($activation));
if($ret != 0)
	html_error_quit("系统错误");

html_init("gb2312");
?>
<body>
<br /><br /><br />
<p align="center">恭喜您，您已激活您的帐号，请登录进站填写注册单。</p>
<p align="center"><a href="/">[登录进站]</a></p>
<p align="center"><font color=red>提示：登录后，请进入[个人参数设定]->[填写注册单]，进行用户注册。</font></p>
<?php
html_normal_quit();
?>