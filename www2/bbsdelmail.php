<?php
	require("www2-funcs.php");
	login_init();
	assert_login();

	$dirname = $_POST["dir"];
	$title = urldecode($_POST["title"]);
	$dstart = $_POST["dstart"];
	$dend = $_POST["dend"];
	$dtype = $_POST["dtype"];
	
	if (strstr($dirname,'..'))
	{
		html_error_quit("读取邮件数据失败!");
	}
	
	$mail_fullpath = bbs_setmailfile($currentuser["userid"],$dirname);
	$mail_num = bbs_getmailnum2($mail_fullpath);
		
	if( $dstart < 1 || $dstart > $mail_num  || $dend < 1 || $dend > $mail_num  || $dstart > $dend  )
	{
		html_error_quit("区段删信起始、结束序号有误，请重新输入！");
	}
	mailbox_header("区段删信");
?>
<form action="bbsmailact.php?act=move&<?php echo "dir=".urlencode($dirname)."&title=".urlencode($title); ?>" method="POST" class="small">
<fieldset><legend>区段删信</legend>
信箱名称：<b class="red"><?php echo $title; ?></b><br/>
起始邮件序号：<b class="red"><?php echo $dstart; ?></b><br/>
结束邮件序号：<b class="red"><?php echo $dend; ?></b><br/>
删除类型：<b class="red">
<?php 
	if($dtype == 1)
		echo "强制删除";
	else
		echo "普通删除";
?>
</b><br/>
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
				echo "<input type=\"hidden\" name=\"file".$j."\" value=\"".$mail["FILENAME"]."\"/>\n";
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
					echo "<input type=\"hidden\" name=\"file".$j."\" value=\"".$mail["FILENAME"]."\"/>\n";
					$j ++;
				}
			}
		}
?>
<input type="hidden" name="mailnum" value="<?php echo $j; ?>"/>
<div class="oper">
<input type="button" value="修改" onclick="history.go(-1)"/>
<input type="submit" value="确认"/>
</div>
</fieldset></form>
<?php	
	page_footer(FALSE);	
?>
