<?php

require("inc/funcs.php");
require("inc/usermanage.inc.php");
require("inc/user.inc.php");

setStat("修改昵称密码");

requireLoginok();

show_nav();

showUserMailbox();
head_var($userid."的控制面板","usermanagemenu.php",0);
showUserManageMenu();
main();

show_footer();

function main() {
	global $currentuser;
?>
<br>
<form action="dochangepasswd.php" method="POST" name="theForm">
<table cellpadding=3 cellspacing=1 align=center class=TableBorder1>
<tr> 
      <th colspan="2" width="100%">用户昵称
      </th>
    </tr>
<tr>    
    <td width="40%" class="TableBody1" align="right"><b>您的昵称：</b>
    <td width="60%" class="TableBody1">    
      <input type="text" name="nick" value="<?php echo htmlspecialchars($currentuser['username'],ENT_QUOTES); ?>" size="24"> &nbsp; &nbsp; <input type="checkbox" value="1" name="chkTmp">临时修改昵称（在线用户列表中有效）
    </td>
</tr>  
    <tr align="center"> 
      <td colspan="2" width="100%"  class=TableBody2>
            <input type="Submit" value="修 改">
      </td>
    </tr>

    </table></form>
<br>
<form action="dochangepasswd.php" method="POST" name="theForm">
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
