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
	$online_user_list = bbs_getonline_user_list();

	$num = count ( $online_user_list );

	for ( $i=0; $i<$num ; $i++ ) {
?>
<tr>
<td class=TableBody1 align=center valign=middle>
<?php echo $i+1 ?>
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
<td align=right valign=middle colspan=6 class=TableBody2>您现在已使用了<?php echo bbs_getmailusedspace() ;?>K邮箱空间，共有<?php echo $mail_num; ?>封信&nbsp;
<?php
			
		if ($startNum > 0)
		{
			$i = $startNum - ARTICLESPERPAGE;
			if ($i < 0) $i = 0;
			echo ' [<a href=usermailbox.php?boxname='.$boxName.'&start=0>第一页</a>] ';
			echo ' [<a href=usermailbox.php?boxname='.$boxName.'&start='.$i.'>上一页</a>] ';
		} else {
?>
<font color=gray>[第一页]</font>
<font color=gray>[上一页]</font>
<?php 
		}
		if ($startNum < $mail_num - ARTICLESPERPAGE)
		{
			$i = $startNum + ARTICLESPERPAGE;
			if ($i > $mail_num -1) $i = $mail_num -1;
			echo ' [<a href=usermailbox.php?boxname='.$boxName.'&start='.$i.'>下一页</a>] ';
			echo ' [<a href=usermailbox.php?boxname='.$boxName.'>最后一页</a>] ';
		} else {
?>
<font color=gray>[下一页]</font>
<font color=gray>[最后一页]</font>
<?php
		}
?>
<br>
<input type="hidden" name="action" id="oAction">
<input type="hidden" name="nums" id="oNums">
<input type="hidden" id="oNum">
<script >
function doAction(desc,action) {
	var nums,s,first;
	if(confirm(desc))	{
		oForm.oNums.value="";
		oForm.oAction.value=action;
		first=true;
		for (nums=new Enumerator(document.all.item("oNum"));!nums.atEnd();nums.moveNext()){
			s=nums.item();
			if (s.checked) {
				if (first) {
					first=false;
				} else {
					oForm.oNums.value+=',';
				}
				oForm.oNums.value+=s.value;
			}
		}
		return oForm.submit()
	}
	return false;
}
</script>
<input type=checkbox name=chkall value=on onclick="CheckAll(this.form)">选中所有显示信件&nbsp;<input type=button onclick="doAction('确定锁定/解除锁定选定的纪录吗?','lock');" value="锁定信件">&nbsp;<input type=button onclick="doAction('确定删除选定的纪录吗?','delete');" value="删除信件">&nbsp;<input type=button onclick="doAction('确定清除<?php echo $desc; ?>所有的纪录吗?','deleteAll');" value="清空<?php   echo $desc; ?>"></td>
</tr>
</table>
</form>
<?php
}


?>
