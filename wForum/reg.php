<?php
	$needlogin=0;
	require("inc/funcs.php");
	$stats="新用户注册";
		show_nav();
	@$action=$_POST['action'];
	if ($action=='apply') {
		$stats="填写资料";
		do_apply();
	} elseif ($action=='save') {
		$stats="提交注册";
		do_save();
	} else {
		$stats="注册协议";
		do_show();
	}

show_footer();

function do_show() {
?>
<table cellpadding=3 cellspacing=1 align=center class=tableborder1>
    <tr><th align=center><form action="reg.php" method=post>服务条款和声明</td></tr>
	<input type="hidden" name="action" value="apply">
    <tr><td class=tablebody1 align=left>
<?php	require("inc/reg_txt.php") ; ?>
	</td></tr>
    <tr><td align=center class=tablebody2><input type=submit value=我同意></td></form></tr>
</table>
<?php


}

function do_apply(){
	global $SiteName;
?>

<form method=post action="reg.php">
<input type="hidden" name="action" value="save">
<table cellpadding=3 cellspacing=1 align=center class=tableborder1>
<thead>
<Th colSpan=2 height=24><?php echo $SiteName; ?> -- 新用户注册</Th>
</thead>
<TBODY> 
<TR> 
<TD width=40% class=tablebody1><B>代号</B>：<BR>2-12字符，可用英文字母或数字，首字符必须是字母</TD>
<TD width=60%  class=tablebody1> 
<input name=userid size=12 maxlength=12></TD>
</TR>
<TR> 
<TD width=40% class=tablebody1><B>密码</B>：<BR>请输入密码，5-39字符，区分大小写。<BR>
请不要使用任何类似 '*'、' ' 或 HTML 字符</TD>
<TD width=60%  class=tablebody1> 
<input type=password name=pass1 size=12 maxlength=12></TD>
</TR>
<TR> 
<TD width=40% class=tablebody1><B>密码</B>：<BR>请再输一遍确认</TD>
<TD width=60%  class=tablebody1> 
<input type=password name=pass2 size=12 maxlength=12></TD>
</TR>
<TR> 
<TD width=40% class=tablebody1><B>昵称</B>：<BR>您在BBS上的昵称，2-39字符，中英文不限</TD>
<TD width=60%  class=tablebody1> 
<input name=username size=20 maxlength=32></TD>
</TR>
<TR> 
<TD width=40% class=tablebody1><B>真实姓名</B>：<BR>请用中文, 至少2个汉字</TD>
<TD width=60%  class=tablebody1> 
<input name=realname size=20></TD>
</TR>
<TR> 
<TD width=40% class=tablebody1><B>性别</B>：<BR>请选择您的性别</TD>
<TD width=60%  class=tablebody1> <INPUT type=radio CHECKED value=1 name=gender>
<IMG  src=pic/Male.gif align=absMiddle>男孩 &nbsp;&nbsp;&nbsp;&nbsp; 
<INPUT type=radio value=0 name=gender>
<IMG  src=pic/Female.gif align=absMiddle>女孩</font></TD>
</TR>
<TR> 
<TD width=40% class=tablebody1><B>生日</B>：<BR>您的生日</TD>
<TD width=60%  class=tablebody1> 
<input name=year size=4 maxlength=4>年<input name=month size=2 maxlength=2>月<input name=day size=2 maxlength=2>日<br></TD>
</TR>
<TR> 
<TD width=40% class=tablebody1><B>Email</B>：<BR>您的有效电子邮件地址</TD>
<TD width=60%  class=tablebody1> 
<input name=email size=40></TD>
</TR>
<TR> 
<TD width=40% class=tablebody1><B>学校系级或工作单位</B>：<BR>请用中文，至少6个字符</TD>
<TD width=60%  class=tablebody1> 
<input name=dept size=40></TD>
</TR>
<TR> 
<TD width=40% class=tablebody1><B>详细通讯地址</B>：<BR>请用中文，至少6个字符</TD>
<TD width=60%  class=tablebody1> 
<input name=address size=40></TD>
</TR>
<TR> 
<TD width=40% class=tablebody1><B>联络电话</B>：<BR>您的联络电话，请写明区号</TD>
<TD width=60%  class=tablebody1> 
<input name=phone size=40> </TD>
</TR>
<tr>
<td colspan=2 align=center>
<input type=submit value=提交表格>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; <input type=reset value=重新填写>
</td>
</tr>
</table>
</form>
<?php
}

function do_save(){
	global $SiteName;
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
	@$gender=$_POST["gender"];


	if(strcmp($pass1,$pass2))
		html_error_quit("两次密码输入不一样");
	else if(strlen($pass1) < 5 || !strcmp($pass1,$userid))
       	html_error_quit("密码长度太短或者和用户名相同!");

	$ret=bbs_createnewid($userid,$pass1,$nickname);
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
	if($gender!='1')$gender=2;
    settype($year,"integer");
	settype($month,"integer");
	settype($day,"integer");
	$ret=bbs_createregform($userid,$realname,$dept,$address,$gender,$year,$month,$day,$email,$phone,FALSE);//自动生成注册单

	switch($ret)
	{
	case 0:
		break;
	case 2:
		html_error_quit("该用户不存在!");
		break;
	case 3:
		html_error_quit("参数错误");
		break;
	default:
		html_error_quit("未知的错误!");
		break;
	}
?>
<table cellpadding=3 cellspacing=1 align=center class=tableborder1>
<tr>
<th height=24>注册单已成功：<?php echo $SiteName; ?>欢迎您的到来</th>
</tr>
<tr><td class=tablebody1><br>
<ul>
<li>你现在还没有通过身份认证，,只有最基本的权限，不能发文、发信、聊天等。</li>
<li>系统会自动生成注册单，待站长审核通过后,你将获得合法用户权限！</li>
<li><a href="index.asp">进入讨论区</a></li></ul>
</td></tr>
</table>
<?php
}

?>

</body>
</html>
