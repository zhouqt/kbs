<?php
require("inc/funcs.php");

require("inc/usermanage.inc.php");

require("inc/user.inc.php");

setStat("好友列表");

show_nav();

if ($loginok==1) {
	showUserMailbox();
	head_var($userid."的控制面板","usermanagemenu.php",0);
	if (preProcess()) {
		html_success_quit("查看所有好友列表", "friendlist.php");
	} else if (!isErrFounded()) {
		showUserManageMenu();
		main();
	}
}else {
	foundErr("本页需要您以正式用户身份登陆之后才能访问！");
}

if (isErrFounded()) {
	html_error_quit();
}

show_footer();

function preProcess() {
	global $_GET, $currentuser;
	
	if (isset($_GET["addfriend"])) {
		$friend = $_GET["addfriend"];
		$ret = bbs_add_friend( $friend ,"" );
		if($ret == -1) {
			foundErr("您没有权限设定好友或者好友个数超出限制");
			return false;
		} else if($ret == -2) {
			foundErr("$friend 本来就在你的好友名单中");
			return false;
		} else if($ret == -3) {
			foundErr("系统出错");
			return false;
		} else if($ret == -4) {
			foundErr("$friend 用户不存在");
			return false;
		} else{
			setSucMsg("$friend 已增加到您的好友名单中");
			return true;
		}
	} else if (isset($_GET["delfriend"])) {
		$friend = $_GET["delfriend"];
		$ret = bbs_delete_friend( $friend );
		$error = 2;
		if ($ret == 1) {
			foundErr("您没有设定任何好友");
			return false;
		} else if($ret == 2) {
			foundErr("$friend 本来就不在你的好友名单中");
			return false;
		} else if($ret == 3) {
			foundErr("删除失败");
			return false;
		} else {
			setSucMsg("$friend 已从您的好友名单中删除");
			return true;		
		}
	}
}

function main() {
	global $currentuser;
?>
<br>
<table cellpadding=3 cellspacing=1 align=center class=TableBorder1>
<tr>
<th valign=middle width=30 height=25>序号</th>
<th valign=middle width=100>用户账号</th>
<th valign=middle width=280>好友说明</th>
<th valign=middle width=150>操作</th>
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

	$i = 0;
	foreach($friends_list as $friend) {
		$i++;
?>
<tr>
<td class=TableBody1 align=center valign=middle>
<?php echo $startNum+$i; ?>
</td>
<td class=TableBody1 align=center valign=middle style="font-weight:normal">
<a href="dispuser.php?id=<?php echo $friend['ID'] ; ?>" target=_blank>
<?php echo $friend['ID'] ?></a>
</td>
<td class=TableBody1 align=left style="font-weight:normal"><a href="dispuser.php?id=<?php echo $friend['ID'] ; ?>" > <?php       echo htmlspecialchars($friend['EXP'],ENT_QUOTES); ?></a>	</td>
<td align=center valign=middle width=150 class=TableBody1>
<a href="friendlist.php?delfriend=<?php echo $friend['ID']; ?>">删除好友</a> |
<a href="sendmail.php?receiver=<?php echo $friend['ID']; ?>">发信问候</a>
</td>
</tr>
<?php
	}
?>
<tr>
<td align=center valign=middle colspan=3 class=TableBody2>
<form method="GET" action="friendlist.php">
<input type="text" name="addfriend">&nbsp;<input type="submit" name="submit" value="添加好友">
</td>
<td align=right valign=middle colspan=1 class=TableBody2>
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
<?php
}


?>
