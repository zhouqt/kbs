<?php
require("funcs.php");
login_init();

function bbs_pwd_form() {
?>  
<script language="JavaScript">
<!--
//document.write("<form action='https://"+window.location.hostname+"/bbspwd.php?do' method='post'>");
-->
</script>
<form action='/bbspwd.php?do' method='post'>
<center>
<table class="t1" width="500" cellspacing="0" cellpadding="5" border="0">
    <tr>
        <td class="t3">您的旧密码</td>
        <td class="t7"> <input maxlength="39" size="12" type="password" name="pw1" class="f1"></td>
    </tr>
    <tr>
        <td class="t3">您的新密码</td>
        <td class="t7"> <input maxlength="39" size="12" type="password" name="pw2" class="f1"></td>
    </tr>
    <tr>
        <td class="t3">再输入一次</td>
        <td class="t7"> <input maxlength="39" size="12" type="password" name="pw3" class="f1"></td>
    </tr>
    <tr>
        <td class="t2" colspan="2"><input type="submit" value="确定修改" class="f1"></td>
    </tr>
</table>
</center>
</form>
<?php    
}

if ($loginok != 1) {
    html_nologin();
    exit();
}
if(!strcmp($currentuser["userid"],"guest"))
    html_error_quit("请先登录");

html_init("gb2312","","",1);
?>
<body topmargin="0">
<table width="100%" border="0" cellspacing="0" cellpadding="3">
  <tr> 
    <td class="b2">
	    <a href="bbssec.php" class="b2"><?php echo BBS_FULL_NAME; ?></a>
	    -
	    <?php echo $currentuser["userid"]; ?>的工具箱
	    -
	    修改密码
    </td>
   </tr>
   <tr>
   <td height="20"> </td>
   </tr>
   <tr>
        <td align="center">
<?php
    if (isset($_GET['do'])) {
        $pass = $_POST['pw2'];
        if (strlen($pass) < 4 || strlen($pass) > 39)
            html_error_quit("新密码长度应为 4～39");
        if ($pass != $_POST['pw3'])
            html_error_quit("两次输入的密码不相同");
        if (bbs_checkpasswd($currentuser["userid"],$_POST['pw1']) != 0)
            html_error_quit("密码不正确");
        if (!bbs_setpassword($currentuser["userid"],$pass))
            html_error_quit("系统错误，请联系管理员");
?>
密码修改成功，您的新密码已设定<br /><br />
[<a href="/<?php echo MAINPAGE_FILE; ?>">返回首页</a>]
[<a href="javascript:history.go(-2);">快速返回</a>]
<?php
    }
    else
        bbs_pwd_form();
?>        
        </td>
   </tr>
</table>
<?php
html_normal_quit();
?>