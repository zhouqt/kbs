<?php

$needlogin=0;

require("inc/funcs.php"); 
setStat("用户登录");
show_nav();
echo "<br>";
head_var("用户登录");

if ($_POST['action']=="doLogon") {
	doLogon();
} else {
	showLogon();
}

show_footer();

function doLogon(){
	GLOBAL $loginok, $guestloginok;
	@$id = $_POST["id"];
	@$passwd = $_POST["password"];
	if ($id=='') {
		foundErr("请输入您的用户名");
	}
	if  ( ($loginok==1) || ($guestloginok==1) ) {
		bbs_wwwlogoff();
	}
	if (($id!='guest') && (bbs_checkpasswd($id,$passwd)!=0)){
		foundErr("您的用户名并不存在，或者您的密码错误");
	}
	$ret=bbs_wwwlogin(1);
	switch ($ret) {
	case -1:
		foundErr("您已登陆的账号过多，无法重复登陆!");
	case 3:
		foundErr("您的账号已被管理员禁用！");
	case 4:
		foundErr("您所使用的IP已被本站禁用！");
	case 5:
		foundErr("请勿频繁登陆!");
	case 1:
		foundErr("系统在线人数已达上限，请稍后再访问本站。");
	}
	$data=array();
    $num=bbs_getcurrentuinfo($data);
	$time=0;
	switch (intval($_POST['CookieDate'])) {
	case 1;
		$time=time()+86400; //24*60*60 sec
		break;
	case 2;
		$time=time()+2592000; //30*24*60*60 sec
		break;
	case 3:
		$time=time()+31536000; //365*24*60*60 sec
		break;
	}
	$path='';
	setcookie("W_UTMPKEY",$data["utmpkey"],time()+360000,$path);
	setcookie("W_UTMPNUM",$num,time()+360000,$path);
	setcookie("W_UTMPUSERID",$data["userid"],$time,$path);
	setcookie("W_LOGINTIME",$data["logintime"],0,$path);
	setcookie("W_PASSWORD",$passwd,$time,$path);

	if ((strpos(strtolower($_POST['comeurl']),'register.php')!==false) || (strpos(strtolower($_POST['comeurl']),'logon.php') !==false) || trim($_POST['comeurl'])=='')  {
		$comeurlname="";
		$comeurl="index.php";
	} else {
		$comeurl=$_POST['comeurl'];
		$comeurlname="<li><a href=".$_POST['comeurl'].">".$_POST['comeurl']."</a></li>";
	} 
?>
<meta HTTP-EQUIV=REFRESH CONTENT='2; URL=<?php   echo $comeurl; ?>' >
<table cellpadding=3 cellspacing=1 align=center class=TableBorder1 >
<tr>
<th height=25>登陆成功：<?php   echo $Forum_info[0]; ?>欢迎您的到来</th>
</tr>
<tr><td class=TableBody1><br>
<ul><?php   echo $comeurlname; ?><li><a href=index.php>返回首页</a></li></ul>
</td></tr>
</table>
<?php 
}
?>
