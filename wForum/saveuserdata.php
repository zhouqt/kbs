<?php
require("inc/funcs.php");

require("inc/usermanage.inc.php");

require("inc/user.inc.php");

setStat("基本资料修改");

show_nav();

if ($loginok==1) {
?>
<table border="0" width="97%">
<?php
	showUserMailbox();
?>
</table>
<?php
}

head_var($userid."的控制面板","usermanagemenu.php",0);

if ($loginok==1) {
	showUserManageMenu();
	main();
}else {
	foundErr("本页需要您以正式用户身份登陆之后才能访问！");
}

if (isErrFounded()) {
		html_error_quit();
} 

show_footer();

function main(){
	global $currentuser;
	require("inc/userdatadefine.inc.php");
	global $SiteName;
	@$userid=$_POST["userid"];
	@$nickname=$_POST["username"];

	@$realname=$_POST["realname"];

    @$address=$_POST["address"];
	@$year=$_POST["year"];
	@$month=$_POST["month"];
	@$day=$_POST["day"];
	@$email=$_POST["email"];
	@$phone=$_POST["userphone"];
	@$mobile_phone=$_POST["mobile"];
	@$gender=$_POST["gender"];
	if($gender!='1')$gender=2;
    settype($year,"integer");
	settype($month,"integer");
	settype($day,"integer");

$ret=bbs_saveuserdata($currentuser['userid'],$realname,$address,$gender,$year,$month,$day,$email,$phone,$mobile_phone, $_POST['OICQ'], $_POST['ICQ'], $_POST['MSN'],  $_POST['homepage'], intval($_POST['face']), $_POST['myface'], intval($_POST['width']), intval($_POST['height']), intval($_POST['groupname']), $_POST['country'],  $_POST['province'], $_POST['city'], intval($_POST['shengxiao']), intval($_POST['blood']), intval($_POST['belief']), intval($_POST['occupation']), intval($_POST['marital']), intval($_POST['education']), $_POST['college'], intval($_POST['character']), $_POST['userphoto'],FALSE);//自动生成注册单

	switch($ret)
	{
	case 0:
		break;
	case -1:
		foundErr("用户自定义图像宽度错误");
		break;
	case -2:
		foundErr("用户自定义图像高度错误");
		break;
	case 3:
		foundErr("该用户不存在!");
		break;
	default:
		foundErr("未知的错误!");
		break;
	}
	if (isErrFounded() ){
		return false;
	}
	$signature=trim($_POST["Signature"]);
	if ($signature!='') {
		$filename=bbs_sethomefile($userid,"signatures");
		$fp=@fopen($filename,"w+");
		if ($fp!=false) {
			fwrite($fp,str_replace("\r\n", "\n", $signature));
			fclose($fp);
		}
	}
	$personal=trim($_POST["personal"]);
	if ($signature!='') {
		$filename=bbs_sethomefile($userid,"plans");
		$fp=@fopen($filename,"w+");
		if ($fp!=false) {
			fwrite($fp,str_replace("\r\n", "\n", $personal));
			fclose($fp);
		}
	}
	setSucMsg("您的数据已成功修改！");
	return html_success_quit('返回控制面板', 'usermanagemenu.php');
}
?>
