<?php
	/**
	 * This file registry a new id, work with bbsreg.html
	 * by binxun 2003.5
	 */
	require("funcs.php");
	set_fromhost();
	if (defined("HAVE_ACTIVATION")) {
		require("reg.inc.php");
	}

	@$num_auth=$_POST["num_auth"];
	@$userid=$_POST["userid"];
	@$nickname=$_POST["username"];
	@$reg_email=$_POST["reg_email"];
	@$password = $_POST["pass1"];
	@$re_password = $_POST["pass2"];
	
	session_start();
	html_init("gb2312");
	
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
	if (defined("HAVE_ACTIVATION")) {
		if(!($activation=bbs_create_activation()))
			html_error_quit("生成激活码错误，请联系管理员!");
	}
	
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

if (defined("HAVE_ACTIVATION")) {
	$ret = bbs_setactivation($userid,"0".$activation.$reg_email);
	if($ret != 0)
		html_error_quit("设置激活码错误");

html_init("gb2312");
	
	$ret=bbs_sendactivation($userid);
    if($ret)
{
?>
<body>
申请<?php echo BBS_FULL_NAME; ?>ID成功,<font color=red>注册码发送到您的注册Email失败!登录后请确认您的Email地址并重新发送注册码</font><br>
激活并通过审核后,你才将获得合法用户权限！<br/><a href="/">现在登录进站</a>
</body>
<?php
}   
else
{	
?>
<body>
申请<?php echo BBS_FULL_NAME; ?>ID成功,你现在还没有通过身份认证,只有最基本的权限,不能发文,发信,聊天等,请查收您收到的注册确认Email，点击里面的激活链接激活您在本站的帐号.<br>
激活并通过审核后,你将获得合法用户权限！<br/><a href="/">现在登录进站</a>
</body>
<?php
}

} else { // !defined("HAVE_ACTIVATION")
	@$realname=$_POST["realname"];
	@$dept=$_POST["dept"];
	@$address=$_POST["address"];
	@$year=$_POST["year"];
	@$month=$_POST["month"];
	@$day=$_POST["day"];
	@$phone=$_POST["phone"];
	@$gender=$_POST["gender"];
	$m_register = 0;
	$mobile_phone = 0;
	if(!strcmp($gender,"男"))$gender=1;
    	else $gender=2;
    	settype($year,"integer");
	settype($month,"integer");
	settype($day,"integer");
	settype($m_register,"bool");

    	if(!$m_register)$mobile_phone="";

	$ret=@bbs_createregform($userid,$realname,$dept,$address,$gender,$year,$month,$day,$reg_email,$phone,$mobile_phone, $_POST['OICQ'], $_POST['ICQ'], $_POST['MSN'],  $_POST['homepage'], intval($_POST['face']), $_POST['myface'], intval($_POST['width']), intval($_POST['height']), intval($_POST['groupname']), $_POST['country'],  $_POST['province'], $_POST['city'], intval($_POST['shengxiao']), intval($_POST['blood']), intval($_POST['belief']), intval($_POST['occupation']), intval($_POST['marital']), intval($_POST['education']), $_POST['college'], intval($_POST['character']), FALSE);//自动生成注册单
	switch($ret)
	{
	case 0:
		break;
	case 2:
		html_error_quit("该用户不存在!");
		break;
	case 3:
		html_error_quit("生成注册单发生 参数错误! 请手工填写注册单");
		break;
	default:
		html_error_quit("生成注册单发生 未知的错误! 请手工填写注册单");
		break;
	}
	html_init("gb2312");
?>
<body>
<h1>申请ID成功</h1>
申请<?php echo BBS_FULL_NAME; ?>ID成功,你现在还没有通过身份认证,只有最基本的权限,不能发文,发信,聊天等,系统已经自动生成注册单.<br>
注册单通过站长审核后,你将获得合法用户权限！<br/><a href="index.html">现在登录进站</a>
</body>
<?php
}
?>
</html>
