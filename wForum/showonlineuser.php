<?php
require("inc/funcs.php");

require("inc/usermanage.inc.php");

require("inc/user.inc.php");

setStat("在线用户列表");

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

head_var("谈天说地","usermanagemenu.php",0);

//if ($loginok==1) {
	main();
//}else {
//	foundErr("本页需要您以正式用户身份登陆之后才能访问！");
//}


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
<th valign=middle width=280>用户昵称</th>
<th valign=middle width=120>用户上线地址</th>
<th valign=middle width=50>发呆</th>
<th valign=middle width=130>操作</th>
</tr>
<?php
    if( isset( $_GET["start"] ) ){
        $startNum = $_GET["start"];
    } else {
        $startNum = 1;
    }
    if ($startNum <= 0) $startNum = 1;
    define("USERSPERPAGE", 20); //ToDo: put this define into site.php  - atppp
	$online_user_list = bbs_getonline_user_list($startNum, USERSPERPAGE);
    $total_online_num = bbs_getonlineusernumber();
    
	$count = count ( $online_user_list );

	for ( $i=0; $i<$count ; $i++ ) {
?>
<tr>
<td class=TableBody1 align=center valign=middle>
<?php echo $startNum+$i; ?>
</td>
<td class=TableBody1 align=center valign=middle style="font-weight:normal">
<a href="dispuser.php?id=<?php echo $online_user_list[$i]['userid'] ; ?>" target=_blank>
<?php echo $online_user_list[$i]['userid'] ?></a>
</td>
<td class=TableBody1 align=left style="font-weight:normal"><a href="usermail.php?boxname=<?php echo $boxName; ?>&num=<?php echo $i+$startNum; ?>" > <?php       echo htmlspecialchars($online_user_list[$i]['username'],ENT_QUOTES); ?></a>	</td>
<td class=TableBody1 style="font-weight:normal"><?php echo $online_user_list[$i]['userfrom']; ?></td>
<td class=TableBody1 style="font-weight:normal"><?php printf('%02d:%02d',intval($online_user_list[$i]['idle']/60), ($online_user_list[$i]['idle']%60)); ?></td>
<td align=center valign=middle width=130 class=TableBody1>
<a href="#">添加好友</a> <a href="#">删除好友</a> <a href="#">发送消息</a> <a href="#">发送短信</a>
</td>
</tr>
<?php
	}
?>
<tr>
<td align=right valign=middle colspan=6 class=TableBody2>
<?php
			
		if ($startNum > 1)
		{
			$i = $startNum - USERSPERPAGE;
			if ($i < 1) $i = 1;
			echo ' [<a href=showonlineuser.php>第一页</a>] ';
			echo ' [<a href=showonlineuser.php?start='.$i.'>上一页</a>] ';
		} else {
?>
<font color=gray>[第一页]</font>
<font color=gray>[上一页]</font>
<?php 
		}
		if ($startNum < $total_online_num - USERSPERPAGE) //这一段是不准确的，因为没有考虑隐身用户，我先不管了。- atppp
		{
			$i = $startNum + USERSPERPAGE;
			if ($i > $total_online_num -1) $i = $total_online_num -1;
			echo ' [<a href=showonlineuser.php?start='.$i.'>下一页</a>] ';
			echo ' [<a href=showonlineuser.php?start='.($total_online_num - USERSPERPAGE).'>最后一页</a>] ';
		} else {
?>
<font color=gray>[下一页]</font>
<font color=gray>[最后一页]</font>
<?php
		}
?>
<br>
目前论坛上总共有 <b><?php echo bbs_getonlinenumber() ; ?></b> 人在线，其中注册用户 <b><?php echo bbs_getonlineusernumber(); ?></b> 人，访客 <b><?php echo bbs_getwwwguestnumber() ; ?></b> 人。
</td>
</tr>
</table>
</form>
<?php
}


?>
