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
	preProcess();
	main();
}else {
	foundErr("本页需要您以正式用户身份登陆之后才能访问！");
}

if (isErrFounded()) {
	html_error_quit();
}

show_footer();

function preProcess() {
	global $_GET, $currentuser;
	$error = 0; //0: nothing happened; 1: OK; 2: error
	$msg = "";
	
	if (isset($_GET["addfriend"])) {
		$friend = $_GET["addfriend"];
		$ret = bbs_add_friend( $friend ,"" );
		$error = 2;
		if($ret == -1) {
			$msg = "您没有权限设定好友或者好友个数超出限制";
		} else if($ret == -2) {
			$msg = "$friend 本来就在你的好友名单中";
		} else if($ret == -3) {
			$msg = "系统出错";
		} else if($ret == -4) {
			$msg = "$friend 用户不存在";
		} else{
			$msg = "$friend 已增加到您的好友名单中";
			$error = 1;
		}
	} else if (isset($_GET["delfriend"])) {
		$friend = $_GET["delfriend"];
		$ret = bbs_delete_friend( $friend );
		$error = 2;
		if ($ret == 1) {
			$msg = "您没有设定任何好友";
		} else if($ret == 2) {
			$msg = "$friend 本来就不在你的好友名单中";
		} else if($ret == 3) {
			$msg = "删除失败";
		} else {
			$msg = "$friend 已从您的好友名单中删除";
			$error = 1;			
		}
	}
	if ($error > 0) {
?>
<table cellpadding=3 cellspacing=1 align=center class=tableborder1 style="width:75%">
<tr align=center>
<th width="100%">论坛<?php echo $error==1?"成功":"错误"; ?>信息
</td>
</tr>
<tr>
<td width="100%" class=tablebody1>
<b><?php echo $error==1?"操作成功":"操作出现错误"; ?>:</b><br><br>
<li><?php echo $msg; ?>
</td></tr>
<tr align=center><td width="100%" class=tablebody2>
<a href="javascript:window.close()">关闭窗口</a> 
</td></tr>
</table>
<?php
	}
}

function main() {
	global $currentuser;
?>
<br>
<form action="" method=post id="oForm">
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
<a href="friendlist.php?delfriend=<?php echo $friend['ID']; ?>">删除好友</a> <a href="sendmail.php?receiver=<?php echo $friend['ID']; ?>">发送邮件</a> [ToDo]
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
