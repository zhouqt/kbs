<?php
require("inc/funcs.php");

require("inc/usermanage.inc.php");

require("inc/user.inc.php");

setStat("好友列表");

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
?>
<br>
<form action="usermailoperations.php" method=post id="oForm">
<input type="hidden" name="boxname" value="<?php echo $boxName; ?>">
<table cellpadding=3 cellspacing=1 align=center class=TableBorder1>
<tr>
<th valign=middle width=30 height=25>序号</th>
<th valign=middle width=100>用户账号</th>
<th valign=middle width=280>好友说明</th>
<th valign=middle width=130>操作</th>
</tr>
<?php
	define("USERSPERPAGE", 20); //ToDo: USERSPERPAGE should always be 20 here because of phplib - atppp
	$total_friends = bbs_countfriends($currentuser["userid"]);
	if( isset( $_GET["start"] ) ){
		$startNum = $_GET["start"];
		if ($startNum >= $total_friends) $startNum = $total_friends - USERSPERPAGE;
		if ($startNum < 0) $startNum = 0;
	} else {
		$startNum = 0;
	}
	$friends_list = bbs_getfriends($currentuser["userid"], $startNum);
    
	$count = count ( $friends_list );

	for ( $i=0; $i<$count ; $i++ ) {
?>
<tr>
<td class=TableBody1 align=center valign=middle>
<?php echo $startNum+$i+1; ?>
</td>
<td class=TableBody1 align=center valign=middle style="font-weight:normal">
<a href="dispuser.php?id=<?php echo $friends_list[$i]['ID'] ; ?>" target=_blank>
<?php echo $friends_list[$i]['ID'] ?></a>
</td>
<td class=TableBody1 align=left style="font-weight:normal"><a href="dispuser.php?id=<?php echo $friends_list[$i]['ID'] ; ?>" > <?php       echo htmlspecialchars($friends_list[$i]['EXP'],ENT_QUOTES); ?></a>	</td>
<td align=center valign=middle width=130 class=TableBody1>
<a href="#">删除好友</a> [ToDo]
</td>
</tr>
<?php
	}
?>
<tr>
<td align=right valign=middle colspan=4 class=TableBody2>
<?php
			
		if ($startNum > 0)
		{
			$i = $startNum - USERSPERPAGE;
			if ($i < 0) $i = 0;
			echo ' [<a href=friendlist.php>第一页</a>] ';
			echo ' [<a href=friendlist.php?start='.$i.'>上一页</a>] ';
		} else {
?>
<font color=gray>[第一页]</font>
<font color=gray>[上一页]</font>
<?php 
		}
		if ($startNum < $total_friends - USERSPERPAGE)
		{
			$i = $startNum + USERSPERPAGE;
			if ($i >= $total_friends) $i = $total_friends - USERSPERPAGE;
			echo ' [<a href=friendlist.php?start='.$i.'>下一页</a>] ';
			echo ' [<a href=friendlist.php?start='.($total_friends - USERSPERPAGE).'>最后一页</a>] ';
		} else {
?>
<font color=gray>[下一页]</font>
<font color=gray>[最后一页]</font>
<?php
		}
?>
<br>
您共有 <b><?php echo $total_friends ; ?></b> 位好友。
</td>
</tr>
</table>
</form>
<?php
}


?>
