<?php


$needlogin=0;

require("inc/funcs.php"); 
setStat("用户登陆");

if ($_POST['action']=="doLogon") {
	doLogon();
	if (isErrFounded()){
		show_nav();
		head_var("用户登陆");
		html_error_quit(); 
	} 
} else {
	show_nav();
	head_var("用户登陆");
	showLogon();
}

show_footer();

function showLogon(){
?>
	<form action="<?php echo $_SERVER['PHP_SELF']; ?>" method=post> 
	<input type="hidden" name="action" value="doLogon">
	<table cellpadding=3 cellspacing=1 align=center class=tableborder1>
    <tr>
    <th valign=middle colspan=2 align=center height=25>请输入您的用户名、密码登陆</td></tr>
    <tr>
    <td valign=middle class=tablebody1>请输入您的用户名</td>
    <td valign=middle class=tablebody1><INPUT name=id type=text> &nbsp; <a href="register.php">没有注册？</a></td></tr>
    <tr>
    <td valign=middle class=tablebody1>请输入您的密码</font></td>
    <td valign=middle class=tablebody1><INPUT name=password type=password> &nbsp; <a href="foundlostpass.php">忘记密码？</a></td></tr>
    <tr>
    <td class=tablebody1 valign=top width=30% ><b>Cookie 选项</b><BR> 请选择你的 Cookie 保存时间，下次访问可以方便输入。</td>
    <td valign=middle class=tablebody1>
	<input type=radio name=CookieDate value=0 checked>不保存，关闭浏览器就失效<br>
                <input type=radio name=CookieDate value=1>保存一天<br>
                <input type=radio name=CookieDate value=2>保存一月<br>
                <input type=radio name=CookieDate value=3>保存一年<br>                
	</td></tr>
	<input type=hidden name=comeurl value="<?php echo $_SERVER['HTTP_REFERER']; ?>">
    <tr>
    <td class=tablebody2 valign=middle colspan=2 align=center><input type=submit name=submit value="登 陆"></td></tr></table>
</form>
<?php
}

function doLogon(){
	GLOBAL $loginok, $guestloginok;
	@$id = $_POST["id"];
	@$passwd = $_POST["password"];
	if ($id=='') {
		foundErr("请输入您的用户名");
		return false;
	}
	if  ( ($loginok==1) || ($guestloginok==1) ) {
		bbs_wwwlogoff();
	}
	if (($id!='guest') && (bbs_checkpasswd($id,$passwd)!=0)){
		foundErr("您的用户名并不存在，或者您的密码错误");
		return;
	}
	$ret=bbs_wwwlogin(1);
	switch ($ret) {
	case -1:
		foundErr("您已登陆的账号过多，无法重复登陆!");
		return false;
	case 3:
		foundErr("您的账号已被管理员禁用！");
		return false;
	case 4:
		foundErr("您所使用的IP已被本站禁用！");
		return false;
	case 5:
		foundErr("请勿频繁登陆!");
		return false;
	case 1:
		foundErr("系统在线人数已达上限，请稍后再访问本站。");
		return false;
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
	show_nav();
	head_var("用户登陆");
?>
<meta HTTP-EQUIV=REFRESH CONTENT='2; URL=<?php   echo $comeurl; ?>' >
<table cellpadding=3 cellspacing=1 align=center class=tableborder1 >
<tr>
<th height=25>登陆成功：<?php   echo $Forum_info[0]; ?>欢迎您的到来</th>
</tr>
<tr><td class=tablebody1><br>
<ul><?php   echo $comeurlname; ?><li><a href=index.php>返回首页</a></li></ul>
</td></tr>
</table>
<?php 
}
?>