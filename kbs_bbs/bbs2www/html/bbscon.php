<?php
	/**
	 * This file displays article to user.
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
		if (isset($_GET["board"]))
			$board = $_GET["board"];
		else {
			html_init("gb2312");
			html_error_quit("错误的讨论区");
		}
		// 检查用户能否阅读该版
		$brdarr = array();
		$brdnum = bbs_getboard($board, $brdarr);
		if ($brdnum == 0) {
			html_init("gb2312");
			html_error_quit("错误的讨论区");
		}
		$usernum = $currentuser["index"];
		if (bbs_checkreadperm($usernum, $brdnum) == 0) {
			html_init("gb2312");
			html_error_quit("错误的讨论区");
		}
		$total = bbs_countarticles($brdnum, $dir_modes["NORMAL"]);
		if ($total <= 0) {
			html_init("gb2312");
			html_error_quit("本讨论区目前没有文章");
		}
		if (isset($_GET["id"]))
			$id = $_GET["id"];
		else {
			html_init("gb2312");
			html_error_quit("错误的文章号");
		}
		$articles = bbs_get_records_from_id($brdarr["NAME"], $id, 
				$dir_modes["NORMAL"]);
		if ($articles == FALSE)
		{
			html_init("gb2312");
			html_error_quit("bbs_get_records_from_id failed.");
		}
		else
		{
			$filename=bbs_get_board_filename($brdarr["NAME"], $articles[1]["FILENAME"]);
/*TODO: check 公开版面和私下版面
            if (cache_header("public",filemtime($filename),300))
                return;
*/
			Header("Cache-control: nocache");
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
				Header("Content-Disposition: attachment; filename=" . $attachname);
				echo fread($file,$attachsize);
				fclose($file);
				exit;
			} else
			{
				html_init("gb2312");
?>
<body>
<?php
				bbs_printansifile($filename,1,$_SERVER['PHP_SELF'] . '?' . $_SERVER['QUERY_STRING']);
			}
		}
		html_normal_quit();
	}
?>
