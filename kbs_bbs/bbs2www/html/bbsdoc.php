<?php
	/**
	 * This file lists articles to user.
	 * $Id$
	 */
	require("funcs.php");

	function display_navigation_bar($brdarr,$brdnum,$start,$total,$page,$order=FALSE)
	{
		global $section_names;
		$brd_encode = urlencode($brdarr["NAME"]);
?>
<a href="bbspst.php?board=<?php echo $brd_encode; ?>">发表文章</a>
<a href="javascript:location=location">刷新</a>
<?php
		if ($order)
		{
			if ($start <= $total - 20)
			{
?>
<a href="<?php echo $_SERVER["PHP_SELF"]; ?>?board=<?php echo $brd_encode; ?>">第一页</a>
<a href="<?php echo $_SERVER["PHP_SELF"]; ?>?board=<?php echo $brd_encode; ?>&page=<?php echo $page + 1; ?>">上一页</a>
<?php
			}
			else
			{
?>
第一页 上一页
<?php
			}
			if ($page > 1)
			{
?>
<a href="<?php echo $_SERVER["PHP_SELF"]; ?>?board=<?php echo $brd_encode; ?>&page=<?php echo $page - 1; ?>">下一页</a>
<a href="<?php echo $_SERVER["PHP_SELF"]; ?>?board=<?php echo $brd_encode; ?>&page=1">最后一页</a>
<?php
			}
			else
			{
?>
下一页 最后一页
<?php
			}
		}
		else
		{
			if ($page > 1)
			{
?>
<a href="<?php echo $_SERVER["PHP_SELF"]; ?>?board=<?php echo $brd_encode; ?>&page=1">第一页</a>
<a href="<?php echo $_SERVER["PHP_SELF"]; ?>?board=<?php echo $brd_encode; ?>&page=<?php echo $page - 1; ?>">上一页</a>
<?php
			}
			else
			{
?>
第一页 上一页
<?php
			}
			if ($start <= $total - 20)
			{
?>
<a href="<?php echo $_SERVER["PHP_SELF"]; ?>?board=<?php echo $brd_encode; ?>&page=<?php echo $page + 1; ?>">下一页</a>
<a href="<?php echo $_SERVER["PHP_SELF"]; ?>?board=<?php echo $brd_encode; ?>">最后一页</a>
<?php
			}
			else
			{
?>
下一页 最后一页
<?php
			}
		}
		if (bbs_is_bm($brdnum, $usernum))
		{
?>
<a href="bbsmdoc.php?board=<?php echo $brd_encode; ?>">管理模式</a>
<?php
		}
?>
<a href="bbsnot.php?board=<?php echo $brd_encode; ?>">进版画面</a>
<a href="bbsgdoc.php?board=<?php echo $brd_encode; ?>">文摘区</a>
<a href="bbstdoc.php?board=<?php echo $brd_encode; ?>">主题模式</a>
<?php
		$ann_path = bbs_getannpath($brdarr["NAME"]);
		if ($ann_path != FALSE)
		{
                    if (!strncmp($ann_path,"0Announce/",10))
			$ann_path=substr($ann_path,9);
?>
<a href="/cgi-bin/bbs/bbs0an?path=<?php echo urlencode($ann_path); ?>">精华区</a>
<?php
		}
?>
<a href="/cgi-bin/bbs/bbsbfind?board=<?php echo $brd_encode; ?>">版内查询</a>
<?php
		if (strcmp($currentuser["userid"], "guest") != 0)
		{
?>
<a href="/cgi-bin/bbs/bbsclear?board=<?php echo $brd_encode; ?>&start=<?php echo $start; ?>">清除未读</a>
<a href="/bbsshowvote.php?board=<?php echo $brd_encode; ?>">本版投票</a>
<a href="/bbsshowtmpl.php?board=<?php echo $brd_encode; ?>">本版发文摸板</a>
<?php
		}
		$sec_index = get_secname_index($brdarr["SECNUM"]);
		if ($sec_index >= 0)
		{
?>
<a href="/bbsboa.php?group=<?php echo $sec_index; ?>">返回[<?php echo $section_names[$sec_index][0]; ?>]</a>
<?php
		}
	}
	
	function display_articles($brdarr,$articles,$start,$order=FALSE)
	{
		global $dir_modes;
		global $default_dir_mode;
		$brd_encode = urlencode($brdarr["NAME"]);
?>
<table width="613">
<tr><td>序号</td><td>标记</td><td>作者</td><td>日期</td><td>标题</td></tr>
<?php
		$ding_cnt = 0;
		foreach ($articles as $article)
		{
			$flags = $article["FLAGS"];
			if (!strncmp($flags,"D",1)||!strncmp($flags,"d",1))
				$ding_cnt++;
		}
		$i = 0;
		if ($order) {
			$articles = array_reverse($articles);
			$i = count($articles) - $ding_cnt - 1;
		}
		foreach ($articles as $article)
		{
			$title = $article["TITLE"];
			if (strncmp($title, "Re: ", 4) != 0)
				$title = "● " . $title;

			$flags = $article["FLAGS"];
?>
<tr>
<?php
			if (!strncmp($flags,"D",1)||!strncmp($flags,"d",1)) {
?>
<td colspan="2" align="center"><strong>[提示]</strong></td>
<?php

			} else {
?>
<td><?php echo $start+$i; ?></td>
<td>
<?php
			if ($flags[1] == 'y')
			{
				if ($flags[0] == ' ')
				{
?>
<font class="f208">&nbsp;</font>
<?php
				}
				else
				{
?>
<font class="f002"><?php echo $flags[0]; ?></font>
<?php
				}
			}
			elseif ($flags[0] == 'N' || $flags[0] == '*')
			{
?>
<font class="f008"><?php echo $flags[0]; ?></font>
<?php
			}
			else
				echo $flags[0];
			echo $flags[3];
?>
</td>
<?php
	}//置顶
?>
<td><a href="/cgi-bin/bbs/bbsqry?userid=<?php echo $article["OWNER"]; ?>"><?php echo $article["OWNER"]; ?></a></td>
<td><?php echo strftime("%b&nbsp;%e", $article["POSTTIME"]); ?></td>
<td>
<?php
	switch ($default_dir_mode)
	{
	case $dir_modes["ORIGIN"]:
		if (!strncmp($flags,"D",1)||!strncmp($flags,"d",1))
		{
?>
<a href="/bbscon.php?board=<?php echo $brd_encode; ?>&id=<?php echo $article["ID"]; ?>&ftype=9"><?php echo htmlspecialchars($title); ?>

</a>
<?php
		}
		else
		{
?>
<a href="/cgi-bin/bbs/bbstcon?board=<?php echo $brd_encode; ?>&file=<?php echo $article["FILENAME"]; ?>"><?php echo htmlspecialchars($title); ?>

</a>
<?php
		}
		break;
	case $dir_modes["NORMAL"]:
	default:
?>
<a href="/bbscon.php?board=<?php echo $brd_encode; ?>&id=<?php echo $article["ID"]; ?><?php if (!strncmp($flags,"D",1)||!strncmp($flags,"d",1)) echo "&ftype=9"; ?>"><?php echo htmlspecialchars($title); ?>

</a>
<?php
	}
?>
</td>
</tr>
<?php
			if ($order)
				$i--;
			else
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
		html_init("gb2312");
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
		if ($brdarr["FLAG"]&BBS_BOARD_GROUP) {
			for ($i=0;$i<sizeof($section_nums);$i++)
				if (!strcmp($section_nums[$i],$brdarr["SECNUM"])) {
			         Header("Location: bbsboa.php?group=" . $i . "&group2=" . $brdnum);
			         return;
                                }
			html_error_quit("错误的讨论区");
		}
		if (!isset($default_dir_mode))
			$default_dir_mode = $dir_modes["NORMAL"];
		$total = bbs_countarticles($brdnum, $default_dir_mode);
		if ($total <= 0)
		    if (strcmp($currentuser["userid"], "guest") != 0)
			html_error_quit("本讨论区目前没有文章<br /><a href=\"bbspst.php?board=" . $board . "\">发表文章</a>");
                    else
			html_error_quit("本讨论区目前没有文章");
        	bbs_set_onboard($brdnum,1);
		$artcnt = 20;
		if (isset($_GET["page"]))
			$page = $_GET["page"];
		elseif (isset($_POST["page"]))
			$page = $_POST["page"];
		else
		{
			if (isset($_GET["start"]))
			{
				$start = $_GET["start"];
				settype($start, "integer");
				$page = ($start + $artcnt - 1) / $artcnt;
			}
			else
				$page = 0;
		}
		settype($page, "integer");
		if ($page > 0)
			$start = ($page - 1) * $artcnt + 1;
		else
			$start = 0;
		/*
		 * 这里存在一个时间差的问题，可能会导致序号变乱。
		 * 原因在于两次调用 bbs_countarticles() 和 bbs_getarticles()。
		 */
		if ($start == 0 || $start > ($total - $artcnt + 1))
		{
			if ($total <= $artcnt)
			{
				$start = 1;
				$page = 1;
			}
			else
			{
				$start = ($total - $artcnt + 1);
				$page = ($start + $artcnt - 1) / $artcnt + 1;
			}
		}
		else
			$page = ($start + $artcnt - 1) / $artcnt;
		settype($page, "integer");
		$articles = bbs_getarticles($brdarr["NAME"], $start, $artcnt, $default_dir_mode);
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
					$bm_url .= sprintf("<a href=\"/cgi-bin/bbs/bbsqry?userid=%s\">%s</a> ", $bm, $bm);
				}
				$bm_url = trim($bm_url);
			}
		}
		if (!isset($order_articles))
			$order_articles = FALSE;
?>
<body>
<center><p><?php echo BBS_FULL_NAME; ?> -- [讨论区: <?php echo $brdarr["NAME"]; ?>] 版主[<?php echo $bm_url; ?>] 文章数[<?php echo $total; ?>] 版内在线人数: <?php echo $brdarr["CURRENTUSERS"]+1; ?> <a href="/cgi-bin/bbs/bbsbrdadd?board=<?php echo $brdarr["NAME"]; ?>">预定本版</a></p>
<?php
		display_navigation_bar($brdarr, $brdnum, $start, $total, $page, 
					$order_articles);
?>
<hr class="default"/>
<?php
		display_articles($brdarr, $articles, $start, $order_articles);
?>
<hr class="default"/>
<?php
		display_navigation_bar($brdarr, $brdnum, $start, $total, $page, 
					$order_articles);
?>
<form name="form1" action="<?php echo $_SERVER["PHP_SELF"]; ?>" method="get">
<input type="hidden" name="board" value="<?php echo $brdarr["NAME"]; ?>"/>
<input type="submit" value="跳转到"/> 第 <input type="text" name="start" size="4"/> 篇
</form>
<?php
		html_normal_quit();
	}
?>
