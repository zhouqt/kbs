<?php
	/**
	 * This file displays mail .
	 * $Id$
	 */
	require("funcs.php");
function get_mimetype($name)
{
	$dot = strrchr($name, '.');
	if ($dot == $name)
		return "text/plain; charset=gb2312";
	if (strcasecmp($dot, ".html") == 0 || strcasecmp($dot, ".htm") == 0)
		return "text/html; charset=gb2312";
	if (strcasecmp($dot, ".jpg") == 0 || strcasecmp($dot, ".jpeg") == 0)
		return "image/jpeg";
	if (strcasecmp($dot, ".gif") == 0)
		return "image/gif";
	if (strcasecmp($dot, ".png") == 0)
		return "image/png";
	if (strcasecmp($dot, ".pcx") == 0)
		return "image/pcx";
	if (strcasecmp($dot, ".css") == 0)
		return "text/css";
	if (strcasecmp($dot, ".au") == 0)
		return "audio/basic";
	if (strcasecmp($dot, ".wav") == 0)
		return "audio/wav";
	if (strcasecmp($dot, ".avi") == 0)
		return "video/x-msvideo";
	if (strcasecmp($dot, ".mov") == 0 || strcasecmp($dot, ".qt") == 0)
		return "video/quicktime";
	if (strcasecmp($dot, ".mpeg") == 0 || strcasecmp($dot, ".mpe") == 0)
		return "video/mpeg";
	if (strcasecmp($dot, ".vrml") == 0 || strcasecmp($dot, ".wrl") == 0)
		return "model/vrml";
	if (strcasecmp($dot, ".midi") == 0 || strcasecmp($dot, ".mid") == 0)
		return "audio/midi";
	if (strcasecmp($dot, ".mp3") == 0)
		return "audio/mpeg";
	if (strcasecmp($dot, ".pac") == 0)
		return "application/x-ns-proxy-autoconfig";
	if (strcasecmp($dot, ".txt") == 0)
		return "text/plain; charset=gb2312";
	if (strcasecmp($dot, ".xht") == 0 || strcasecmp($dot, ".xhtml") == 0)
		return "application/xhtml+xml";
	if (strcasecmp($dot, ".xml") == 0)
		return "text/xml";
	return "application/octet-stream";
}

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
				$filep = fopen($filename, "rb");
				fseek($filep,$attachpos);
				$attachname='';
				while (1) {
					$char=fgetc($filep);
					if (ord($char)==0) break;
					$attachname=$attachname . $char;
				}
				$str=fread($filep,4);
				$array=unpack('Nsize',$str);
				$attachsize=$array["size"];
				Header("Content-type: " . get_mimetype($attachname));
				Header("Accept-Ranges: bytes");
				Header("Accept-Length: " . $attachsize);
				Header("Content-Disposition: filename=" . $attachname);
				echo fread($filep,$attachsize);
				fclose($filep);
				exit;
			} else
			{
				html_init("gb2312");
?>
<body>
<center><p><?php echo BBS_FULL_NAME; ?> -- 阅读信件 - <?php echo $title; ?> [使用者: <?php echo $currentuser["userid"]; ?>]</p>
<hr class="default" />
<table width="610" border="1">
<tr><td>
<?php
				echo bbs_printansifile($filename,1,$_SERVER['PHP_SELF'] . '?' . $_SERVER['QUERY_STRING']);
?>
</td></tr></table>
<hr>
[<a onclick='return confirm("你真的要删除这封信吗？")' href="/bbsdelmail.php?dir=<?php echo $dirname;?>&file=<?php echo $file;?>&title=<?php echo $title;?>">删除</a>]
[<a href="javascript:history.go(-1)">返回上一页</a>]
<?php
				if($num > 0){
?>
[<a href="/bbsmailcon.php?dir=<?php echo $dirname;?>&num=<?php echo $num-1;?>&title=<?php echo $title;?>">上一篇</a>]
<?php
				}
?>
[<a href="/bbsreadmail.php?path=<?php echo $dirname;?>&title=<?php echo $title;?>">返回信件列表</a>]
<?php
				if($num < $total-1){
?>
[<a href="/bbsmailcon.php?dir=<?php echo $dirname;?>&num=<?php echo $num+1;?>&title=<?php echo $title;?>">下一篇</a>]
<?php
				}
?>
[<a href="/bbspstmail.php?userid=<?php echo $articles[0]["OWNER"]; ?>&file=<?php echo $articles[0]["FILENAME"]; ?>&title=<?php if(strncmp($articles[0]["TITLE"],"Re:",3)) echo "Re: "; ?><?php echo urlencode($articles[0]["TITLE"]); ?>">回信</a>]
</center>
<?php
			}
		bbs_setmailreaded($dir,$num);
		html_normal_quit();
	}
?>
