<?php
	/**
	 * This file lists articles to user.
	 * $Id$
	 */
	require("funcs.php");
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
		$total = bbs_countarticles($brdnum, $dir_modes["DIGEST"]);
		if ($total <= 0)
			html_error_quit("本讨论区目前没有文章");
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
					$bm_url .= sprintf("<a href=\"/cgi-bin/bbs/bbsqry?userid=%s\">%s</a> ", $bm, $bm);
				}
				$bm_url = trim($bm_url);
			}
		}
?>
<body>
<nobr/>
<center><?php echo $BBS_FULL_NAME; ?> -- [讨论区: <?php echo $brdarr["NAME"]; ?>] 版主[<?php echo $bm_url; ?>] 文章数[<?php echo $total; ?>] <a href="/cgi-bin/bbs/bbsbrdadd?board=<?php echo $brdarr["NAME"]; ?>">预定本版</a>
<hr class="default"/>
<table width="613">
<tr><td>序号</td><td>标记</td><td>作者</td><td>日期</td><td>标题</td></tr>
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
<td><?php echo $start + $i; ?></td>
<td>
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
</td>
<td><a href="/cgi-bin/bbs/bbsqry?userid=<?php echo $article["OWNER"]; ?>"><?php echo $article["OWNER"]; ?></a></td>
<td><?php echo strftime("%b&nbsp;%e", $article["POSTTIME"]); ?></td>
<td>
<a href="/cgi-bin/bbs/bbsgcon?board=<?php echo $brd_encode; ?>&file=<?php echo $article["FILENAME"]; ?>&num=<?php echo $start + $i; ?>"><?php echo htmlspecialchars($title); ?></a>
</td>
</tr>
<?php
			$i++;
		}
?>
</table>
<hr class="default"/>
<a href="bbspst.html?board=<?php echo $brd_encode; ?>&sig=<?php echo $currentuser["signature"];?>&attach=<?php
    if ($brdarr["FLAG"]&BBS_BOARD_ATTACH)
    	echo 1;
    else
        echo 0;
?>">发表文章</a>
<a href="javascript:location=location">刷新</a>
<?php
		if ($start > 0)
		{
?>
<a href="<?php echo $_SERVER["PHP_SELF"]; ?>?board=<?php echo $brd_encode; ?>&start=<?php echo $start - $artcnt; ?>">上一页</a>
<?php
		}
		if ($start < $total - 20)
		{
?>
<a href="<?php echo $_SERVER["PHP_SELF"]; ?>?board=<?php echo $brd_encode; ?>&start=<?php echo $start + $artcnt; ?>">下一页</a>
<?php
		}
		if (bbs_is_bm($brdnum, $usernum))
		{
?>
<a href="bbsmdoc.php?board=<?php echo $brd_encode; ?>">管理模式</a>
<?php
		}
?>
<a href="bbsnot.php?board=<?php echo $brd_encode; ?>">进版画面</a>
<a href="bbsdoc.php?board=<?php echo $brd_encode; ?>">普通模式</a>
<?php
		$ann_path = bbs_getannpath($brdarr["NAME"]);
		if ($ann_path != FALSE)
		{
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
<?php
		}
		$sec_index = get_secname_index($brdarr["SECNUM"]);
		if ($sec_index >= 0)
		{
?>
<a href="bbsboa.php?group=<?php echo $sec_index; ?>">返回[<?php echo $section_names[$sec_index][0]; ?>]</a>
<?php
		}
?>
<form name="form1" action="<?php echo $_SERVER["PHP_SELF"]; ?>" method="get">
<input type="hidden" name="board" value="<?php echo $brdarr["NAME"]; ?>"/>
<input type="submit" value="跳转到"/> 第 <input type="text" name="start" size="4"/> 篇
</form>
<?php
		html_normal_quit();
	}
?>
