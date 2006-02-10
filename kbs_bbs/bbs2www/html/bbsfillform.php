<?php
	require("funcs.php");
	login_init();
	html_init("gb2312");

	if(!strcmp($currentuser["userid"],"guest"))
		html_error_quit("请申请另外的帐号填写注册单!");

	if (!isset($_POST["realname"])) {
		show_fill_form();
		exit;
	}

	@$realname=$_POST["realname"];
	@$dept=$_POST["dept"];
	@$address=$_POST["address"];
	@$year=$_POST["year"];
	@$month=$_POST["month"];
	@$day=$_POST["day"];
	@$email=$_POST["email"];
	@$phone=$_POST["phone"];
	@$gender=$_POST["gender"];
	@$mobile_phone=$_POST["mobile"];

	//用户已经通过注册
	//未满等待时间(先放到phplib里面做了)
	if(!strcmp($gender,"男"))$gender=1;
    else $gender=2;
	settype($year,"integer");
	settype($month,"integer");
	settype($day,"integer");
    if (BBS_WFORUM==0)  {
        $ret=bbs_createregform($currentuser["userid"],$realname,$dept,$address,$gender,$year,$month,$day,$email,$phone,$mobile_phone,FALSE);//自动生成注册单
    } else {
        $ret=@bbs_createregform($currentuser["userid"],$realname,$dept,$address,$gender,$year,$month,$day,$email,$phone,$mobile_phone, $_POST['OICQ'], $_POST['ICQ'], $_POST['MSN'],  $_POST['homepage'], intval($_POST['face']), $_POST['myface'], intval($_POST['width']), intval($_POST['height']), intval($_POST['groupname']), $_POST['country'],  $_POST['province'], $_POST['city'], intval($_POST['shengxiao']), intval($_POST['blood']), intval($_POST['belief']), intval($_POST['occupation']), intval($_POST['marital']), intval($_POST['education']), $_POST['college'], intval($_POST['character']), FALSE);//自动生成注册单
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
?>
<body>
注册单已经提交,24小时内站务将会审核,如果通过,你就会获得合法用户权限！<br>
<a href="javascript:history.go(-1)">快速返回</a>
</body>
</html>
<?php

function show_fill_form()
{
	$SITENAME = BBS_FULL_NAME;
	echo <<<EOF
<html>
<meta HTTP-EQUIV="Content-Type" CONTENT="text/html; charset=gb2312">
<link rel=stylesheet type=text/css href='bbs.css'>
<script type="text/javascript">
function check_field(fieldId,fieldName,minLength) {
	var fieldValue = document.regform.elements[fieldId].value; 
	if (fieldValue == '') {
		alert('请输入您的' + fieldName + '!');
		return false;
	}
	if (minLength != 0) {
		if (fieldValue.length < minLength) {
			alert('请认真填写您的' + fieldName + '!');
			return false;
		}
	}
	return true;
}
	
function check_reg_form() {
	var fields = new Array();
	fields = Array(0,1,2,7);
	var fieldsMinLenths = new Array();
	fieldsMinLenths = Array(2,4,4,7);
	var fieldsNames = new Array();
	fieldsNames = Array('真实姓名','系别或工作单位','详细住址','联系电话');
	var fieldsNum = fields.length;
	var i;
	for ( i = 0 ; i < fieldsNum ; i ++ ) {
		if (!check_field(fields[i],fieldsNames[i],fieldsMinLenths[i]))
			return false;	
	}
	document.regform.submit();
	return true;
}
</script>
<nobr><center>$SITENAME -- 注册单填写<hr color=green>
<font color=green>所填写的资料系统都会为您保密, 请如实填写, 注册单在本站站务手工认证通过以后，你就将成为本站合法用户。</font>
<br /><br />
<form method=post action="bbsfillform.php" name="regform" id="regform" />
<table width=600>
<tr><td align=right>*您的真实姓名:<td align=left><input name="realname" id="realname" size=20> (请用中文, 至少2个汉字)
<tr><td align=right>*学校系级或工作单位:<td align=left><input name="dept" id="dept" size=40> (至少6个字符)
<tr><td align=right>*您的详细住址:<td align=left><input name="address" id="address" size=40> (至少6个字符)
<tr><td align=right>您的性别:<td align=left><select name=gender><option>男</option><option>女</option>
<tr><td align=right>您的出生年月日:
<td align=left><input name=year size=4 maxlength=4>年<input name=month size=2 maxlength=2>月<input name=day size=2 maxlength=2>日<br>
<!--
<tr><td align=right>*您的Email地址(<font color=red>请填写您激活本帐户所用的Email</font>):<td align=left><input name=email size=40>
-->
<tr><td align=right>*您的联络电话:<td align=left><input name="phone" id="phone" size=40> 
<tr><td align=right>手机号码:<td align=left><input name=mobile size=40> 
</table><br>
<hr color=green>
<input type="button" value="提交注册单" onclick="check_reg_form()">
<input type=reset value=重新填写>
</form></center>
</html>
EOF;
}
?>