<?php
require("inc/funcs.php");

require("inc/usermanage.inc.php");

require("inc/user.inc.php");

setStat("修改密码");

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

function main() {
?>
<form action="dochangepasswd.php" method=POST name="theForm">
<table cellpadding=3 cellspacing=1 align=center class=TableBorder1>
<tr> 
      <th colspan="2" width="100%">用户密码资料
      </th>
    </tr>
<tr>    
        <td width="40%" class=TableBody1><B>旧密码确认</B>：<BR>如要修改请输入旧密码进入确认</td>   
        <td width="60%" class=TableBody1>    
          <input type="password" name="oldpsw" value="" size=30 maxlength=13>   
        </td>   
      </tr>  
    <tr>    
        <td width="40%" class=TableBody1><B>新密码</B>：<BR>如要修改请直接输入新密码进入更新</td>   
        <td width="60%" class=TableBody1>    
          <input type="password" name="psw" value="" size=30 maxlength=13>   
        </td>   
      </tr>   
    <tr>    
        <td width="40%" class=TableBody1><B>新密码确认</B>：<BR>再输一次新密码防止输入错误</td>   
        <td width="60%" class=TableBody1>    
          <input type="password" name="psw2" value="" size=30 maxlength=13>   
        </td>   
      </tr>   
    <tr align="center"> 
      <td colspan="2" width="100%"  class=TableBody2>
            <input type=Submit value="更 新" name="Submit"> &nbsp; <input type="reset" name="Submit2" value="清 除">
      </td>
    </tr>

    </table></form>
<?php

	return false;
}

?>
