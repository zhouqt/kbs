<?php
	/**
	 * This file read mails from the mail path file and start index .
	 * by -binxun
	 */
	require("funcs.php");
	if ($loginok != 1)
		html_nologin();
	else
	{
		html_init("gb2312");
		if (isset($_GET["path"])){
			$mail_path = $_GET["path"];
			$mail_title = $_GET["title"];
		}
		else {
			$mail_path = ".DIR";    //default is .DIR
			$mail_title = "收件箱";
		}
		if (isset($_GET["start"]))
			$start = $_GET["start"];
		else
			$start = 999999;   //default*/
		$mail_fullpath = bbs_setmailfile($currentuser["userid"],$mail_path);
		$mail_num = bbs_getmailnum2($mail_fullpath);
		if($mail_num < 0 || $mail_num > 30000)http_error_quit("Too many mails!");

		$num = 19;
		if ($start > $mail_num - 19)$start = $mail_num - 19;
        if ($start < 0)
		{
		    $start = 0;
			$num = $mail_num;
		}
		$maildata = bbs_getmails($mail_fullpath,$start,$num);
		if ($maildata == FALSE)
			html_error_quit("读取邮件数据失败!");

?>
<center>信件列表 - 邮箱 - <?php echo $mail_title;?>[使用者: <?php echo $currentuser["userid"] ?>]<hr color=green>
<?php
	if($mail_num == 0)
	{
		echo "该信箱没有任何信件!";
	}
	else
	{
?>
<table width="610">
<tr><td>序号<td>状态<td>发信人<td>日期<td>信件标题</tr></td>
<?php
		for ($i = 0; $i < $num; $i++)
		{
			/*$ptr = strtok($maildata[$i]["OWNER"]," (");
			if($ptr == 0)$ptr = " ";
			$ptr = nohtml($*/

?>
<tr><td><?php echo $start+$i+1;?><td><?php echo $maildata[$i]["FLAGS"];?><td>
<a href=/cgi-bin/bbs/bbsqry?userid=<?php echo $maildata[$i]["OWNER"];?>><?php echo $maildata[$i]["OWNER"];?></a>
<td><?php echo strftime("%b&nbsp;%e&nbsp;%H&nbsp;:%M",$maildata[$i]["POSTTIME"]);?>
<td><a href=/cgi-bin/bbs/bbsmailcon?file=<?php echo $maildata[$i]["FILENAME"];?>&dir=<?php echo $mail_path;?>&num=<?php echo $i;?>&title=<?php echo $mail_title;?>><?php
if(strncmp($maildata[$i]["TITLE"],"Re: ",4))
	echo "★" .  htmlspecialchars($maildata[$i]["TITLE"]);
else
    echo htmlspecialchars($maildata[$i]["TITLE"]);
?></a>
</tr>
<?php
		}
	}
?>
</table>
<hr class="default"/>
[信件总数: <?php echo $mail_num;?>]
[已用空间: <?php echo bbs_getmailusedspace();?>K]
[<a href=/cgi-bin/bbs/bbspstmail>发送信件</a>]
<?php
		if ($start > 0)
		{
			$i = $start - 19;
			if ($i < 0)$i = 0;
			echo "[<a href=bbsreadmail.php?path=$mail_path&start=0&title=$mail_title>第一页</a>] ";
			echo "[<a href=bbsreadmail.php?path=$mail_path&start=$i&title=$mail_title>上一页</a>] ";
		}
		if ($start < $mail_num - 19)
		{
			$i = $start + 19;
			if ($i > $mail_num -1)$i = $mail_num -1;
			echo "<a href=bbsreadmail.php?path=$mail_path&start=$i&title=$mail_title>下一页</a> ";
			echo "<a href=bbsreadmail.php?path=$mail_path&title=$mail_title>最后一页</a> ";
		}
?>
<form><input type=submit value=跳转到><input type=hidden name=path value=<?php echo $mail_path;?>>
<input type=hidden name=title value=<?php echo $mail_title;?>>
 第 <input sytle='height:20px' type=text name=start size=3> 封</form>
<?php
	}
?>
