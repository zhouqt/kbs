<?php

$needlogin=0;

require("inc/funcs.php"); 
setStat("用户登录");

global $comeurl;
	
if (isset($_POST["id"])) {
	doLogon();
} else {
	show_nav();
	echo "<br>";
	head_var();
	showLogon();
}

show_footer();

function doLogon(){
	GLOBAL $loginok, $guestloginok;
	global $SiteName;
	global $comeurl;
	
	if ((strpos(strtolower($_POST['comeurl']),'register.php')!==false) || (strpos(strtolower($_POST['comeurl']),'bbsleft.php')!==false) || (strpos(strtolower($_POST['comeurl']),'logon.php') !==false) || trim($_POST['comeurl'])=='')  {
		$comeurlname="";
		$comeurl="index.php";
	} else {
		$comeurl=$_POST['comeurl'];
		$comeurlname="<li><a href=\"".$comeurl."\">".$comeurl."</a></li>";
	}
	
	@$id = $_POST["id"];
	@$passwd = $_POST["passwd"];
	$cookieDate = 0;
	$cookieDate = intval($_POST['CookieDate']);
	if ($id=='') {
		errorQuit("请输入您的用户名。");
	}
	if  ( ($loginok==1) || ($guestloginok==1) ) {
		bbs_wwwlogoff();
	}
	if (($id!='guest') && (bbs_checkpasswd($id,$passwd)!=0)){
		errorQuit("您的用户名并不存在，或者您的密码错误。");
	}
	if (AUTO_KICK) {
		$kick = 1;
	} else {
		if (isset($_POST["kick"])) $kick = 1;
		else $kick = 0;
	}
	$ret=bbs_wwwlogin($kick);
	switch ($ret) {
	case -1:
		if (AUTO_KICK) {
			errorQuit("您已登录的账号过多，无法重复登录!");
		} else {
			promptKick($id, $passwd, $comeurl, $cookieDate);
		}
	case 3:
		errorQuit("您的账号已被管理员禁用！");
	case 4:
		errorQuit("您所使用的IP已被本站禁用！");
	case 5:
		errorQuit("请勿频繁登录!");
	case 1:
		errorQuit("系统在线人数已达上限，请稍后再访问本站。");
	case 7:
		errorQuit("对不起,当前位置不允许登录该ID。");
	}
	$data=array();
	$num=bbs_getcurrentuinfo($data);
	$time=0;
	switch ($cookieDate) {
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
	setcookie(COOKIE_PREFIX."UTMPKEY",$data["utmpkey"],time()+3600,COOKIE_PATH);
	setcookie(COOKIE_PREFIX."UTMPNUM",$num,time()+3600,COOKIE_PATH);
	setcookie(COOKIE_PREFIX."UTMPUSERID",$data["userid"],$time,COOKIE_PATH);
	setcookie(COOKIE_PREFIX."LOGINTIME",$data["logintime"],0,COOKIE_PATH);
	if ($time!=0) {
		$u = array();
		if (bbs_getcurrentuser($u) > 0) {
			setcookie(COOKIE_PREFIX."PASSWORD",base64_encode($u["md5passwd"]),$time,COOKIE_PATH);
		}
	}

	show_nav(false);
	echo "<br>";
	head_var();
?>
<meta HTTP-EQUIV=REFRESH CONTENT='2; URL=<?php   echo $comeurl; ?>' >
<script language="JavaScript">
<!--
    refreshLeft();
//-->
</script>
<table cellpadding=3 cellspacing=1 align=center class=TableBorder1 style="width: 75%;">
<tr>
<th height=25>登录成功：<?php echo $SiteName; ?>欢迎您的到来</th>
</tr>
<tr><td class=TableBody1><br>
<ul><?php   echo $comeurlname; ?><li><a href=index.php>返回首页</a></li></ul>
</td></tr>
</table>
<?php 
}

function errorQuit($errMsg) {
	global $comeurl;

	show_nav(false);
	echo "<br>";
	head_var();
?>
<table cellpadding=3 cellspacing=1 align=center class=TableBorder1 style="width: 75%;">
<tr align=center>
<th height=25>论坛错误信息</th>
</td>
</tr>
<tr>
<td class=TableBody1>
<b>产生错误的可能原因：</b>
<ul>
<li><?php   echo $errMsg; ?></li>
</ul>
</td></tr></table>
<?php
	showLogon(1, $comeurl);
	show_footer(false, false);
	exit;
}

function promptKick($id, $passwd, $comeurl, $cookieDate) {
	global $comeurl;

	show_nav(false);
	echo "<br>";
	head_var();
?>
<form name="infoform" action="logon.php" method="post">
<input type="hidden" name="id" value="<?php echo htmlspecialchars($id); ?>">
<input type="hidden" name="password" value="<?php echo htmlspecialchars($passwd); ?>">
<input type="hidden" name="comeurl" value="<?php echo htmlspecialchars($comeurl); ?>">
<input type="hidden" name="CookieDate" value="<?php echo $cookieDate; ?>">
<input type="hidden" name="kick" value="1">
<table cellpadding=3 cellspacing=1 align=center class=TableBorder1 style="width: 75%;">
<tr align=center>
<th height=25>论坛提示信息</th>
</td>
</tr>
<tr>
	<td class=TableBody1>
		<b>产生错误的可能原因：</b>
		<ul><li>你登录的窗口过多，点击 确定登录 踢出多余的登录。</li></ul>
	</td>
</tr>
<tr>
	<td class=TableBody2 valign=middle colspan=2 align=center>
		<input type=submit name=submit value="确定登录">&nbsp;&nbsp;
		<input type=button name="back" value="返 回" onclick="location.href='<?php echo htmlspecialchars($comeurl, ENT_QUOTES); ?>'">
	</td>
</tr>
</table>
</form>
<?php
	show_footer(false, false);
	exit;

}
?>
