<?php
	/**
	 * $Id$ 
	 */
	require("funcs.php");
	if ($loginok !=1 )
		html_nologin();
	else
	{
		html_init("gb2312");
?>
<body>
<center><?php echo BBS_FULL_NAME; ?> -- 分类讨论区
<hr class="default">
<table>
<tr><td>区号</td><td>类别</td><td>描述</td></tr>
<?php
		$i = 0;
		foreach ($section_names as $secname)
		{
			$i++;
?>
<tr><td><?php echo $i-1; ?></td>
<td><a href="/bbsboa.php?group=<?php echo $i-1; ?>"><?php echo $secname[0]; ?></a></td>
<td><a href="/bbsboa.php?group=<?php echo $i-1; ?>"><?php echo $secname[1]; ?></a></td></tr>
<?php
		}
?>
</table>
<hr class="default">
[<a href="/cgi-bin/bbs/bbsall">全部讨论区</a>]
[<a href="/cgi-bin/bbs/bbs0an">精华公布栏</a>]
</center>
<?php
	}
	@require("ad.php");
?>
</html>
