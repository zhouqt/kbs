<?php
	/**
	 * This file display user custom mailboxs .
	 */
	require("funcs.php");
	if ($loginok != 1)
		html_nologin();
	else
	{
		html_init("gb2312");

		if(!strcmp($currentuser["userid"],"guest"))
			html_error_quit("guest 没有自己的邮箱!");
		
		$mailboxs = bbs_loadmaillist($currentuser["userid"]);
		if ($mailboxs == FALSE)html_error_quit("读取自定义邮箱数据失败!");

		if (isset($_GET["delete"]))//delete
		{
			$delete =$_GET["delete"];  //1-based
			unset($mailboxs[$delete -1]);
			if(!bbs_changemaillist(FALSE,$currentuser["userid"],"",$delete-1))
				html_error_quit("存储自定义邮箱数据失败!");
			$mailboxs = bbs_loadmaillist($currentuser["userid"]);
		}
		else
			$delete = 0;

		if (isset($_GET["boxname"]))//add
		{
			$boxname = $_GET["boxname"];
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
			$mailboxs = bbs_loadmaillist($currentuser["userid"]);
		}

?>
<body>
<center><?php echo BBS_FULL_NAME;?> -- 信件列表 - 自定义邮箱 [使用者: <?php echo $currentuser["userid"]; ?>]<hr color=green>

<?php
		if($mailboxs == -1)
		{
			echo "无自定义邮箱";
		}
		else
		{
?>
<table width="250">
<tr><td>序号</td><td>邮箱名称</td><td>邮件封数</td></tr>
<?php
			$i = 0;
			foreach ($mailboxs as $mailbox)
			{
				$i++;
?>
<tr><td> <?php echo $i; ?></td>
<td><a href="bbsreadmail.php?path=<?php echo $mailbox["pathname"];?>&title=<?php echo $mailbox["boxname"];?>"><?php echo $mailbox["boxname"]; ?></a></td>
<td><?php
echo bbs_getmailnum2(bbs_setmailfile($currentuser["userid"],$mailbox["pathname"]));
?></td>
<td><a href=bbsmailbox_custom.php?delete=<?php echo $i;?>>删除</a>
</td></tr>
<?php
			}
		}
?>
</table>
</body>
<form>增添自定义邮箱:<input name=boxname size=24 maxlength=20 type=text value=''><input type=submit value=确定></form>
<hr class="default"/>
<?php
	}

?>
