<?php
	require("funcs.php");
	
	function show_file( $board , $brd_encode , $bid , $article , $i )
	{
?>
&nbsp;<table width="90%" cellspacing=0 cellpadding=0 class="t1">
<tr><td class="t2">
<table width="100%" cellspacing=0 cellpadding=5 class=b7>
<tr><td>
[<a href="/bbscon.php?board=<?php echo $brd_encode; ?>&id=<?php echo $article["ID"]; ?>"><font class="b7">本篇全文</font></a>]
[<a href="/bbspst.php?board=<?php echo $brd_encode; ?>&reid=<?php echo $article["ID"]; ?>"><font class="b7">回复文章</font></a>]
[<a href="/bbspstmail.php?board=<?php echo $brd_encode; ?>&file=<?php echo $article["FILENAME"]; ?>&userid=<?php echo $article["OWNER"]; ?>&title=<?php if(strncmp($article["TITLE"],"Re:",3)) echo "Re: ";  echo urlencode($article["TITLE"]); ?>"><font class="b7">回信给作者</font></a>]
[本篇作者：<a href="/bbsqry.php?userid=<?php echo $article["OWNER"]; ?>"><font class="b7"><?php echo $article["OWNER"]; ?></font></a>]
[<a href="/bbsdoc.php?board=<?php echo $brd_encode; ?>"><font class="b7">进入讨论区</font></a>]
[<a href="#top"><font class="b7">返回顶部</font></a>]
</td>
<td align=center><strong><?php echo $i; ?></strong>
</td></tr></table>
</td></tr>
<tr><td class="t8">
<font class="content">
<script language="Javascript" src="/jscon.php?bid=<?php echo $bid; ?>&id=<?php echo $article["ID"]; ?>">
</script>
</font>
</td></tr>
</table><br />
<?php		
		
	}
	
	
	$gid = $_GET["gid"];
	settype($gid, "integer");
	if( $gid < 0 ) $gid = 0 ; 
	$board = $_GET["board"];
	$brdarr = array();
	
	$bid = bbs_getboard($board , $brdarr);
	if($bid == 0)
	{
		html_init("gb2312","","",1);
		html_error_quit("错误的讨论区");
	}
	$board = $brdarr["NAME"];
	$board_desc = $brdarr["DESC"];
	$brd_encode = urlencode( $board );
	
	$isnormalboard = bbs_normalboard($board);
	if (($loginok != 1)&&!$isnormalboard) 
        {
        	html_nologin();
        	return;
        }
        bbs_set_onboard($brcnum,1);

	if($loginok == 1)
		$usernum = $currentuser["index"];
	if (!$isnormalboard && bbs_checkreadperm($usernum, $brdnum) == 0) 
	{
		html_init("gb2312","","",1);
		html_error_quit("错误的讨论区");
	}
	
	$num = bbs_get_threads_from_gid($bid, $gid, $articles);
	if( $num == 0 )
	{
		html_init("gb2312","","",1);
		html_error_quit("错误的参数");
	}
	
	/*
        ** Cache只判断了同主题的最后一篇文章，没有顾及前面文章的修改  windinsn jan 26 , 2004
         */
        if ($isnormalboard)
	{
       		$lastfilename = bbs_get_board_filename($board , $articles[$num - 1]["FILENAME"]);
		if (cache_header("public",filemtime($lastfilename),300))
        	return;
        }
        
        html_init("gb2312","","",1);
?>
<a name="top">
<table width="100%" border="0" cellspacing="0" cellpadding="3">
  <tr> 
    <td class="b2">
	    <a href="/mainpage.html" class="b2"><font class="b2"><?php echo BBS_FULL_NAME; ?></font></a>
	    -
	    <?php
	    	$sec_index = get_secname_index($brdarr["SECNUM"]);
		if ($sec_index >= 0)
		{
	    ?>
		<a href="/bbsboa.php?group=<?php echo $sec_index; ?>" class="b2"><font class="b2"><?php echo $section_names[$sec_index][0]; ?></font></a>
	    <?php
		}
	    ?>
	    -
	    <a href="/bbsdoc.php?board=<?php echo $brd_encode; ?>" class="b2"><font class="b2"><?php echo $brdarr["NAME"]; ?></font></a>版(<a href="bbsnot.php?board=<?php echo $brd_encode; ?>" class="b2"><font class="b2">进版画面</font></a>|<a href="/bbsfav.php?bname=<?php echo $brd_encode; ?>&select=-1" class="b2"><font class="b2">添加到收藏夹</font></a>)
    </td>
  </tr>
  <tr>
    <td class="b4">同主题阅读：&nbsp;&nbsp;&nbsp<?php echo htmlspecialchars($articles[0]["TITLE"]); ?></td>
  </tr>
</table>
<?php
	for( $i = 0 ; $i< $num ; $i ++ )
	{
		show_file( $board , $brd_encode , $bid , $articles[$i] , $i + 1);	
		if ($loginok==1&&($currentuser["userid"] != "guest"))
			bbs_brcaddread($board , $articles[$i]["ID"]);
	}
	
?>
<hr size=1>
<p align=center>
<a href="javascript:history.go(-1)">后退</a>
<a href="/bbsdoc.php?board=<?php echo $board; ?>">进入<strong><?php echo $board_desc; ?></strong>讨论区</a>
<a href="#top">返回顶部</a>
</p>
<?php
	html_normal_quit();
?>