<?php
	/**
	 * This file displays article to user in digest .
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
		if (isset($_GET["path"]))
			$path = $_GET["path"];
		else {
			html_init("gb2312");
			html_error_quit("错误的文章");
		}

    	if (strstr($path, ".Names") || strstr($path, "..") || strstr($path, "SYSHome")){
			html_init("gb2312");
			html_error_quit("错误的文章");
		}

		if($path[0]=='/')
			$filename="0Announce".$path;
		else
			$filename="0Announce/".$path;

		if( bbs_ann_traverse_check($filename) < 0 ) {
			html_init("gb2312");
			html_error_quit("错误的目录");
		}

		if(! file_exists($filename)){
			html_init("gb2312");
			html_error_quit("错误的文章号...");
		}

			@$attachpos=$_GET["ap"];//pointer to the size after ATTACHMENT PAD
			if ($attachpos!=0) {
				$file = fopen($filename, "rb");
				fseek($file,$attachpos);
				$attachname='';
				while (1) {
					$char=fgetc($file);
					if (ord($char)==0) break;
					$attachname=$attachname . $char;
				}
				$str=fread($file,4);
				$array=unpack('Nsize',$str);
				$attachsize=$array["size"];
				Header("Content-type: " . get_mimetype($attachname));
				Header("Accept-Ranges: bytes");
				Header("Accept-Length: " . $attachsize);
				Header("Content-Disposition: filename=" . $attachname);
				echo fread($file,$attachsize);
				fclose($file);
				exit;
			} else
			{
				html_init("gb2312");
?>
<body>
<center><p><?php echo BBS_FULL_NAME; ?> -- 精华区文章阅读 </p>
<hr class="default" />
<table width="610" border="1">
<tr><td>
<?php
				echo bbs_printansifile($filename,1,$_SERVER['PHP_SELF'] . '?' . $_SERVER['QUERY_STRING']);
?>
</td></tr></table>
<hr>
[<a href="javascript:history.go(-1)">返回上一页</a>]</center>
<?php
			}
		html_normal_quit();
	}
?>
