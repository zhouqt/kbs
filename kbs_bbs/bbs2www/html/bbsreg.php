<?php
	/**
	 * This file registry a new id, work with bbsreg.html
	 * by binxun 2003.5
	 */
	$needlogin=false;
	require("funcs.php");
	html_init("gb2312");

	@$userid=$_POST["userid"];
	@$nickname=$_POST["username"];

	@$realname=$_POST["realname"];
	@$dept=$_POST["dept"];
    @$address=$_POST["address"];
	@$year=$_POST["year"];
	@$month=$_POST["month"];
	@$day=$_POST["day"];
	@$reg_email=$_POST["reg_email"];
	@$phone=$_POST["phone"];
	@$gender=$_POST["gender"];
	@$m_register=$_POST["m_register"];
	@$mobile_phone=$_POST["mobile_phone"];

    if(!strchr($reg_email,'@'))
	    html_error_quit("错误的注册 email 地址!");

	//generate passwd
	$password=bbs_findpwd_check("","","");

	//发送密码到reg_email
/*	bool mail ( string to, string subject, string message [, string
additional_headers [, string additional_parameters]])*/
    if(!mail($reg_email,"BBS水木清华站用户注册密码",$userid . "的密码是" . $password))
	    html_error_quit("发送密码到您的注册Email失败!请确认您的该Email地址正确");

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

	if(!strcmp($gender,"男"))$gender=1;
    else
        $gender=2;
    settype($year,"integer");
	settype($month,"integer");
	settype($day,"integer");
	settype($m_register,"bool");

    if(!$m_register)$mobile_phone="";
	$ret=bbs_createregform($userid,$realname,$dept,$address,$gender,$year,$month,$day,$reg_email,$phone,$mobile_phone,TRUE);//自动生成注册单
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
?>
<body>
申请BBS水木清华ID成功,你现在还没有通过身份认证,只有最基本的权限,不能发文,发信,聊天等,两天后系统会自动生成注册单.<br>
注册单通过审核后,你将获得合法用户权限！<br/><a href="https://www.smth.edu.cn">现在登录进站</a>
</body>
</html>
