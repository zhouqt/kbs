<?php
	/**
	 * This file registry a new id, work with bbsreg.html
	 * by binxun 2003.5
	 */
	$needlogin=false;
	require("funcs.php");
	html_init("gb2312");

	@$userid=$_POST["userid"];
	@$pass1=$_POST["pass1"];
	@$pass2=$_POST["pass2"];
	@$nickname=$_POST["username"];

	@$realname=$_POST["realname"];
	@$dept=$_POST["dept"];
       @$address=$_POST["address"];
	@$year=$_POST["year"];
	@$month=$_POST["month"];
	@$day=$_POST["day"];
	@$email=$_POST["email"];
	@$phone=$_POST["phone"];


	if(strcmp($pass1,$pass2))
		html_error_quit("两次密码输入不一样");
	else if(strlen($pass1) < 5 || !strcmp($pass1,$userid))
       	html_error_quit("密码长度太短或者和用户名相同!");

	$ret=bbs_createnewid($userid,$pass1,$username);
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
			html_error_quit("未知的错误!");
			break;
	}
?>

	$ret=bbs_createregform($realname,$dept,$address,$year,$month,$day,$email,$phone,TRUE);//自动生成注册单
	switch($ret)
	{
	case 0:
		break;
	default:
		html_error_quit("未知的错误!");
		break;
	}
<body>
申请BBS水木清华ID成功,你现在还没有通过身份认证,只有最基本的权限,不能发文,发信,聊天等,两天后系统会自动生成注册单.<br>
注册单通过审核后,你将获得合法用户权限！<br/><a href="http://www.smth.edu.cn">现在登录进站</a>
</body>
</html>
