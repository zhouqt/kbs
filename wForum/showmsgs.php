<?php


require("inc/funcs.php");

require("inc/usermanage.inc.php");

require("inc/user.inc.php");

setStat("用户短信服务");

show_nav();

if ($loginok==1) {
?>
<table border="0" width="97%">
<?php
	showUserMailbox();
?>
</table>
<?php
	head_var($userid."的控制面板","usermanagemenu.php",0);
	main();
}else {
	foundErr("本页需要您以正式用户身份登陆之后才能访问！");
}

if (isErrFounded()) {
		html_error_quit();
}

show_footer();

function main() {
	global $currentuser;
	$msgs=bbs_getwebmsgs();
	$num=count($msgs);
?>
<table cellpadding=3 cellspacing=1 align=center class=TableBorder1 width="97%"><tr><th colspan=3>短消息记录</td></tr>
<?php
	if ($num==0) {
?>
      <tr><td  class=TableBody1>您在本次访问本站期间没有收到任何短消息</td></tr>
<?php
	} else {
		for ($i=0;$i<$num;$i++) {
?>
     <tr> 
         <td class=TableBody2>
<?php
			if ($msg[$i]['SENT']) 
				echo '您于'.strftime('%Y-%m-%d %H:%M:%S', $msgs[$i]['TIME']).'发送给<b>'.$msgs[$i]['ID'].'</b>的短消息：';
			else 
				echo '<b>'.$msgs[$i]['ID'].'</b>于'.strftime('%Y-%m-%d %H-%M-%S', $msgs[$i]['TIME']).'发送给您的短消息：';
?>
          </td>
          </tr>
           <tr> 
            <td  class=TableBody1>
			<?php echo $msgs[$i]['content']; ?>
            </td>
          </tr>
<?php
		}
?>
           <tr><td  class=TableBody2 align="middle"><form method="post" action="mailmsgs.php"><input type="submit" value="将记录保存到信箱中"></form></td></tr>
<?php
	}
?>
        </table>
<?php
}
?>
