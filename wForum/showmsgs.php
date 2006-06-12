<?php

require("inc/funcs.php");
require("inc/usermanage.inc.php");
require("inc/user.inc.php");

setStat("用户短信服务");

requireLoginok();

show_nav();

showUserMailbox();
head_var($userid."的控制面板","usermanagemenu.php",0);
main();

show_footer();

function main() {
	$msgs=bbs_getwebmsgs();
	$num=count($msgs);
?>
<form method="post" action="mailmsgs.php">
<table cellpadding=3 cellspacing=1 align=center class=TableBorder1 width="97%"><tr><th colspan=3>短消息记录</td></tr>
<?php
	if ($num==0) {
?>
      <tr><td  class=TableBody1 align="center">您在本次访问本站期间没有收到任何短消息</td></tr>
<?php
	} else {
		for ($i=0;$i<$num;$i++) {
?>
     <tr> 
         <td class=TableBody2>
<?php
			if (!$msgs[$i]['SENT']) 
				echo '您于'.strftime('%Y-%m-%d %H:%M:%S', $msgs[$i]['TIME']).'发送给 <b>'.$msgs[$i]['ID'].'</b> 的短消息：';
			else {
				if ($msgs[$i]['MODE'] == 3)
					echo '<b>站长</b> 于 '.strftime('%Y-%m-%d %H:%M:%S', $msgs[$i]['TIME']).' 广播：';
				else
					echo '<b>'.$msgs[$i]['ID'].'</b> 于 '.strftime('%Y-%m-%d %H-%M-%S', $msgs[$i]['TIME']).' 发送给您的短消息：[<a href="javascript:replyMsg(\''.$msgs[$i]['ID'].'\')">回消息</a>]';
			}
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
           <tr><td  class=TableBody2 align="middle"><input type="submit" value="将记录保存到信箱中"></td></tr>
<?php
	}
?>
</table></form>
<?php
}
?>
