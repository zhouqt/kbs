<?php
	/*	This file dispaly mailbox to user	windinsn nov 7,2003*/
	require("funcs.php");
	
	if ($loginok != 1 || $currentuser[userid] == "guest" )
		html_nologin();
	else{
		html_init("gb2312","","",1);
		if(!strcmp($currentuser["userid"],"guest"))
			html_error_quit("guest 没有自己的邮箱!");
			
		//add custom mail box
		if (isset($_POST["boxname"]))
		{
			$boxname = $_POST["boxname"];
			$ret = bbs_changemaillist(TRUE,$currentuser["userid"],$boxname,0);
			if (!$ret)html_error_quit("存储自定义邮箱数据失败!");
			if ($ret > 0)  //数目到上限
			{
?>
<SCRIPT language="javascript">
	alert("自定义邮箱数已到上限!上限是" + <?php echo "\"$ret\"";?>);
	history.go(-1);
</SCRIPT>
<?php
			}
		}	
		//delete custom mail box
		if (isset($_GET["delete"]))
		{
			$delete =$_GET["delete"];  //1-based
			unset($mailboxs[$delete -1]);
			if(!bbs_changemaillist(FALSE,$currentuser["userid"],"",$delete-1))
				html_error_quit("存储自定义邮箱数据失败!");
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
<script language='JavaScript'>
<!--
function bbs_confirm(url,infor){
	if(confirm(infor)){
		window.location.href=url;
		return true;
		}
	return false;
}
-->
</script>
<table width="100%" border="0" cellspacing="0" cellpadding="3">
  <tr> 
    <td class="b2">
	    <a href="bbssec.php" class="b2"><?php echo BBS_FULL_NAME; ?></a>
	    -
	    <?php echo $currentuser["userid"]; ?>的邮箱
    </td>
  </tr>
  <tr>
  	<td class="ts2">
  	<?php echo $currentuser["userid"].".bbs@smth.org&nbsp;&nbsp".date("Y-m-d  l"); ?>
  	</td>
  </tr>
  <tr>
  	<td class="b9" align="center" height="50" valign="middle">
  	您的邮箱中共有<font class="b3"><?php echo $totle_mails; ?></font>封邮件，占用空间<font class="b3"><?php echo $mail_used_space; ?></font>K
  	</td>
  </tr>
  <tr><td background="/images/dashed.gif" height="9"> </td></tr>
  <tr>
  	<td align="center">
  	<table width="90%" border="0" cellspacing="0" cellpadding="3" class="t1">
  	<tr>
  		<td class="t2">文件夹</td>
  		<td class="t2">信件数</td>
  		<td class="t2">新邮件</td>
  		<td class="t2">删除</td>
<?php
		for($i=0;$i<$mailboxnum;$i++){
?>
	<tr>	
		<td class="t3"><a href="bbsmailbox.php?path=<?php echo $mail_box[$i];?>&title=<?php echo urlencode($mail_boxtitle[$i]);?>" class="ts2"><?php echo $mail_boxtitle[$i]; ?></a></td>
		<td class="t4"><?php echo $mail_boxnums[$i]; ?></td>
		<td class="t3">
		<?php 
			if($i==0&&$unread!=0){
?>
<img src="/images/nmail.gif" alt="您有未读邮件"><?php echo $unread; ?>
<?php 
				}
			else{
?>
<font color="#999999">-</font>
<?php
				}
?>
		</td>
		<td class="t4">
<?php
			if($i>2){
?>
<a href="bbsmailbox.php?delete=<?php echo $i-2; ?>">删除</a>
<?php
				}
			else{
?>
-
<?php
				}
?>
		</td>
	</tr>	
<?php			
			}
?>
  	<tr>	
		<td class="t3">合计</td>
		<td class="t4" colspan="3"><?php echo $totle_mails; ?></td>
	</tr>
  	</table>
  	</td>
  </tr>
  <tr>
  	<td align=center>
  	[<a href="bbspstmail.php">发送新邮件</a>]
  	[<a href="#" onclick="bbs_confirm('bbsmailbox.php?clear=TURE','清空垃圾箱内的所有邮件吗?')">清空垃圾箱</a>]
  	<!--
  	<input type="button" onclick="window.location.href='bbspstmail.php'" value="发送新邮件" class="b9">
  	<input type="button" onclick="{if(confirm('清空垃圾箱内的所有邮件吗?')){window.location.href='bbsmailbox.php?clear=TURE';return true;}return false;}" value="清空垃圾箱" class="b9">
  	-->
  	</td>
  </tr>
 <tr>
  	<td background="/images/dashed.gif" height="9"> </td>
  </tr>
  <form action="<?php echo $_SERVER["PHP_SELF"];	?>" method="POST" onsubmit="
  	if(this.boxname.value==''){
  		alert('请输入新文件夹名称！');
  		return false;
  		}
  	">
  <tr>
  	<td align="center" class="b9">
  	新建文件夹:
  	<input name="boxname" size=24 maxlength=20 type=text value='' class="b9">
  	<input type="submit" value="添加" class="b9">
  	</td>
  </tr>	
  </form>	
</table>
<?php
		html_normal_quit();
		}
?>