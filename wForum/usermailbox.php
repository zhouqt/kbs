<?php
require("inc/funcs.php");

require("inc/usermanage.inc.php");

setStat("用户邮件服务");

show_nav();

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
		showmailBox('.DIR','收件箱', $startNum);
		return true;
	}
	if ($boxName=='sendbox') {
		showUserManageMenu();
		showmailBoxes();
		showmailBox('.SENT','发件箱',$startNum );
		return true;
	}
	if ($boxName=='deleted') {
		showUserManageMenu();
		showmailBoxes();
		showmailBox('.DELETED','垃圾箱',$startNum);
		return true;
	}
	foundErr("您指定了错误的邮箱名称！");
	return false;
}

function showmailBoxes() {
?>
<TABLE cellpadding=6 cellspacing=1 align=center class=tableborder1><TBODY><TR>
<TD align=center class=tablebody1><a href="usermailbox.php?boxname=inbox"><img src=pic/m_inbox.gif border=0 alt=收件箱></a> &nbsp; <a href="usermailbox.php?boxname=sendbox"><img src=pic/m_outbox.gif border=0 alt=发件箱></a> &nbsp; <a href="usermailbox.php?boxname=deleted"><img src=pic/m_recycle.gif border=0 alt=废件箱></a>&nbsp; <a href="friendlist.php"><img src=pic/m_address.gif border=0 alt=地址簿></a>&nbsp;<a href=JavaScript:openScript('messanger.php?action=new',500,400)><img src=pic/m_write.gif border=0 alt=发送消息></a></td></tr></TBODY></TABLE>
<?php
}

function showmailBox($path, $desc, $startNum){
	global $currentuser;
?>
<br>
<table cellpadding=3 cellspacing=1 align=center class=tableborder1>
<tr>
<th valign=middle width=30 height=25>已读</th>
<th valign=middle width=100>
<?php   if ($Desc=="发件箱")
    echo "发件人";  
  else
    echo "收件人";?>
</th>
<th valign=middle width=300>主题</th>
<th valign=middle width=150>日期</th>
<th valign=middle width=50>大小</th>
<th valign=middle width=30>操作</th>
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
	if ($startNum > $mail_num - $num + 1) $startNum = $mail_num - $num + 1;
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
         if ($$maildata[$i]["FLAGS"]=='Y')
		print "<img src=\"pic/m_news.gif\">";
          else
         	 print "<img src=\"".$Forum_info[7]."m_olds.gif\">";
          break;
        case "outbox":
          print "<img src=\"".$Forum_info[7]."m_issend_2.gif\">";
          break;
        case "issend":
          print "<img src=\"".$Forum_info[7]."m_issend_1.gif\">";
          break;
        case "recycle":
          if ($rs["flag"]==0)
		print "<img src=\"".$Forum_info[7]."m_news.gif\">";
            else
          	print "<img src=\"".$Forum_info[7]."m_olds.gif\">";
          break;
      } 
?>
</td>
<td class=<?php       echo $tablebody; ?> align=center valign=middle style="<?php       echo $newstyle; ?>">
<?php       if ($smstype=="inbox" || $smstype=="recycle")
      {
?>
<a href="dispuser.php?name=<?php         echo HTMLEncode($rs["sender"]); ?>" target=_blank><?php         echo HTMLEncode($rs["sender"]); ?></a>
<?php       }
        else
      {
?>
<a href="dispuser.php?name=<?php         echo HTMLEncode($rs["incept"]); ?>" target=_blank><?php         echo HTMLEncode($rs["incept"]); ?></a>
<?php       } ?>
</td>
<td class=<?php       echo $tablebody; ?> align=left style="<?php       echo $newstyle; ?>"><a href="JavaScript:openScript('messanger.php?action=<?php       echo $readaction; ?>&id=<?php       echo $rs["id"]; ?>&sender=<?php       echo $rs["sender"]; ?>',500,400)"><?php       echo HTMLEncode($rs["title"]); ?></a>	</td>
<td class=<?php       echo $tablebody; ?> style="<?php       echo $newstyle; ?>"><?php       echo $rs["sendtime"]; ?></td>
<td class=<?php       echo $tablebody; ?> style="<?php       echo $newstyle; ?>"><?php       echo mb_strlen($rs["content"],"EUC-JP"); ?>Byte</td>
<td align=center valign=middle width=30 class=<?php       echo $tablebody; ?>><input type=checkbox name=id value=<?php       echo $rs["id"]; ?>></td>
</tr>
<?php
	}
?>
</table>
<?php
}


?>
