<?php
	/**
	 * This file displays article to user.
	 * $Id$
	 */
	$needlogin=0;
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

function display_navigation_bar($brdarr, $articles, $num, $brdnum)
{
	global $currentuser;

	$brd_encode = urlencode($brdarr["NAME"]);
	$PAGE_SIZE = 20;
?>
[<a href="<?php echo $_SERVER["PHP_SELF"]; ?>?bid=<?php echo $brdnum; ?>&id=<?php echo $articles[1]["ID"]; ?>&p=p">上一篇</a>]
[<a href="<?php echo $_SERVER["PHP_SELF"]; ?>?bid=<?php echo $brdnum; ?>&id=<?php echo $articles[1]["ID"]; ?>&p=n">下一篇</a>]
[<a href="<?php echo $_SERVER["PHP_SELF"]; ?>?bid=<?php echo $brdnum; ?>&id=<?php echo $articles[1]["ID"]; ?>&p=tp">同主题上篇</a>]
[<a href="<?php echo $_SERVER["PHP_SELF"]; ?>?bid=<?php echo $brdnum; ?>&id=<?php echo $articles[1]["ID"]; ?>&p=tn">同主题下篇</a>]
<?php
	if( $articles[1]["ATTACHPOS"] == 0)
	{
?>
[<a href="/cgi-bin/bbs/bbsfwd?board=<?php echo $brd_encode; ?>&file=<?php echo $articles[1]["FILENAME"]; ?>">转寄</a>]
[<a href="/cgi-bin/bbs/bbsccc?board=<?php echo $brd_encode; ?>&file=<?php echo $articles[1]["FILENAME"]; ?>">转贴</a>]
<?php
	}
?>
[<a onclick="return confirm('你真的要删除本文吗?')" href="bbsdel.php?board=<?php echo $brd_encode; ?>&file=<?php echo $articles[1]["FILENAME"]; ?>">删除文章</a>]
[<a href="/cgi-bin/bbs/bbsedit?board=<?php echo $brd_encode; ?>&file=<?php echo $articles[1]["FILENAME"]; ?>">修改文章</a>]
[<a href="/bbsdoc.php?board=<?php echo $brd_encode; ?>&page=<?php echo intval(($num + $PAGE_SIZE - 1) / $PAGE_SIZE); ?>">本讨论区</a>]
[<a href="/bbspst.php?board=<?php echo $brd_encode; ?>&reid=<?php echo $articles[1]["ID"];?>">回文章</a>]
[<a href="/bbspstmail.php?board=<?php echo $brd_encode; ?>&file=<?php echo $articles[1]["FILENAME"]; ?>&userid=<?php echo $articles[1]["OWNER"]; ?>&title=<?php if(strncmp($articles[1]["TITLE"],"Re:",3)) echo "Re: "; ?><?php echo urlencode($articles[1]["TITLE"]); ?>">回信给作者</a>]
[<a href="/cgi-bin/bbs/bbstfind?board=<?php echo $brd_encode; ?>&title=<?php echo urlencode($articles[1]["TITLE"]); ?>&groupid=<?php echo $articles[1]["GROUPID"];?>">同主题阅读</a>]
[<a href="javascript:history.go(-1)">快速返回</a>]
<?php
}

	$brdarr = array();
	if( isset( $_GET["bid"] ) ){
		$brdnum = $_GET["bid"] ;
		if( $brdnum == 0 ){
			html_init("gb2312");
			html_error_quit("错误的讨论区!");
		}
		$board = bbs_getbname($brdnum);
		if( !$board ){
			html_init("gb2312");
			html_error_quit("错误的讨论区");
		}
		if( $brdnum != bbs_getboard($board, $brdarr) ){
			html_init("gb2312");
			html_error_quit("错误的讨论区");
		}
	}
	elseif (isset($_GET["board"])){
		$board = $_GET["board"];
		$brdnum = bbs_getboard($board, $brdarr);
		if ($brdnum == 0) {
			html_init("gb2312");
			html_error_quit("错误的讨论区");
		}
	}
	elseif (isset($_SERVER['argv'])){
		$board = $_SERVER['argv'][1];
		$brdnum = bbs_getboard($board, $brdarr);
		if ($brdnum == 0) {
			html_init("gb2312");
			html_error_quit("错误的讨论区");
		}
	}
	else {
		html_init("gb2312");
		html_error_quit("错误的讨论区");
	}
               $isnormalboard=bbs_normalboard($board);
               if (($loginok != 1)&&!$isnormalboard) {
                   html_nologin();
                   return;
               }
        bbs_set_onboard($brcnum,1);

	if($loginok == 1)
		$usernum = $currentuser["index"];

	if (!$isnormalboard && bbs_checkreadperm($usernum, $brdnum) == 0) {
		html_init("gb2312");
		html_error_quit("错误的讨论区");
	}
	if (isset($_GET["id"]))
		$id = $_GET["id"];
	elseif (isset($_SERVER['argv'][2]))
		$id = $_SERVER['argv'][2];
	else {
		html_init("gb2312");
		html_error_quit("错误的文章号");
	}
	settype($id, "integer");
	// 获取上一篇或下一篇，同主题上一篇或下一篇的指示
	@$ptr=$_GET["p"];
	// 同主题的指示在这里处理
	if ($ptr == "tn")
	{
		$articles = bbs_get_threads_from_id($brdnum, $id, $dir_modes["NORMAL"],1);
		if ($articles == FALSE)
			$redirt_id = $id;
		else
			$redirt_id = $articles[0]["ID"];
		if (($loginok == 1) && $currentuser["userid"] != "guest")
			bbs_brcaddread($brdarr["NAME"], $redirt_id);
		header("Location: " . "/bbscon.php?bid=" . $brdnum . "&id=" . $redirt_id);
		exit;
	}
	elseif ($ptr == "tp")
	{
		$articles = bbs_get_threads_from_id($brdnum, $id, $dir_modes["NORMAL"],-1);
		if ($articles == FALSE)
			$redirt_id = $id;
		else
			$redirt_id = $articles[0]["ID"];
		if (($loginok == 1) && $currentuser["userid"] != "guest")
			bbs_brcaddread($brdarr["NAME"], $redirt_id);
		header("Location: " . "/bbscon.php?bid=" . $brdnum . "&id=" . $redirt_id);
		exit;
	}

	if (isset($_GET["ftype"])){
		$ftype = $_GET["ftype"];
		if($ftype != $dir_modes["ZHIDING"])
			$ftype = $dir_modes["NORMAL"];
	}
	else
		$ftype = $dir_modes["NORMAL"];
	$total = bbs_countarticles($brdnum, $ftype);
	if ($total <= 0) {
		html_init("gb2312");
		html_error_quit("本讨论区目前没有文章,$brdnum,$board,$ftype,$total".$brdarr["NAME"]);
	}
	$articles = array ();
	$num = bbs_get_records_from_id($brdarr["NAME"], $id, 
			$ftype, $articles);
	if ($num == 0)
	{
		html_init("gb2312");
		html_error_quit("错误的文章号.");
	}
	else
	{
		$filename=bbs_get_board_filename($brdarr["NAME"], $articles[1]["FILENAME"]);
		if ($isnormalboard) {
       			if (cache_header("public",filemtime($filename),300))
               			return;
               	}
//		Header("Cache-control: nocache");
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
			//$http_uri = "http" . ($_SERVER["HTTPS"] == "on" ? "s" : "") . "://";
			if ($ptr == 'p' && $articles[0]["ID"] != 0)
		{
				if (($loginok == 1) && $currentuser["userid"] != "guest")
					bbs_brcaddread($brdarr["NAME"], $articles[0]["ID"]);
				header("Location: " . "/bbscon.php?bid=" . $brdnum . "&id=" . $articles[0]["ID"]);
				exit;
			}
			elseif ($ptr == 'n' && $articles[2]["ID"] != 0)
			{
				if (($loginok == 1) && $currentuser["userid"] != "guest")
					bbs_brcaddread($brdarr["NAME"], $articles[2]["ID"]);
				header("Location: " ."/bbscon.php?bid=" . $brdnum . "&id=" . $articles[2]["ID"]);
				exit;
			}
			html_init("gb2312");
?>
<body>
<center><p><?php echo BBS_FULL_NAME; ?> -- 文章阅读 [讨论区: <?php echo $brdarr["NAME"]; ?>]</a></p></center>
<?php
				display_navigation_bar($brdarr, $articles, $num, $brdnum);
?>
<hr class="default" />
<table width="610" border="0">
<tr><td>
<?php
			echo bbs_printansifile($filename,1,$_SERVER['PHP_SELF'] . '?' . $_SERVER['QUERY_STRING']);
?>
</td></tr></table>
<hr class="default" />
<?php
			display_navigation_bar($brdarr, $articles, $num, $brdnum);
		}
	}
	if ($loginok==1&&($currentuser["userid"] != "guest"))
		bbs_brcaddread($brdarr["NAME"], $articles[1]["ID"]);
	html_normal_quit();
?>
