<?php
	/**
	 * send mail .
	 * $Id$
	 */
	require("funcs.php");
	if ($loginok != 1) {
		html_nologin();
		exit;
	}
		html_init("gb2312","","",1);
		if (! bbs_can_send_mail() )
			html_error_quit("您不能发送信件");
		if (isset($_GET["board"]))
			$board = $_GET["board"];
		if (isset($_GET["file"]))
			$file = $_GET["file"];
		if (isset($_GET["title"]))
			$title = $_GET["title"];
		if (isset($_GET["userid"]))
			$destuserid = $_GET["userid"];

		if (isset( $board )){
			$brdarr = array();
			$brdnum = bbs_getboard($board, $brdarr);
			if ($brdnum == 0)
				html_error_quit("错误的讨论区");
			$usernum = $currentuser["index"];
			if (bbs_checkreadperm($usernum, $brdnum) == 0)
				html_error_quit("错误的讨论区");
			if (isset($file) && bbs_valid_filename($file) < 0)
				html_error_quit("错误的文章");
		}else{
			if (isset($file) && ( $file[0]!='M' || strstr($file,"..") ) )
				html_error_quit("错误的文章..");
		}
		$brd_encode = urlencode($brdarr["NAME"]);

		//system mailboxs
		$mail_box = array(".DIR",".SENT",".DELETED");
		$mail_boxtitle = array("收件箱","发件箱","垃圾箱");

		//custom mailboxs
		$mail_cusbox = bbs_loadmaillist($currentuser["userid"]);
		$i = 2;
		if ($mail_cusbox != -1){
			foreach ($mail_cusbox as $mailbox){
				$i++;
				$mail_box[$i] = $mailbox["pathname"];
				$mail_boxtitle[$i] = $mailbox["boxname"];
				//$mail_boxnums[$i] = bbs_getmailnum2(bbs_setmailfile($currentuser["userid"],$mailbox["pathname"]));
				//$totle_mails+= $mail_boxnums[$i];
				}
			}
		$mailboxnum = $i + 1;
?>
<body>
<p align="left" class="b2">
<a href="bbssec.php" class="b2"><?php echo BBS_FULL_NAME; ?></a>
-
<a href="bbsmail.php">
<?php echo $currentuser["userid"]; ?>的邮箱
</a></p>
<center>
<table border="0" width="750" cellspacing="0" cellpadding="0">
	<tr>
	<td align="center" valign="middle" background="/images/m2.gif" width="80" height="26">
	<a href="bbspstmail.php" class="mb1">写邮件</a>
	</td>
<?php
	for($i=0;$i<$mailboxnum;$i++){
?>
<td align="center" valign="middle" background="/images/m1.gif" width="80" height="26">
<a href="bbsmailbox.php?path=<?php echo $mail_box[$i];?>&title=<?php echo urlencode($mail_boxtitle[$i]);?>" class="mb1"><?php echo htmlspecialchars($mail_boxtitle[$i]); ?></a>
</td>
<?php		
	}
?>
		<td width="<?php echo (int)(670-80*$mailboxnum);	?>"><img src="/images/empty.gif"></td>
	</tr>
	<tr>
		<td style="{background-image: url('/images/m3.gif'); background-repeat:repeat-y; background-color: #CEE3F8;}"><img src="/images/empty.gif"></td>
		<td colspan="<?php echo $mailboxnum + 1;	?>" align="right" background="/images/m10.gif"><img src="/images/m12.gif"></td>
	</tr>
	<tr>
		<td height=200 colspan="<?php echo $mailboxnum+2;	?>">
		<table width="100%" cellspacing="0" cellpadding="0">
			<tr>
				<td width="7" background="/images/m3.gif"><img src="/images/empty.gif"></td>
				<td background="/images/m6.gif" height="400" align="center" valign="top">

<form name="postform" method="post" action="/cgi-bin/bbs/bbssndmail">
<table>
<tr>
<td>
发信人: <?php echo $currentuser["userid"]; ?><br />
信件标题: <input type="text" name="title" size="40" maxlength="100" value="<?php echo $title; ?>"><br />
收信人: <input type="text" name="userid" value="<?php echo $destuserid; ?>"><br />
使用签名档 <select name="signature">
<?php
		if ($currentuser["signum"] == 0)
		{
?>
<option value="0" selected="selected">不使用签名档</option>
<?php
		}
		else
		{
?>
<option value="0">不使用签名档</option>
<?php
			for ($i = 1; $i < 6; $i++)
			{
				if ($currentuser["signature"] == $i)
				{
?>
<option value="<?php echo $i; ?>" selected="selected">第 <?php echo $i; ?> 个</option>
<?php
				}
				else
				{
?>
<option value="<?php echo $i; ?>">第 <?php echo $i; ?> 个</option>
<?php
				}
			}
		}
?>
</select>
 [<a target="_balnk" href="bbssig.php">查看签名档</a>]
<input type="checkbox" name="backup">备份<br />
<textarea name="text"  onkeydown='if(event.keyCode==87 && event.ctrlKey) {document.postform.submit(); return false;}'  onkeypress='if(event.keyCode==10) return document.postform.submit()' rows="20" cols="80" wrap="physical">
<?php
    if(isset($file)){
		if(isset($board)){
    		$filename = "boards/" . $board . "/" . $file;
            echo "\n【 在 " . $destuserid . " 的大作中提到: 】\n";
		}else{
			$filename = "mail/".strtoupper($currentuser["userid"]{0})."/".$currentuser["userid"]."/".$file;
            echo "\n【 在 " . $destuserid . " 的来信中提到: 】\n";
		}
		if(file_exists($filename))
		{
		    $fp = fopen($filename, "r");
	        if ($fp) {
			    $lines = 0;
	            $buf = fgets($fp,256);       /* 取出第一行中 被引用文章的 作者信息 */
				$end = strrpos($buf,")");
				$start = strpos($buf,":");
				if($start != FALSE && $end != FALSE)
				    $quser=substr($buf,$start+2,$end-$start-1);

		        for ($i = 0; $i < 3; $i++) {
	                if (($buf = fgets($fp,500)) == FALSE)
	   	                break;
	            }
	            while (1) {
	                if (($buf = fgets($fp,500)) == FALSE)
	                    break;
	                if (strncmp($buf, ": 【", 4) == 0)
	                    continue;
	                if (strncmp($buf, ": : ", 4) == 0)
	                    continue;
	                if (strncmp($buf, "--\n", 3) == 0)
	                    break;
	                if (strncmp($buf,'\n',1) == 0)
	                    continue;
	                if (++$lines > 10) {
	                    echo ": ...................\n";
	                    break;
	                }
	                /* */
	                if (stristr($buf, "</textarea>") == FALSE)  //filter </textarea> tag in the text
	                    echo ": ". $buf;
	            }
	            fclose($fp);
	        }
	    }
	}
?>
</textarea><br><div align="center">
<input type="submit" value="发送" />
<input type="reset" value="清除" />
<input type="button" value="返回" onclick="window.location.href='bbsmail.php'" />
</div></table></form>

				</td>
				<td width="7" background="/images/m4.gif"><img src="/images/empty.gif"></td>
			</tr>
		
		</table>
		</td>
	<tr>
		
		<td colspan="<?php echo $mailboxnum+2;	?>">
		<table width="100%" cellspacing="0" cellpadding="0"><tr>
			<td width="9" height="26"><img src="/images/m7.gif"></td>
			<td background="/images/m5.gif" height="26"><img src="/images/empty.gif"></td>
			<td width="9" height="26"><img src="/images/m8.gif"></td>
		</tr></table>
		</td>
	</tr>
</table><br>
</center>
<?php
	html_normal_quit();
?>

