<?php
	/**
	 * This file read new mails
	 * @binxun
	 */
	require("funcs.php");
	if ($loginok != 1)
		html_nologin();
	else
	{
		html_init("gb2312");

		$mail_fullpath = bbs_setmailfile($currentuser["userid"],".DIR");
		$mail_num = bbs_getmailnum2($mail_fullpath);

		if($mail_num < 0 || $mail_num > 30000)http_error_quit("Too many mails!");

		$maildata = bbs_getmails($mail_fullpath,0,$mail_num);
		if ($maildata == FALSE)
			html_error_quit("读取邮件数据失败!");
?>
<center><?php echo BBS_FULL_NAME; ?> 新信件列表 - [使用者: <?php echo $currentuser["userid"] ?>]<hr color=green>
<table width= 610 border=1>
<tr><td>序号<td>状态<td>发信人<td>日期<td>信件标题
<?php
    $newtotal = 0;
	for($i = 0;$i < $mail_num;$i++)
	{
        if($maildata[$i]["FLAGS"][0] == 'N')
		{
	        $newtotal++;
?>
<tr><td><?php echo $start+$i+1;?><td><?php echo $maildata[$i]["FLAGS"];?><td>
<a href=/cgi-bin/bbs/bbsqry?userid=<?php echo $maildata[$i]["OWNER"];?>><?php echo $maildata[$i]["OWNER"];?></a>
<td><?php echo strftime("%b&nbsp;%e&nbsp;%H&nbsp;:%M",$maildata[$i]["POSTTIME"]);?>
<td><a href=/bbsmailcon.php?dir=.DIR&num=<?php echo $i;?>&title=收件箱><?php
if(strncmp($maildata[$i]["TITLE"],"Re: ",4))
	echo "★" .  htmlspecialchars($maildata[$i]["TITLE"]);
else
    echo htmlspecialchars($maildata[$i]["TITLE"]);
?></a>
</tr>
<?php
		}
	}
}
?>
</table><hr>
您的信箱里共有 <?php echo $mail_num; ?> 封信件,其中新信 <?php echo $newtotal; ?> 封.
