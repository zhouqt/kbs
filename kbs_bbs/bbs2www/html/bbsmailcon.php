<?php
	/**
	 * This file displays mail .
	 * $Id$
	 */
	require("funcs.php");
login_init();

	if ($loginok != 1)
		html_nologin();
	else
	{
		if (isset($_GET["num"]))
			$num = $_GET["num"];
		else {
			html_init("gb2312");
			html_error_quit("错误的参数5");
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
			html_init("gb2312");
			html_error_quit("错误的参数2");
		}
		$dir = "mail/".strtoupper($currentuser["userid"]{0})."/".$currentuser["userid"]."/".$dirname ;

		$total = filesize( $dir ) / 256 ;
		if( $total <= 0 ){
			html_init("gb2312");
			html_error_quit("信笺不存在");
		}

		$articles = array ();
		if( bbs_get_records_from_num($dir, $num, $articles) ) {
			$file = $articles[0]["FILENAME"];
		}else{
			html_init("gb2312");
			html_error_quit("错误的参数4");
		}

		$filename = "mail/".strtoupper($currentuser["userid"]{0})."/".$currentuser["userid"]."/".$file ;
		if(! file_exists($filename)){
			html_init("gb2312");
			html_error_quit("信件不存在...");
		}

			@$attachpos=$_GET["ap"];//pointer to the size after ATTACHMENT PAD
			if ($attachpos!=0) {
				require_once("attachment.php");
				output_attachment($filename, $attachpos);
				exit;
			} else
			{
				html_init("gb2312");
?>
<body>
<center><p><?php echo BBS_FULL_NAME; ?> -- 阅读信件 - <?php echo $title; ?> [使用者: <?php echo $currentuser["userid"]; ?>]</p>
<hr class="default" />
<table width="610" border="0" cellpadding="5" cellspacing="0" class="t1" >
<tr><td class="t5" height="200" valign="top" align="left"><font class="content">
<?php
				bbs_print_article($filename,1,$_SERVER['PHP_SELF'] . '?' . $_SERVER['QUERY_STRING']);
?>
&nbsp;
</font>
</td></tr></table>
<hr>
[<a onclick='return confirm("你真的要删除这封信吗？")' href="/bbsmailact.php?act=del&dir=<?php echo $dirname;?>&file=<?php echo $file;?>&title=<?php echo $title_encode;?>">删除</a>]
[<a href="javascript:history.go(-1)">返回上一页</a>]
<?php
				if($num > 0){
?>
[<a href="/bbsmailcon.php?dir=<?php echo $dirname;?>&num=<?php echo $num-1;?>&title=<?php echo $title_encode;?>">上一篇</a>]
<?php
				}
?>
[<a href="/bbsmailbox.php?path=<?php echo $dirname;?>&title=<?php echo $title_encode;?>&start=<?php echo $num-10;?>">返回信件列表</a>]
<?php
				if($num < $total-1){
?>
[<a href="/bbsmailcon.php?dir=<?php echo $dirname;?>&num=<?php echo $num+1;?>&title=<?php echo $title_encode;?>">下一篇</a>]
<?php
				}
?>
[<a href="/bbspstmail.php?userid=<?php echo $articles[0]["OWNER"]; ?>&file=<?php echo $articles[0]["FILENAME"]; ?>&title=<?php if(strncmp($articles[0]["TITLE"],"Re:",3)) echo "Re: "; ?><?php echo urlencode($articles[0]["TITLE"]); ?>">回信</a>]
</center><br>
<center>
<form action="/bbsmailfwd.php" method="post">
<input type="hidden" name="dir" value="<?php echo $dirname;?>">
<input type="hidden" name="id" value="<?php echo $num;?>">
[把信件转寄给 <input type="text" name="target" size="20" maxlength="69" value="<?php echo $currentuser["userid"];?>"> (输入对方的id或email)
(<input type="checkbox" name="big5" value="1">BIG5)
(<input type="checkbox" name="noansi" value="1">过滤ANSI)
<input type="submit" name="submit" value="转寄">]
</form>
</center>
<?php
			}
		bbs_setmailreaded($dir,$num);
		html_normal_quit();
	}
?>
