<?php
	/*
	** @id:windinsin nov 29,2003
	*/
	require("pcstat.php");
	
	$link = pc_db_connect();
	$newBlogs = getNewBlogs($link);
	pc_db_close($link);
	
	
	pc_html_init("gb2312","个人文集");
?>
<br><br>
<p align=center class=f2>
欢迎使用<?php echo BBS_FULL_NAME; ?>Blog系统
</p>
<p align=center class=f1>
[<a href="javascript:location=location">刷新</a>]
[<a href="pc.php">用户列表</a>]
[<a href="pcsearch2.php">文集搜索</a>]
[<a href="pcnsearch.php">文章搜索</a>]
</p>
<hr size=1>
<center>
<p align=center class=f1><strong>
最近更新的<?php echo $pcconfig["NEWS"]; ?>篇文章列表
</strong></p>
<table cellspacing=0 cellpadding=5 width=98% border=0 class=t1>
	<tr>
		<td class=t2 width=80>用户名</td>
		<td class=t2 width=160>Blog名称</td>
		<td class=t2 width=80>Blog主题</td>
		<td class=t2 width=40>访问量</td>
		<td class=t2 width=40>文章数</td>
		<td class=t2>文章主题</td>
		<td class=t2 width=120>更新时间</td>
	</tr>
	<?php
		foreach($newBlogs[useretems] as $node)
			echo "<tr>\n<td class=t4><a href='/bbsqry.php?useid=".$node[pc][USER]."'>".$node[pc][USER]."</a></td>\n".
				"<td class=t3><span title='".$node[pc][DESC]."'><a href='index.php?id=".$node[pc][USER]."'>".$node[pc][NAME]."</a>&nbsp;</span></td>\n".
				"<td class=t4>".$node[pc][THEM]."&nbsp;</td>\n".
				"<td class=t3>".$node[pc][VISIT]."</td>\n".
				"<td class=t4>".$node[pc][NODES]."</td>\n".
				"<td class=t8><a href='pccon.php?id=".$node[pc][UID]."&tid=".$node[tid]."&nid=".$node[nid]."&s=all'>".$node[title]."</a>&nbsp;</td>\n".
				"<td class=t4>".$node[created]."</td>\n</tr>\n";
	?>
</table>
<hr size=1>
<p align=center class=f1>
[<a href="javascript:location=location">刷新</a>]
[<a href="pc.php">用户列表</a>]
[<a href="pcsearch2.php">文集搜索</a>]
[<a href="pcnsearch.php">文章搜索</a>]
<br><br>
<a href="rssnew.php" target="_blank"><img src="images/xml.gif" border="0" align="absmiddle" alt="XML"></a>
</p>
</center>
<?php	
	html_normal_quit();
?>