<?php
require("inc/funcs.php");

require("inc/usermanage.inc.php");

require("inc/user.inc.php");

setStat("用户邮件服务");

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
} else {
	showMailSampleIcon();
}

show_footer();

function main() {
	global $_GET;

	$boxName=$_GET['boxname'];
	if (!isset($_GET['start'])) {
		$startNum=99999;
	} else {
		$startNum=intval($_GET['start']);
	}

	if ($boxName=='') {
		$boxName='inbox';
	}
	if ($boxName=='inbox') {
		showUserManageMenu();
		showmailBoxes();
		showmailBox('inbox','.DIR','收件箱', $startNum);
		return true;
	}
	if ($boxName=='sendbox') {
		showUserManageMenu();
		showmailBoxes();
		showmailBox('sendbox','.SENT','发件箱',$startNum );
		return true;
	}
	if ($boxName=='deleted') {
		showUserManageMenu();
		showmailBoxes();
		showmailBox('deleted','.DELETED','垃圾箱',$startNum);
		return true;
	}
	foundErr("您指定了错误的邮箱名称！");
	return false;
}



function showmailBox($boxName, $path, $desc, $startNum){
	global $currentuser;
?>
<br>
<form action="usermailoperations.php" method=post id="oForm">
<input type="hidden" name="boxname" value="<?php echo $boxName; ?>">
<table cellpadding=3 cellspacing=1 align=center class=tableborder1>
<tr>
<th valign=middle width=30 height=25>已读</th>
<th valign=middle width=100>
<?php   if ($Desc=="发件箱")
    echo "收件人";  
  else
    echo "发件人";?>
</th>
<th valign=middle width=380>主题</th>
<th valign=middle width=120>日期</th>
<th valign=middle width=50>大小</th>
<th valign=middle width=30>删除</th>
</tr>
<?php
	$mail_fullpath = bbs_setmailfile($currentuser["userid"],$path);
	$mail_num = bbs_getmailnum2($mail_fullpath);
	if($mail_num < 0 || $mail_num > 30000) {
		foundErr('您的'.$desc.'中信件太多！');
		return false;
	}
	if($mail_num == 0) {
?>
<tr>
<td class=tablebody1 align=center valign=middle colspan=6>您的<?php echo $desc; ?>中没有信件。</td>
</tr>
</table>
<?php
	return false;
	}
	$num=ARTICLESPERPAGE;
	if ($startNum > $mail_num - $num ) $startNum = $mail_num - $num ;
	if ($startNum < 0)
	{
		$startNum = 0;
		$num = $mail_num;
	}
	$maildata = bbs_getmails($mail_fullpath,$startNum,$num);
	if ($maildata == FALSE) {
			foundErr("读取邮件数据失败!");
			return false;
	}
	for ($i = 0; $i < $num; $i++){
?>
<tr>
<td class=tablebody1 align=center valign=middle>
<?php 
		
		if ($maildata[$i]["FLAGS"][1]=='R') {
			
			switch($maildata[$i]["FLAGS"][0]){
			case 'M':
			case 'm':
				echo  '<img src="pic/m_lockreplys.gif">';
					break;
			default:
				echo  '<img src="pic/m_replys.gif">';
			}
		} else {
			switch($maildata[$i]["FLAGS"][0]){
			case 'N':
				echo  '<img src="pic/m_news.gif">';
				break;
			case 'M':
				echo  '<img src="pic/m_oldlocks.gif">';
					break;
			case 'm':
				echo  '<img src="pic/m_newlocks.gif">';
					break;
			default:
				echo  '<img src="pic/m_olds.gif">';
			}
		}
?>
</td>
<td class=tablebody1 align=center valign=middle style="font-weight:normal">
<a href="userinfo.php?id=<?php echo $maildata[$i]['OWNER'] ; ?>" target=_blank><?php echo $maildata[$i]['OWNER'] ; ?></a>
</td>
<td class=tablebody1 align=left style="font-weight:normal"><a href="usermail.php?boxname=<?php echo $boxName; ?>&num=<?php echo $i+$startNum; ?>" > <?php       echo htmlspecialchars($maildata[$i]['TITLE'],ENT_QUOTES); ?></a>	</td>
<td class=tablebody1 style="font-weight:normal"><?php echo strftime("%Y-%m-%d %H:%M:%S", $maildata[$i]['POSTTIME']); ?></td>
<td class=tablebody1 style="font-weight:normal"> N/A Byte</td>
<td align=center valign=middle width=30 class=tablebody1><input type=checkbox name=num id="oNum" value=<?php echo $i+$startNum; ?>></td>
</tr>
<?php
	}
?>
<tr> 
<td align=right valign=middle colspan=6 class=tablebody2>您现在已使用了<?php echo bbs_getmailusedspace() ;?>K邮箱空间，共有<?php echo $mail_num; ?>封信&nbsp;
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
