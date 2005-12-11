<?php
	require("www2-funcs.php");
	login_init();
	page_header($currentuser["userid"] . " 的信箱");
	assert_login();

	$mail_fullpath = bbs_setmailfile($currentuser["userid"],".DIR");
	$mail_num = bbs_getmailnum2($mail_fullpath);

	if($mail_num < 0 || $mail_num > 30000)html_error_quit("Too many mails!");

	$maildata = bbs_getmails($mail_fullpath,0,$mail_num);
	if ($maildata == FALSE)
		html_error_quit("读取邮件数据失败!");
	
	mailbox_header("新信件列表");
?>
<table class="main wide adj">
<caption>新信件列表</caption>
<col class="center"/><col class="center"/><col class="center"/><col class="center"/><col/>
<tbody><tr><th>序号</th><th>状态</th><th>发信人</th><th>日期</th><th>信件标题</th>
<?php
	$newtotal = 0; $start = 0;
	for($i = 0;$i < $mail_num;$i++)
	{
		if($maildata[$i]["FLAGS"][0] == 'N')
		{
			$newtotal++;
?>
<tr><td><?php echo $start+$i+1;?><td><?php echo $maildata[$i]["FLAGS"];?></td>
<td><a href=bbsqry.php?userid=<?php echo $maildata[$i]["OWNER"];?>><?php echo $maildata[$i]["OWNER"];?></a></td>
<td><?php echo strftime("%b&nbsp;%e&nbsp;%H&nbsp;:%M",$maildata[$i]["POSTTIME"]);?></td>
<td><a href=bbsmailcon.php?dir=.DIR&num=<?php echo $i;?>&title=收件箱><?php
if(strncmp($maildata[$i]["TITLE"],"Re: ",4))
	echo "★" .  htmlspecialchars($maildata[$i]["TITLE"]);
else
	echo htmlspecialchars($maildata[$i]["TITLE"]);
?> </a></td>
</tr>
<?php
		}
	}
?>
</tbody></table>
<div class="oper">您的信箱里共有 <?php echo $mail_num; ?> 封信件,其中新信 <?php echo $newtotal; ?> 封</div>
<?php
	page_footer();
?>