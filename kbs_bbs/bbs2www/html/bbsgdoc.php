<?php
	/**
	 * This file lists articles to user.
	 * $Id$
	 */
	require("funcs.php");

	function display_navigation_bar($brdarr,$brdnum,$start,$total,$order=FALSE)
	{
		global $section_names;
		$brd_encode = urlencode($brdarr["NAME"]);
	?>		
<table width="100%" border="0" cellspacing="0" cellpadding="0" class="b1">		
<form name="form1" action="<?php echo $_SERVER["PHP_SELF"]; ?>" method="get">
<input type="hidden" name="board" value="<?php echo $brdarr["NAME"]; ?>"/>
<tr>
<td>
    	<?php
    		if (strcmp($currentuser["userid"], "guest") != 0)
		{
    	?>
<a href="bbspst.php?board=<?php echo $brd_encode; ?>"><img src="images/postnew.gif" border="0" alt="发表话题"></a>
    	<?php
    		}
    	?>
</td>
<td align="right">
 	<?php
		      if($order)
		      {
		   	if ($start <= $total - 20)
			{
		    ?>
[<a class="b1" href="<?php echo $_SERVER["PHP_SELF"]; ?>?board=<?php echo $brd_encode; ?>">第一页</a>]
[<a class="b1" href="<?php echo $_SERVER["PHP_SELF"]; ?>?board=<?php echo $brd_encode; ?>&start=<?php echo $start + 20; ?>">上一页</a>]
		    <?php
			}
			else
			{
		    ?>
[第一页] 
[上一页]
		    <?php
			}
			if ($start > 1)
			{
		?>
[<a class="b1" href="<?php echo $_SERVER["PHP_SELF"]; ?>?board=<?php echo $brd_encode; ?>&start=<?php if($start>20) echo $start - 20; else echo "1";?>">下一页</a>]
[<a class="b1" href="<?php echo $_SERVER["PHP_SELF"]; ?>?board=<?php echo $brd_encode; ?>&start=1">最后一页</a>]
		    <?php
			}
			else
			{
		    ?>
[下一页] 
[最后一页]
		    <?php
			}
		     }
		     else
		     {	
		     	if ($start > 1)
			{
		?>
[<a class="b1" href="<?php echo $_SERVER["PHP_SELF"]; ?>?board=<?php echo $brd_encode; ?>&start=1">第一页</a>]
[<a class="b1" href="<?php echo $_SERVER["PHP_SELF"]; ?>?board=<?php echo $brd_encode; ?>&start=<?php if($start > 20) echo $start - 20; else echo "1"; ?>">上一页</a>]
		    <?php
			}
			else
			{
		    ?>
[第一页] 
[上一页]
		    <?php
			}
			if ($start <= $total - 20)
			{
		    ?>
[<a class="b1" href="<?php echo $_SERVER["PHP_SELF"]; ?>?board=<?php echo $brd_encode; ?>&start=<?php echo $start + 20; ?>">下一页</a>]
[<a class="b1" href="<?php echo $_SERVER["PHP_SELF"]; ?>?board=<?php echo $brd_encode; ?>">最后一页</a>]
		    <?php
			}
			else
			{
		    ?>
[下一页] 
[最后一页]
		    <?php
			}
		    }
	?>
<input type="submit" class="b5" value="跳转到"/> 第 <input type="text" name="start" size="3"  onmouseover=this.focus() onfocus=this.select() class="b5"> 篇 
</td></tr></form></table>
	<?php
	}
	
	
	function display_g_articles($brdarr,$articles,$start,$order=FALSE){
?>
<table width="100%" border="0" cellspacing="0" cellpadding="3" class="t1">
<tr><td class="t2" width="50">序号</td><td class="t2" width="40">标记</td><td class="t2" width="120">作者</td><td class="t2" width="80">日期</td><td class="t2">标题</td></tr>
<?php
		$brd_encode = urlencode($brdarr["NAME"]);
		$i = 0;
		foreach ($articles as $article)
		{
			$title = $article["TITLE"];
			if (strncmp($title, "Re: ", 4) != 0)
				$title = "● " . $title;
			$flags = $article["FLAGS"];
?>
<tr>
<td class="t3"><?php echo $start + $i; ?></td>
<td class="t4">&nbsp;
<?php
			if ($flags[1] == 'y')
			{
				if ($flags[0] == ' ')
				{
?>
<font face="Webdings" color="#008000">&lt;</font>
<?php
				}
				else
				{
?>
<font color="#008000"><?php echo $flags[0]; ?></font>
<?php
				}
			}
			elseif ($flags[0] == 'N' || $flags[0] == '*')
			{
?>
<font color="#909090"><?php echo $flags[0]; ?></font>
<?php
			}
			else
				echo $flags[0];
?>
&nbsp;</td>
<td class="t3"><a class="ts1" href="/cgi-bin/bbs/bbsqry?userid=<?php echo $article["OWNER"]; ?>"><?php echo $article["OWNER"]; ?></a></td>
<td class="t4"><?php echo strftime("%b&nbsp;%e", $article["POSTTIME"]); ?></td>
<td class="t5">&nbsp;
<a class="ts2" href="/bbsgcon.php?board=<?php echo $brd_encode; ?>&file=<?php echo $article["FILENAME"]; ?>&num=<?php echo $start + $i; ?>"><?php echo htmlspecialchars($title); ?></a>
</td>
</tr>
<?php
			$i++;
		}
?>
</table>
<?php		
		}

	if ($loginok != 1)
		html_nologin();
	else
	{
		html_init("gb2312","","",1);
		if (isset($_GET["board"]))
			$board = $_GET["board"];
		else
			html_error_quit("错误的讨论区");
		// 检查用户能否阅读该版
		$brdarr = array();
		$brdnum = bbs_getboard($board, $brdarr);
		if ($brdnum == 0)
			html_error_quit("错误的讨论区");
		$usernum = $currentuser["index"];
		if (bbs_checkreadperm($usernum, $brdnum) == 0)
			html_error_quit("错误的讨论区");
		$total = bbs_countarticles($brdnum, $dir_modes["DIGEST"]);
		if ($total <= 0)
			html_error_quit("本讨论区目前没有文章");
		bbs_set_onboard($brcnum,1);

		if (isset($_GET["start"]))
			$start = $_GET["start"];
		elseif (isset($_POST["start"]))
			$start = $_POST["start"];
		else
			$start = 0;
		settype($start, "integer");
		$artcnt = 20;
		/*
		 * 这里存在一个时间差的问题，可能会导致序号变乱。
		 * 原因在于两次调用 bbs_countarticles() 和 bbs_getarticles()。
		 */
		if ($start == 0 || $start > ($total - $artcnt + 1))
			$start = ($total - $artcnt + 1);
		if ($start < 0)
			$start = 1;
		$articles = bbs_getarticles($board, $start, $artcnt, $dir_modes["DIGEST"]);
		if ($articles == FALSE)
			html_error_quit("读取文章列表失败");
		$bms = explode(" ", trim($brdarr["BM"]));
		$bm_url = "";
		if (strlen($bms[0]) == 0 || $bms[0][0] <= chr(32))
			$bm_url = "诚征版主中";
		else
		{
			if (!ctype_alpha($bms[0][0]))
				$bm_url = $bms[0];
			else
			{
				foreach ($bms as $bm)
				{
					$bm_url .= sprintf("<a href=\"/cgi-bin/bbs/bbsqry?userid=%s\" class=\"b3\">%s</a> ", $bm, $bm);
				}
				$bm_url = trim($bm_url);
			}
		}
	
		$brd_encode = urlencode($brdarr["NAME"]);
?>
<body>
<a name="listtop"></a>
<table width="100%" border="0" cellspacing="0" cellpadding="3">
  <tr> 
    <td colspan="2" class="b2">
	    <a href="bbssec.php" class="b2"><?php echo BBS_FULL_NAME; ?></a>
	    -
	    <?php
	    	$sec_index = get_secname_index($brdarr["SECNUM"]);
		if ($sec_index >= 0)
		{
	    ?>
		<a href="/bbsboa.php?group=<?php echo $sec_index; ?>" class="b2"><?php echo $section_names[$sec_index][0]; ?></a>
	    <?php
		}
	    ?>
	    -
	    <a href="/bbsdoc.php?board=<?php echo $brd_encode; ?>" class="b2"><?php echo $brdarr["NAME"]; ?>版</a>(<a href="bbsnot.php?board=<?php echo $brd_encode; ?>" class="b2">进版画面</a>|<a href="/cgi-bin/bbs/bbsbrdadd?board=<?php echo $brd_encode; ?>" class="b2">预定本版</a>)
	    -
	    文摘区
    </td>
  </tr>
  <tr> 
    <td colspan="2" align="center" class="b4"><?php echo $brdarr["NAME"]."(".$brdarr["DESC"].")"; ?> 版</td>
  </tr>
  <tr><td class="b1">
  <img src="images/bm.gif" alt="版主" align="absmiddle">版主 <?php echo $bm_url; ?>
  </td>
    <td align="right" class="b1">
	    <img src="images/gmode.gif" align="absmiddle" alt="一般模式"><a class="b1" href="bbsdoc.php?board=<?php echo $brdarr["NAME"]; ?>">一般模式</a> 
	    <?php
                    if ($ann_path = bbs_getannpath($brdarr["NAME"])){
	    ?>
	    | 
  	    <img src="images/soul.gif" align="absmiddle" alt="精华区"><a class="b1" href="/cgi-bin/bbs/bbs0an?path=<?php echo urlencode($ann_path); ?>">精华区</a>
	    <?php
		}
	    ?>
	    | 
  	    <img src="images/search.gif" align="absmiddle" alt="版内查询"><a class="b1" href="/cgi-bin/bbs/bbsbfind?board=<?php echo $brd_encode; ?>">版内查询</a>
	    <?php
    		if (strcmp($currentuser["userid"], "guest") != 0)
		{
    	    ?>
	    | 
  	    <img src="images/vote.gif" align="absmiddle" alt="本版投票"><a class="b1" href="/bbsshowvote.php?board=<?php echo $brd_encode; ?>">本版投票</a>
	    | 
  	    <img src="images/model.gif" align="absmiddle" alt="发文模板"><a class="b1" href="/bbsshowtmpl.php?board=<?php echo $brd_encode; ?>">发文模板</a>
    	    <?php
    		}
    	    ?>	
    </td>
  </tr>
  <tr> 
    <td colspan="2" height="9" background="images/dashed.gif"> </td>
  </tr>
  <tr><td colspan="2" align="right" class="b1">
  <?php
  	display_navigation_bar($brdarr, $brdnum, $start, $total, $order_articles );
  ?>
  </td></tr>
  <tr> 
    <td colspan="2" align="center">
    	<?php
		display_g_articles($brdarr,$articles,$start,$order=FALSE);
	?>	
    </td>
  </tr>
  <tr><td colspan="2" align="right" class="b1">
  <?php
  	display_navigation_bar($brdarr, $brdnum, $start, $total, $order_articles);
  ?>
  </td></tr>
  <tr> 
    <td colspan="2" height="9" background="images/dashed.gif"> </td>
  </tr>
  <tr> 
    <td colspan="2" align="center" class="b1">
    	[<a href="#listtop">返回顶部</a>]
    	[<a href="bbsdoc.php?board=<?php echo $brdarr["NAME"]; ?>">返回一般模式</a>]
    	[<a href="javascript:location=location">刷新</a>]
    	<?php
    		if (strcmp($currentuser["userid"], "guest") != 0)
		{
    	?>
    	[<a href="/cgi-bin/bbs/bbsclear?board=<?php echo $brd_encode; ?>&start=<?php echo $start; ?>">清除未读</a>]
	<?php
		}
		if (bbs_is_bm($brdnum, $usernum))
		{
	?>
	[<a href="bbsmdoc.php?board=<?php echo $brd_encode; ?>">管理模式</a>]
	<?php
		}
	?>
    </td>
  </tr>
</table>
<?php
	}
	html_normal_quit();
?>
