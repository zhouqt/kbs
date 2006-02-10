<?php
	require("funcs.php");
	set_fromhost();

	if (!isset($_POST["userid"])) {
		show_reg_form();
		exit;
	}
	
	@$num_auth=$_POST["num_auth"];
	@$userid=$_POST["userid"];
	@$nickname=$_POST["username"];
	@$reg_email=$_POST["reg_email"];
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

	$_SESSION['num_auth'] = "";
	
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
</html>
<?php

function show_reg_form() {
	$SITENAME = BBS_FULL_NAME;
	echo <<<EOF
<html>
<meta HTTP-EQUIV="Content-Type" CONTENT="text/html; charset=gb2312">
<link rel=stylesheet type=text/css href='bbs.css'>
<nobr><center>$SITENAME -- 新用户注册<hr color=green>
<font color=green>欢迎加入$SITENAME  (有"*"号的必须填写，一个汉字作为两个字符计算)</font>
<script type="text/javascript">
<!--
	function checkEmpty(form, fieldID, fieldName) {
		if (form[fieldID].value == "") {
			alert('请输入您的' + fieldName + '!');
			form[fieldID].focus();
			return false;
		}
		return true;
	}
	function CheckDataValid(form) 
	{
		var fID = new Array("userid", "pass1", "num_auth", "realname", "dept", "address", "reg_email", "phone");
		var fName = new Array("用户名", "密码", "验证码", "真实姓名", "学校系级或工作单位", "详细通讯地址", "注册email", "联络电话");
		var len = fID.length;
		for (i = 0; i < len; i++) {
			if (!checkEmpty(form, fID[i], fName[i])) return false;
		}
		if (form["pass1"].value != form["pass2"].value) {
			alert("你两次输入的密码不一样：（"); 
			form["pass1"].focus(); 
			return false; 
		}
		return true;
	}
//-->
</script> 
<form method=post action="bbsreg.php" onsubmit="return CheckDataValid(this);" name="theForm">
<img src="img_rand/img_rand.php">
<table width=700>
  <TR>
	<TD align=right>*上面图片中的字符串:</TD>
	<TD align=left><INPUT size=5 name=num_auth>(为了防止机器人注册。如果辨认不清，可按F5 键刷新本页面)</TD> 
  </TR>
  <TR>
	<TD align=right>*请输入代号:</TD>
	<TD align=left><INPUT maxLength=12 size=12 name=userid> (2-12字符, 
	  可用英文字母或数字，首字符必须是字母) </TD>
  </TR>
  <TR>
	<TD align=right>*请输入密码:</TD>
	<TD align=left><INPUT maxLength=39 size=12 type=password name=pass1> (4-39字符)
  </TD>
  </TR>
  <TR>
	<TD align=right>*请输入确认密码:</TD>
	<TD align=left><INPUT maxLength=39 size=12 type=password name=pass2> (4-39字符, 
	  必须和密码一致) </TD>
  </TR>
  <TR>
	<TD align=right>请输入昵称:</TD>
	<TD align=left><INPUT maxLength=32 name=username> (2-39字符, 中英文不限) </TD>
  </TR>
  <TR>
	<TD align=right>*您的真实姓名:</TD>
	<TD align=left><INPUT name=realname> (请用中文, 至少2个汉字) </TD>
  </TR>
  <TR>
	<TD align=right>*学校系级或工作单位:</TD>
	<TD align=left><INPUT size=40 name=dept> (至少6个字符) </TD>
  </TR>
  <TR>
	<TD align=right>&nbsp;</TD>
	<TD align=left><font color=red>(学校请具体到系，工作单位请具体到部门)</font></TD>
  </TR>
  <TR>
	<TD align=right>*您的详细住址:</TD>
	<TD align=left><INPUT size=40 name=address> (至少6个字符) </TD>
  </TR>
  <TR>
	<TD align=right>&nbsp;</TD>
	<TD align=left><font color=red>(住址请具体到宿舍或者门牌号码)</font></TD>
  </TR>
  <TR>
	<TD align=right>您的性别:</TD>
	<TD align=left><SELECT name=gender><OPTION 
		selected>男</OPTION><OPTION>女</OPTION></SELECT></TD>
  </TR>
  <TR>
	<TD align=right>您的出生年月日: </TD>
	<TD align=left><INPUT maxLength=4 size=4 name=year>年<INPUT maxLength=2 
	  size=2 name=month>月<INPUT maxLength=2 size=2 name=day>日</TD>
  </TR>
  <TR>
	<TD align=right>*您的注册Email 地址:</TD>
	<TD align=left><INPUT size=60 name=reg_email></TD>
  </TR>
  <TR>
	<TD align=right>*您的联络电话:</TD>
	<TD align=left><INPUT size=40 name=phone> </TD>
  </TR>
</table>
<hr color=green>
<input type=submit value=申请> <input type=reset value=重新填写>
</form></center>
</html>
EOF;
}
?>