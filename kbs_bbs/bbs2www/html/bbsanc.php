<?php
	/**
	 * This file displays article to user in digest .
	 * $Id$
	 */
	require("funcs.php");
	require("board.inc.php");
login_init();
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
		
		$board = '';
		$up_dirs = array();
		$up_cnt = bbs_ann_updirs($filename,$board,$up_dirs);
		
if ($board) {
    $brdarr = array();
    $bid = bbs_getboard($board,$brdarr);
    if ($bid) {
        $board = $brdarr['NAME'];
        $usernum = $currentuser['index'];
        if (bbs_checkreadperm($usernum, $bid) == 0) 
    		html_error_quit('不存在该目录');
        bbs_set_onboard($bid,1);
        if (bbs_normalboard($board)) {
            if (cache_header('public, must-revalidate',filemtime($filename),10))
                return;
        }
    }
    else {
        $board = '';
        if (cache_header('public, must-revalidate',filemtime($filename),10))
            return;
    }
    
}
else {
    if (cache_header('public, must-revalidate',filemtime($filename),10))
        return;
    $bid = 0;
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
				Header("Content-Length: " . $attachsize);
				Header("Content-Disposition: filename=" . $attachname);
				echo fread($file,$attachsize);
				fclose($file);
				exit;
			} else
			{
				html_init("gb2312","","",1);
?>
<link rel="stylesheet" type="text/css" href="/ansi.css"/>
<?php
				$bid?bbs_board_header($brdarr):bbs_ann_header($board);
				bbs_ann_xsearch($board);
?>
<center>
<table width="98%" border="0" class="t1" cellspacing="0" cellpadding="3" >
<tr><td class="t3">精华区文章阅读</td></tr>
<tr><td class="t7"><font class="content">
<?php
				bbs_print_article($filename,1,$_SERVER['PHP_SELF'] . '?' . $_SERVER['QUERY_STRING']);
?>
</font></td></tr></table></center>
<?php
            
            if ($bid)
            bbs_board_foot($brdarr,'');
    
			($up_cnt >= 2)?bbs_ann_foot($up_dirs[$up_cnt - 2]):bbs_ann_foot('');
			html_normal_quit();
			}
	}
?>
