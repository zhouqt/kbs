<?php
	/**
	 * This file lists boards to user.
	 * $Id$
	 */
	require("funcs.php");
	if ($loginok !=1 )
		html_nologin();
	else
	{
		html_init("gb2312");
		if (isset($_GET["group"]))
			$group = $_GET["group"];
		else
			$group = 0;
		settype($group, "integer");
		if (isset($_GET["yank"]))
			$yank = $_GET["yank"];
		else
			$yank = 0;
		settype($yank, "integer");
		if ($group < 0 || $group > sizeof($section_nums))
			html_error_quit("错误的参数");
		$boards = bbs_getboards($section_nums[$group], $yank);
		//print_r($boards);
		if ($boards == FALSE)
			html_error_quit("该分区尚未有版面");
?>
<style type="text/css">A {color: #0000f0}</style>
<body>
<center><?php echo BBS_FULL_NAME; ?> -- 分类讨论区 [<?php echo $section_names[$group][0]; ?>]
<?php
		if ($yank == 0)
		{
?>
[<a href="<?php echo $_SERVER["PHP_SELF"]; ?>?group=<?php echo $group; ?>&yank=1">本区所有看板</a>]
<?php
		}
		else
		{
?>
[<a href="<?php echo $_SERVER["PHP_SELF"]; ?>?group=<?php echo $group; ?>">本区已订阅看板</a>]
<?php
		}
?>
[<a href="/bbssec.php">分类讨论区</a>]
<hr class="default"/>
<table width="610">
<tr>
<td>序号</td>
<td>未</td>
<td>讨论区名称</td>
<td>类别</td>
<td>中文描述</td>
<td>版主</td>
<td>文章数</td>
</tr>
<?php
		$brd_name = $boards["NAME"]; // 英文名
		$brd_desc = $boards["DESC"]; // 中文描述
		$brd_class = $boards["CLASS"]; // 版分类名
		$brd_bm = $boards["BM"]; // 版主
		$brd_artcnt = $boards["ARTCNT"]; // 文章数
		$brd_unread = $boards["UNREAD"]; // 未读标记
		$brd_zapped = $boards["ZAPPED"]; // 是否被 z 掉
		$rows = sizeof($brd_name);
		for ($i = 0; $i < $rows; $i++)	
		{
?>
<tr>
<td><?php echo $i+1; ?></td>
<td>
<?php
			if ($brd_unread[$i] == 1)
				echo "◆";
			else
				echo "◇";
?>
</td>
<td>
<?php
			if ($brd_zapped[$i] == 1)
				echo "*";
			else
				echo "&nbsp;";
?><a href="/bbsdoc.php?board=<?php echo urlencode($brd_name[$i]); ?>"><?php echo $brd_name[$i]; ?></a>
</td>
<td><?php echo $brd_class[$i]; ?></td>
<td>
<a href="/bbsdoc.php?board=<?php echo urlencode($brd_name[$i]); ?>"><?php echo $brd_desc[$i]; ?></a>
</td>
<td>
<?php
			$bms = explode(" ", trim($brd_bm[$i]));
			if (strlen($bms[0]) == 0 || $bms[0][0] <= chr(32))
				echo "诚征版主中";
			else
			{
				if (!ctype_print($bms[0][0]))
					echo $bms[0];
				else
				{
?>
<a href="/cgi-bin/bbs/bbsqry?userid=<?php echo $bms[0]; ?>"><?php echo $bms[0]; ?></a>
<?php
				}
			}
?>
</td>
<td><?php echo $brd_artcnt[$i]; ?></td>
</tr>
<?php
		}
?>
</table>
<hr class="default"/>
</center>
<?php
		html_normal_quit();
	}
?>
