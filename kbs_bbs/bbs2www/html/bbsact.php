<?php
/*
** activate register user
** @author: windinsn Apr 12 , 2004
*/
require("funcs.php");
require("reg.inc.php");
$retry_time = intval($_COOKIE["ACTRETRYTIME"]);

//if($retry_time > 9)
//	html_error_quit("您的重试次数过多，请稍后再激活");
	
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
/*
if(!strcmp($gender,"男"))
	$gender=1;
else
	$gender=2;
settype($year,"integer");
settype($month,"integer");
settype($day,"integer");
settype($m_register,"bool");

if(!$m_register)$mobile_phone="";
if (BBS_WFORUM==0)  {
    $ret=bbs_createregform($userid,$realname,$dept,$address,$gender,$year,$month,$day,$reg_email,$phone,$mobile_phone,TRUE);//自动生成注册单
}
else {
    $ret=bbs_createregform($userid,$realname,$dept,$address,$gender,$year,$month,$day,$email,$phone,$mobile_phone, $_POST['OICQ'], $_POST['ICQ'], $_POST['MSN'],  $_POST['homepage'], intval($_POST['face']), $_POST['myface'], intval($_POST['width']), intval($_POST['height']), intval($_POST['groupname']), $_POST['country'],  $_POST['province'], $_POST['city'], intval($_POST['shengxiao']), intval($_POST['blood']), intval($_POST['belief']), intval($_POST['occupation']), intval($_POST['marital']), intval($_POST['education']), $_POST['college'], intval($_POST['character']), TRUE);//自动生成注册单
}

switch($ret)
{
	case 0:
		break;
	case 2:
		html_error_quit("该用户不存在!");
		break;
	case 3:
		html_error_quit("生成注册单发生 参数错误! 请两天后手工填写注册单");
		break;
	default:
		html_error_quit("生成注册单发生 未知的错误! 请两天后手工填写注册单");
		break;
}
*/
?>