<?php
	/**
	 * This file display system defined mailboxs .
	 */
	require("funcs.php");
	if ($loginok != 1)
		html_nologin();
	else
	{
		html_init("gb2312");
		
		$mail_sysbox = array(".DIR",".SENT",".DELETED");
		$mail_sysboxtitle = array("收件箱","发件箱","垃圾箱");
?>
<body>
<center><?php echo $BBS_FULL_NAME;?> -- 信件列表 - 系统预定义邮箱 [使用者: <?php echo $currentuser["userid"]; ?>]<hr color=green>
<table width="200">
<tr><td>序号</td><td>邮箱名称</td><td>邮件封数</td></tr>
<?php
		$i = 0;
		foreach ($mail_sysbox as $sysbox_path)
		{
			$i++;
?>
<tr><td> <?php echo $i; ?></td>
<td><a href="bbsreadmail.php?path=<?php echo $sysbox_path;?>&title=<?php echo $mail_sysboxtitle[$i-1];?>"><?php echo $mail_sysboxtitle[$i-1]; ?></a></td>
<td><?php
echo bbs_getmailnum2(bbs_setmailfile($currentuser["userid"],$sysbox_path));
?></td></tr>
<?php
		}
?>
</table>
</body>
<hr class="default"/>
<?php
	}
?>
