<?php
	/*
	** @id:windinsin nov 29,2003
	*/
	require("pcstat.php");

	function display_navigation_bar()
	{
?>
<p align=center class=f1>
[<a href="javascript:location=location">刷新</a>]
[<a href="pc.php">用户列表</a>]
[<a href="pcsearch2.php">文集搜索</a>]
[<a href="pcnsearch.php">文章搜索</a>]
</p>
<?php
	}

	function display_page_tools($pno,$etemno)
	{
		global $pcconfig;
?>
<p align=center class=f1>
<?php
		if($pno > 1)
			echo "[<a href='pcnew.php?pno=".($pno - 1)."'>前".$pcconfig["NEWS"]."条记录</a>]\n";	
		else
			echo "[前".$pcconfig["NEWS"]."条记录]\n";
		if($etemno == $pcconfig["NEWS"])
			echo "[<a href='pcnew.php?pno=".($pno + 1)."'>后".$pcconfig["NEWS"]."条记录</a>]\n";	
		else
			echo "[后".$pcconfig["NEWS"]."条记录]\n";
?>
</p>
<?php
	}
	
	$pno = (int)($_GET["pno"]);
	$pno = ($pno < 1)?1:$pno;
	$link = pc_db_connect();
	$newBlogs = getNewBlogs($link,$pno);
	$newBlogsNum = count($newBlogs[useretems]);
	pc_db_close($link);
	
	
	pc_html_init("gb2312","个人文集");
?>
<br><br>
<p align=center class=f2>
欢迎使用<?php echo BBS_FULL_NAME; ?>Blog系统
</p>
<?php display_navigation_bar(); ?>
<hr size=1>
<center>
<p align=center class=f1><strong>
最近更新的<?php echo $pcconfig["NEWS"]; ?>篇文章列表
</strong></p>
<?php display_page_tools($pno,$newBlogsNum); ?>
<?php
	
?>
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
		for($i=0;$i < $newBlogsNum;$i++)
			echo "<tr>\n<td class=t4><a href='/bbsqry.php?userid=".$newBlogs[useretems][$i][pc][USER]."'>".$newBlogs[useretems][$i][pc][USER]."</a></td>\n".
				"<td class=t3><span title=\"".$newBlogs[useretems][$i][pc][DESC]."\"><a href=\"index.php?id=".$newBlogs[useretems][$i][pc][USER]."\">".$newBlogs[useretems][$i][pc][NAME]."</a>&nbsp;</span></td>\n".
				"<td class=t4>".$newBlogs[useretems][$i][pc][THEM]."&nbsp;</td>\n".
				"<td class=t3>".$newBlogs[useretems][$i][pc][VISIT]."</td>\n".
				"<td class=t4>".$newBlogs[useretems][$i][pc][NODES]."</td>\n".
				"<td class=t8><a href='pccon.php?id=".$newBlogs[useretems][$i][pc][UID]."&tid=".$newBlogs[useretems][$i][tid]."&nid=".$newBlogs[useretems][$i][nid]."&s=all'>".$newBlogs[useretems][$i][title]."</a>&nbsp;</td>\n".
				"<td class=t4>".$newBlogs[useretems][$i][created]."</td>\n</tr>\n";
	?>
</table>
<?php display_page_tools($pno,$newBlogsNum); ?>
<hr size=1>
<?php display_navigation_bar(); ?>
<p align=center class=f1>
<a href="rssnew.php" target="_blank"><img src="images/xml.gif" border="0" align="absmiddle" alt="XML"></a>
</p>
</center>
<?php	
	html_normal_quit();
?>