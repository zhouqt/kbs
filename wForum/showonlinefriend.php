<?php

require("inc/funcs.php");
require("inc/usermanage.inc.php");
require("inc/user.inc.php");

setStat("在线好友列表");

requireLoginok();

show_nav();

showUserMailBox();
head_var("谈天说地","usermanagemenu.php",0);
main();

show_footer();

function main() {
?>
<form action="" method=post id="oForm">
<table cellpadding=3 cellspacing=1 align=center class=TableBorder1>
<tr>
<th valign=middle width=30 height=25>序号</th>
<th valign=middle width=100>用户账号</th>
<th valign=middle width=*>用户昵称</th>
<th valign=middle width=120>用户上线地址</th>
<th valign=middle width=50>发呆</th>
<th valign=middle width=220>操作</th>
</tr>
<?php
	$online_user_list = bbs_getonlinefriends();
    
	$count = count ( $online_user_list );

	$i = 0;
	foreach($online_user_list as $friend) {
		$i++;
?>
<tr>
<td class=TableBody1 align=center valign=middle>
<?php echo $i; ?>
</td>
<td class=TableBody1 align=center valign=middle style="font-weight:normal">
<a href="dispuser.php?id=<?php echo $friend['userid'] ; ?>" target=_blank>
<?php echo $friend['userid'] ?></a>
</td>
<td class=TableBody1 align=left style="font-weight:normal"><a href="dispuser.php?id=<?php echo $friend['userid'] ; ?>" > <?php echo htmlspecialchars($friend['username'],ENT_QUOTES); ?></a>	</td>
<td align=center class=TableBody1 style="font-weight:normal"><?php echo $friend['userfrom']; ?></td>
<td align=center class=TableBody1 style="font-weight:normal"><?php printf('%02d:%02d',intval($friend['idle']/60), ($friend['idle']%60)); ?></td>
<td align=center valign=middle width=220 class=TableBody1><nobr>
<a target="_blank" href="friendlist.php?delfriend=<?php echo $friend['userid']; ?>">删除好友</a>
| <a href="sendmail.php?receiver=<?php echo $friend['userid']; ?>">发信问候</a>
| <a href="javascript:replyMsg('<?php echo $friend['userid'] ; ?>')">发送消息</a>
<!-- | <a href="#">发送短信</a> -->
</nobr></td>
</tr>
<?php
	}
?>
<tr>
<td align=right valign=middle colspan=6 class=TableBody2>
<a href="friendlist.php">编辑好友名册</a>。您共有 <b><?php echo $count; ?></b> 位好友在线。
</td>
</tr>
</table>
</form>
<?php
}


?>
