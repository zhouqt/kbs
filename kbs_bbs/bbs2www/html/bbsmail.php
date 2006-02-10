<?php
	require("www2-funcs.php");
	login_init();
	bbs_session_modify_user_mode(BBS_MODE_MAIL);
	assert_login();
	
	mailbox_header("信箱一览");
	
	//add custom mail box
	if (isset($_POST["boxname"]))
	{
		$boxname = $_POST["boxname"];
		$ret = bbs_changemaillist(TRUE,$currentuser["userid"],$boxname,0);
		if (!$ret)html_error_quit("存储自定义邮箱数据失败!");
		if ($ret > 0)  //数目到上限
		{
			html_error_quit("自定义邮箱数已到上限!上限是" . $ret);
		}
	}
		//delete custom mail box
	if (isset($_GET["delete"]))
	{
		$delete =$_GET["delete"];  //1-based
		if(!bbs_changemaillist(FALSE,$currentuser["userid"],"",$delete-1))
			html_error_quit("存储自定义邮箱数据失败! 有可能是因为该信箱非空。");
	}	
		
	//system mailboxs
	$mail_box = array(".DIR",".SENT",".DELETED");
	$mail_boxtitle = array("收件箱","发件箱","垃圾箱");
	$mail_boxnums = array(bbs_getmailnum2(bbs_setmailfile($currentuser["userid"],".DIR")),bbs_getmailnum2(bbs_setmailfile($currentuser["userid"],".SENT")),bbs_getmailnum2(bbs_setmailfile($currentuser["userid"],".DELETED")));
	//custom mailboxs
	$mail_cusbox = bbs_loadmaillist($currentuser["userid"]);
	$totle_mails = $mail_boxnums[0]+$mail_boxnums[1]+$mail_boxnums[2];
	$i = 2;
	if ($mail_cusbox != -1){
		foreach ($mail_cusbox as $mailbox){
			$i++;
			$mail_box[$i] = $mailbox["pathname"];
			$mail_boxtitle[$i] = $mailbox["boxname"];
			$mail_boxnums[$i] = bbs_getmailnum2(bbs_setmailfile($currentuser["userid"],$mailbox["pathname"]));
			$totle_mails+= $mail_boxnums[$i];
			}
		}
	$mailboxnum = $i + 1;
	$mail_used_space = bbs_getmailusedspace();
	
	bbs_getmailnum($currentuser["userid"],$total,$unread, 0, 0);
?>
<table class="main wide adj">
<col class="center"/><col class="center"/><col class="center"/><col class="center"/>
<caption>您的邮箱中共有 <?php echo $totle_mails; ?> 封邮件，占用空间 <?php echo $mail_used_space; ?> K。</caption>
<tbody>
<tr><th>文件夹</th><th>信件数</th><th>新邮件</th><th>删除</th></tr>
<?php
	for($i=0;$i<$mailboxnum;$i++){
?>
<tr>
<td><a href="bbsmailbox.php?path=<?php echo $mail_box[$i];?>&title=<?php echo urlencode($mail_boxtitle[$i]);?>" class="ts2"><?php echo $mail_boxtitle[$i]; ?></a></td>
<td><?php echo $mail_boxnums[$i]; ?></td>
<td>
<?php 
		if($i==0&&$unread!=0){
?>
<img src="images/nmail.gif" alt="您有未读邮件"><?php echo $unread; ?>
<?php 
		}else{
			echo "-";
		}
?>
</td><td>
<?php
		if($i>2){
			if ($mail_boxnums[$i] == 0) {
?>
<a onclick="return confirm('你真的要删除整个目录吗?')" href="bbsmail.php?delete=<?php echo $i-2; ?>">删除</a>
<?php
			} else {
?>
[信箱非空]
<?php
			}
		} else{
			echo "-";
		}
?>
</td></tr>	
<?php			
	}
?>
<tr><td>合计</td><td colspan="3"><?php echo $totle_mails; ?></td></tr>
</table>
<div class="oper">
	[<a href="bbspstmail.php">发送新邮件</a>]
	[<a href="bbsmailact.php?act=clear" onclick="return confirm('清空垃圾箱内的所有邮件吗?')">清空垃圾箱</a>]
</div>
<form action="<?php echo $_SERVER["PHP_SELF"];	?>" method="POST" onsubmit="return chkreq(this);">
<label>新建文件夹:</label>
<input name="boxname" size="24" maxlength="20" type="text" class="req"/>
<input type="submit" value="添加"/>
</form><br />	
<?php
	page_footer();
?>
