<?php
require("inc/funcs.php");

require("inc/usermanage.inc.php");

require("inc/user.inc.php");

setStat("用户自定义参数");

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
	global $user_define,$user_define_num;
	require("inc/userdatadefine.inc.php");
?>
<form action="saveuserparam.php" method=POST name="theForm">
<table cellpadding=3 cellspacing=1 border=0 align=center class=TableBorder1>
<tr> 
      <th colspan="2" width="100%">用户个人参数（www方式）</th>
 </tr> 
 <?php
	$userparam=bbs_getuserparam();
	for ($i=$user_define_num-1;$i>=0;$i--){
		if ($user_define[$i][0]!=1) 
			continue;
		$flag=1<<$i;
?>
<tr><td width="40%" class=TableBody1><B><?php echo $user_define[$i][1]; ?></B>：<BR><?php echo $user_define[$i][2]; ?></td>   
        <td width="60%" class=TableBody1>    
			<input type="radio" name="param<?php echo $i; ?>" value="1" <?php if ($userparam & $flag) echo "checked"; ?> ><?php echo $user_define[$i][3]; ?>
			<input type="radio" name="param<?php echo $i; ?>" value="0" <?php if (!($userparam & $flag)) echo "checked"; ?> ><?php echo $user_define[$i][4]; ?>
        </td>   
</tr>
<?php
	}
?>
<tr> 
      <th colspan="2" width="100%">用户个人参数（telnet方式）</th>
 </tr> 
<?php
	$userparam=bbs_getuserparam();
	for ($i=0;$i<$user_define_num;$i++){
		if ($user_define[$i][0]!=0) 
			continue;
		$flag=1<<$i;
?>
<tr><td width="40%" class=TableBody1><B><?php echo $user_define[$i][1]; ?></B>：<BR><?php echo $user_define[$i][2]; ?></td>   
        <td width="60%" class=TableBody1>    
			<input type="radio" name="param<?php echo $i; ?>" value="1" <?php if ($userparam & $flag) echo "checked"; ?> ><?php echo $user_define[$i][3]; ?>
			<input type="radio" name="param<?php echo $i; ?>" value="0" <?php if (!($userparam & $flag)) echo "checked"; ?> ><?php echo $user_define[$i][4]; ?>
        </td>   
</tr>
<?php
	}
?>
<tr align="center"> 
<td colspan="2" width="100%" class=TableBody2>
<input type=Submit value="更 新" name="Submit"> &nbsp; <input type="reset" name="Submit2" value="清 除">
</td></tr>
</table></form>
<?php
}
?>
