<?php
	/* this file delete user mails  windinsn nov 15,2003*/
	require("funcs.php");
	
	if ($loginok != 1)
		html_nologin();
	elseif(!strcmp($currentuser["userid"],"guest"))
	{
		html_init("gb2312");
		html_error_quit("guest 没有自己的邮箱!");
		die();
	}
	else
	{
		html_init("gb2312","","",1);
		$dirname = $_POST["dir"];
		$title = $_POST["title"];
		$dstart = $_POST["dstart"];
		$dend = $_POST["dend"];
		$dtype = $_POST["dtype"];
		
		if (strstr($dirname,'..'))
		{
			html_error_quit("读取邮件数据失败!");
                        die();
                }
		
		$mail_fullpath = bbs_setmailfile($currentuser["userid"],$dirname);
		$mail_num = bbs_getmailnum2($mail_fullpath);
			
		if( $dstart < 1 || $dstart > $mail_num  || $dend < 1 || $dend > $mail_num  || $dstart > $dend  )
		{
			html_error_quit("区段删信起始、结束序号有误，请重新输入！");
			die();
		}
		
?>
<p align="left" class="b2">
<a href="bbssec.php" class="b2"><?php echo BBS_FULL_NAME; ?></a>
-
<a href="bbsmail.php">
<?php echo $currentuser["userid"]; ?>的邮箱
</a>
-
区段删除邮件
</p>
<p align="left">
您现在在 <font class="b3"><?php echo $title; ?></font> 中进行区段删除邮件，<br>
起始邮件序号：<font class="b3"><?php echo $dstart; ?></font><br>
结束邮件序号：<font class="b3"><?php echo $dend; ?></font><br>
删除类型：<font class="b3">
<?php 
	if($dtype == 1)
		echo "强制删除";
	else
		echo "普通删除";
?>
</font><br>
</p>
<form action="/bbsmailact.php?act=move&<?php echo "dir=".urlencode($dirname)."&title=".urlencode($title); ?>" method="POST">
<input type="hidden" name="act2" value="delarea">
<?php			
		$dnum = $dend - $dstart + 1;
		$dstart-- ;
		$j = 0;
		
		$maildata = bbs_getmails($mail_fullpath,$dstart,$dnum);
		if($dtype == 1)
		{
			foreach( $maildata as $mail )
			{
				echo "<input type=\"hidden\" name=\"file".$j."\" value=\"".$mail["FILENAME"]."\">\n";
				$j ++;
			}
		}
		else
		{
			foreach( $maildata as $mail )
			{
				if(stristr($mail["FLAGS"],"m"))
					continue;
				else
				{
					echo "<input type=\"hidden\" name=\"file".$j."\" value=\"".$mail["FILENAME"]."\">\n";
					$j ++;
				}
			}
		}
?>
<input type="hidden" name="mailnum" value="<?php echo $j; ?>">
<input type="button" value="修改" class="bt1" onclick="history.go(-1)">
<input type="submit" value="确认" class="bt1">
</form>
<?php	
	}
	
?>