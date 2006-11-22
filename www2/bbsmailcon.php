<?php
	require("www2-funcs.php");
	login_init();
	bbs_session_modify_user_mode(BBS_MODE_RMAIL);
	assert_login();
	
	if (isset($_GET["num"]))
		$num = $_GET["num"];
	else {
		html_error_quit("错误的参数");
	}

	if (isset($_GET["dir"]))
		$dirname = $_GET["dir"];
	else
		$dirname = ".DIR";

	if (isset($_GET["title"]) )
		$title=$_GET["title"];
	else
		$title="收件箱";
		
	$title_encode = rawurlencode($title);

	if (strstr($dirname, "..") || strstr($dirname, "/")){
		html_error_quit("错误的参数");
	}

	$dir = bbs_setmailfile($currentuser["userid"], $dirname);
	
	$total = bbs_getmailnum2($dir);
	if($total <= 0 || $total > 30000) html_error_quit("读取邮件数据失败!");
	if ($num < 0 || $num >= $total) html_error_quit("错误的参数");

	$articles = array ();
	if( bbs_get_records_from_num($dir, $num, $articles) ) {
		$file = $articles[0]["FILENAME"];
	}else{
		html_error_quit("错误的参数");
	}

	$filename = bbs_setmailfile($currentuser["userid"], $file);
	if(! file_exists($filename)){
		html_error_quit("信件不存在...");
	}

	@$attachpos=$_GET["ap"];//pointer to the size after ATTACHMENT PAD
	if ($attachpos!=0) {
		bbs_file_output_attachment($filename, $attachpos);
		exit;
	}
	mailbox_header("信件阅读");
?>
<div class="large">
<div class="article">
<script type="text/javascript"><!--
<?php $s = bbs2_readfile($filename); if (is_string($s)) echo 'att=new attWriter(-1);'.$s; ?>
//-->
</script>
</div></div>
<div class="oper">
[<a onclick='return confirm("你真的要删除这封信吗？")' href="bbsmailact.php?act=del&dir=<?php echo $dirname;?>&file=<?php echo $file;?>&title=<?php echo $title_encode;?>">删除</a>]
[<a href="javascript:history.go(-1)">返回上一页</a>]
<?php
				if($num > 0){
?>
[<a href="bbsmailcon.php?dir=<?php echo $dirname;?>&num=<?php echo $num-1;?>&title=<?php echo $title_encode;?>">上一篇</a>]
<?php
				}
?>
[<a href="bbsmailbox.php?path=<?php echo $dirname;?>&title=<?php echo $title_encode;?>&start=<?php echo $num-10;?>">返回信件列表</a>]
<?php
				if($num < $total-1){
?>
[<a href="bbsmailcon.php?dir=<?php echo $dirname;?>&num=<?php echo $num+1;?>&title=<?php echo $title_encode;?>">下一篇</a>]
<?php
				}
?>
[<a href="bbspstmail.php?dir=<?php echo $dirname ?>&userid=<?php echo $articles[0]["OWNER"]; ?>&num=<?php echo $num; ?>&file=<?php echo $articles[0]["FILENAME"]; ?>&title=<?php if(strncmp($articles[0]["TITLE"],"Re:",3)) echo "Re: "; ?><?php echo urlencode($articles[0]["TITLE"]); ?>">回信</a>]
</div>
<form action="bbsmailfwd.php" method="post" class="medium">
<input type="hidden" name="dir" value="<?php echo $dirname;?>"/>
<input type="hidden" name="id" value="<?php echo $num;?>"/>
	<fieldset><legend>转寄信件</legend>
		<div class="inputs">
		<label>对方的id或email:</label><input type="text" name="target" size="20" maxlength="69" value="<?php echo $currentuser["userid"];?>"/><br/>
		<input type="checkbox" name="big5" id="big5" value="1"/><label for="big5" class="clickable">BIG5</label>
		<input type="checkbox" name="noansi" id="noansi" value="1"/><label for="noansi" class="clickable">过滤ANSI</label>
		<input type="submit" value="转寄"/>
	</div></fieldset>
</form>
<form action="bbsmailcross.php" method="post" class="medium">
<input type="hidden" name="filename" value="<?php echo $articles[0]["FILENAME"];?>"/>
<input type="hidden" name="title" value="<?php echo urlencode($articles[0]["TITLE"]);?> "/>
	<fieldset><legend>转贴到版面</legend>
		<div class="inputs">
		<label>版面名称:</label><input type="text" name="target" size="20" maxlength="69" value=""/>
		<input type="checkbox" name="outgo" checked />转信
		<input type="submit" value="转贴"/>
	</div></fieldset>
</form><br /><br />
<?php
	bbs_setmailreaded($dir,$num);
	page_footer();
?>
