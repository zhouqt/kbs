<?php
	/**
	 * This file displays article to user.
	 * $Id$
	 */
	require("funcs.php");
login_init();

function display_navigation_bar_in($brdarr, $articles, $num, $brdnum )
{
	global $currentuser;

	$brd_encode = urlencode($brdarr["NAME"]);
	$PAGE_SIZE = 20;
?>
<table width="100%" border="0" cellspacing="0" cellpadding="0" class="b7">
<tr><td align="left">
<a href="/bbspst.php?board=<?php echo $brd_encode; ?>&reid=<?php echo $articles[1]["ID"];?>"><img src="images/reply.gif" border="0" alt="回复帖子" align="absmiddle"></a>
<a href="bbspst.php?board=<?php echo $brd_encode; ?>"><img src="images/postnew.gif" border="0" alt="发表话题" align="absmiddle"></a>
</td><td align="right">
[<a class="b7" href="<?php echo $_SERVER["PHP_SELF"]; ?>?bid=<?php echo $brdnum; ?>&id=<?php echo $articles[1]["ID"]; ?>&p=p"><font class="b7">上一篇</font></a>]
[<a class="b7" href="<?php echo $_SERVER["PHP_SELF"]; ?>?bid=<?php echo $brdnum; ?>&id=<?php echo $articles[1]["ID"]; ?>&p=n"><font class="b7">下一篇</font></a>]
[<a class="b7" href="<?php echo $_SERVER["PHP_SELF"]; ?>?bid=<?php echo $brdnum; ?>&id=<?php echo $articles[1]["ID"]; ?>&p=tp"><font class="b7">同主题上篇</font></a>]
[<a class="b7" href="<?php echo $_SERVER["PHP_SELF"]; ?>?bid=<?php echo $brdnum; ?>&id=<?php echo $articles[1]["ID"]; ?>&p=tn"><font class="b7">同主题下篇</font></a>]
</td></tr></table>
<?php
}

function display_navigation_bar_out($brdarr, $articles, $num, $brdnum)
{
	global $currentuser;

	$brd_encode = urlencode($brdarr["NAME"]);
	$PAGE_SIZE = 20;

	if( $articles[1]["ATTACHPOS"] == 0)
	{
?>
[<a href="/bbsfwd.php?board=<?php echo $brd_encode; ?>&id=<?php echo $articles[1]["ID"]; ?>">转寄</a>]
[<a href="bbsccc.php?board=<?php echo $brd_encode; ?>&id=<?php echo $articles[1]["ID"]; ?>">转贴</a>]
<?php
	}
?>
[<?php bbs_add_super_fav ($articles[1]['TITLE'], "/bbscon.php?bid=" . $brdnum . "&id=" . $articles[1]["ID"]); ?>]
[<a href="/bbspstmail.php?board=<?php echo $brd_encode; ?>&file=<?php echo $articles[1]["FILENAME"]; ?>&userid=<?php echo $articles[1]["OWNER"]; ?>&title=<?php if(strncmp($articles[1]["TITLE"],"Re:",3)) echo "Re: "; ?><?php echo urlencode($articles[1]["TITLE"]); ?>">回信</a>]
[<a href="/bbsedit.php?board=<?php echo $brd_encode; ?>&id=<?php echo $articles[1]["ID"]; ?>">修改</a>]
[<a href="/bbsedittitle.php?board=<?php echo $brd_encode; ?>&id=<?php echo $articles[1]["ID"]; ?>">修改标题</a>]
[<a onclick="return confirm('你真的要删除本文吗?')" href="bbsdel.php?board=<?php echo $brd_encode; ?>&file=<?php echo $articles[1]["FILENAME"]; ?>">删除</a>]
[<a href="/bbstcon.php?board=<?php echo $brd_encode; ?>&gid=<?php echo $articles[1]["GROUPID"]; ?>">同主题</a>]
[<a href="/bbscon.php?board=<?php echo $brd_encode; ?>&id=<?php echo $articles[1]["GROUPID"]; ?>">同主题第一篇</a>]
[<a href="/bbstcon.php?board=<?php echo $brd_encode; ?>&gid=<?php echo $articles[1]["GROUPID"]; ?>&start=<?php echo $articles[1]["ID"]; ?>">从此处展开</a>]
[<a href="/bbsdoc.php?board=<?php echo $brd_encode; ?>&page=<?php echo intval(($num + $PAGE_SIZE - 1) / $PAGE_SIZE); ?>">返回版面</a>]
[<a href="javascript:history.go(-1)">快速返回</a>]
<?php
}
	$brdarr = array();
	if( isset( $_GET["bid"] ) ){
		$brdnum = $_GET["bid"] ;
                settype($brdnum,"integer");
		if( $brdnum == 0 ){
			html_init("gb2312","","",1);
			html_error_quit("错误的讨论区!");
		}
		$board = bbs_getbname($brdnum);
		if( !$board ){
			html_init("gb2312","","",1);
			html_error_quit("错误的讨论区");
		}
		if( $brdnum != bbs_getboard($board, $brdarr) ){
			html_init("gb2312","","",1);
			html_error_quit("错误的讨论区");
		}
	}
	elseif (isset($_GET["board"])){
		$board = $_GET["board"];
		$brdnum = bbs_getboard($board, $brdarr);
		if ($brdnum == 0) {
			html_init("gb2312","","",1);
			html_error_quit("错误的讨论区");
		}
	}
	elseif (isset($_SERVER['argv'])){
		$board = $_SERVER['argv'][1];
		$brdnum = bbs_getboard($board, $brdarr);
		if ($brdnum == 0) {
			html_init("gb2312","","",1);
			html_error_quit("错误的讨论区");
		}
	}
	else {
		html_init("gb2312","","",1);
		html_error_quit("错误的讨论区");
	}
               $isnormalboard=bbs_normalboard($board);
               if (($loginok != 1)&&!$isnormalboard) {
                   html_nologin();
                   return;
               }
        bbs_set_onboard($brdnum,1);

	if($loginok == 1)
		$usernum = $currentuser["index"];

	if (!$isnormalboard && bbs_checkreadperm($usernum, $brdnum) == 0) {
		html_init("gb2312","","",1);
		html_error_quit("错误的讨论区");
	}
	if (isset($_GET["id"]))
		$id = $_GET["id"];
	elseif (isset($_SERVER['argv'][2]))
		$id = $_SERVER['argv'][2];
	else {
		html_init("gb2312","","",1);
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
		html_init("gb2312","","",1);
		html_error_quit("本讨论区目前没有文章,$brdnum,$board,$ftype,$total".$brdarr["NAME"]);
	}
	$articles = array ();
	$num = bbs_get_records_from_id($brdarr["NAME"], $id, 
			$ftype, $articles);
	if ($num == 0)
	{
		html_init("gb2312","","",1);
		html_error_quit("错误的文章号,原文可能已经被删除");
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
			require_once("attachment.php");
			output_attachment($filename, $attachpos);
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
			html_init("gb2312","","",1);
?>
<body topmargin="0">
<!-- 暂时这样改，比较 ugly  -->
<script language="javascript">
<?php
                      $query_str = urlencode($_SERVER["PHP_SELF"] . "?" . $_SERVER["QUERY_STRING"]);
                      $home_url = "/guest-frames.html?mainurl=" . $query_str;
?>
var strHomeURL = '<?php echo $home_url; ?>';
var strBBSName = '<?php echo BBS_FULL_NAME; ?>';
var strDesc = '<?php echo htmlspecialchars($brdarr["DESC"]); ?>';
</script>
<script language="javascript" src="bbscon.js"></script>
<tr><td align="center" class="b4"><?php echo $brdarr["NAME"]; ?> 版</td></tr>
<tr><td class="b1">
<table width="100%" border="0" cellspacing="0" cellpadding="3" class="t1">
<tr><td class="t2">
<?php
			display_navigation_bar_in($brdarr, $articles, $num, $brdnum );
?>
</td></tr>
<tr><td class="t5">
<font class="content"><script language="Javascript" src="jscon.php?ftype=<?php echo $ftype; ?>&bid=<?php echo $brdarr["BID"]; ?>&id=<?php echo $articles[1]["ID"]; ?>">
</script>
</font>
</td></tr>
</td></tr>
<tr><td class="t2">
<?php
			display_navigation_bar_in($brdarr, $articles, $num, $brdnum );
?>
</td></tr>
</table></td></tr>
<tr><td height="20" class="b1"> </td></tr>
<tr><td align="center" class="b1">
<?php
			display_navigation_bar_out($brdarr, $articles, ($ftype == $dir_modes["ZHIDING"]) ? 0 : $num, $brdnum);
?>
</td></tr>
</table>
<?php
		}
	}
	if ($loginok==1&&($currentuser["userid"] != "guest"))
		bbs_brcaddread($brdarr["NAME"], $articles[1]["ID"]);
	html_normal_quit();
?>
