<?php
	/**
	 * This file Fill registry form.
	 * by binxun 2003.5
	 */
	require("funcs.php");
	html_init("gb2312");

	if ($loginok != 1)
		html_nologin();
	else
	{
		@$realname=$_POST["realname"];
		@$dept=$_POST["dept"];
		@$address=$_POST["address"];
		@$year=$_POST["year"];
		@$month=$_POST["month"];
		@$day=$_POST["day"];
		@$email=$_POST["email"];
		@$phone=$_POST["phone"];
		@$gender=$_POST["gender"];


	if(!strcmp($currentuser["userid"],"guest"))
		html_error_quit("请申请另外的帐号填写注册单!");

	//用户已经通过注册
	//未满等待时间(先放到phplib里面做了)
	if(!strcmp($gender,"男"))$gender=1;
    else
        $gender=2;
	settype($year,"integer");
	settype($month,"integer");
	settype($day,"integer");
    if (BBS_WFORUM==0)  {
        $ret=bbs_createregform($userid,$realname,$dept,$address,$gender,$year,$month,$day,$reg_email,$phone,$mobile_phone,FALSE);//自动生成注册单
    } else {
        $ret=bbs_createregform($userid,$realname,$dept,$address,$gender,$year,$month,$day,$email,$phone,$mobile_phone, $_POST['OICQ'], $_POST['ICQ'], $_POST['MSN'],  $_POST['homepage'], intval($_POST['face']), $_POST['myface'], intval($_POST['width']), intval($_POST['height']), intval($_POST['groupname']), $_POST['country'],  $_POST['province'], $_POST['city'], intval($_POST['shengxiao']), intval($_POST['blood']), intval($_POST['belief']), intval($_POST['occupation']), intval($_POST['marital']), intval($_POST['education']), $_POST['college'], intval($_POST['character']), TRUE);//自动生成注册单
    }
//	$ret=bbs_createregform($currentuser["userid"],$realname,$dept,$address,$gender,$year,$month,$day,$email,$phone,"",FALSE); //手工填写注册单

	switch($ret)
	{
	case 0:
		break;
	case 1:
		html_error_quit("您的注册单还没有处理，请耐心等候");
		break;
	case 2:
		html_error_quit("该用户不存在!");
		break;
	case 3:
		html_error_quit("参数错误");
		break;
	case 4:
		html_error_quit("你已经通过注册了!");
		break;
	case 5:
		html_error_quit("您注册尚不满48小时,请在首次注册48小时后再填写注册单.");
		break;
	default:
		html_error_quit("未知的错误!");
		break;
	}
}
?>
<body>
注册单已经提交,24小时内站务将会审核,如果通过,你就会获得合法用户权限！<br>
<a href=\"javascript:history.go(-1)\">快速返回</a>
</body>
</html>
