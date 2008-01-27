<?php
	/*
	** @id:windinsin nov 29,2003
	*/
	require("pcstat.php");

	function display_navigation_bar($type)
	{
?>
<p align=center class=f1>
<?php pc_main_navigation_bar(); ?>
</p>
<?php
	}

	function display_page_tools($pno,$etemno,$type)
	{
		global $pcconfig;
?>
<p align=center class=f1>
<?php
		$loadtype = ($type=="n")?"":"&t=c";
		if($pno > 1)
			echo "[<a href='pcnew.php?pno=".($pno - 1).$loadtype."'>前".$pcconfig["NEWS"]."条记录</a>]\n";	
		else
			echo "[前".$pcconfig["NEWS"]."条记录]\n";
		if($etemno == $pcconfig["NEWS"])
			echo "[<a href='pcnew.php?pno=".($pno + 1).$loadtype."'>后".$pcconfig["NEWS"]."条记录</a>]\n";	
		else
			echo "[后".$pcconfig["NEWS"]."条记录]\n";
?>
</p>
<?php
	}
	
	//20min更新一次
	if(pc_update_cache_header())
		return;
	
	@$pno = (int)($_GET["pno"]);
	$pno = ($pno < 1)?1:$pno;
	$type = (@$_GET["t"]=="c")?"c":"n";
	$link = pc_db_connect();
	if($type=="n")
	{
		$newBlogs = getNewBlogs($link,$pno);
		$newNum = count($newBlogs["useretems"]);
	}
	else
	{
		$newComments = getNewComments($link,$pno);
		$newNum = count($newComments);	
	}
	pc_db_close($link);
	
	
	pc_html_init("gb2312",$pcconfig["BBSNAME"]."Blog");
?>
<br><br>
<p align=center class=f2>
欢迎使用<?php echo $pcconfig["BBSNAME"]; ?>Blog系统
</p>
<?php display_navigation_bar($type); ?>
<hr size=1>
<center>
<p align=center class=f1><strong>
最近更新的<?php echo $pcconfig["NEWS"]; ?>篇<?php echo ($type=="n")?"文章":"评论"; ?>列表
</strong></p>
<?php display_page_tools($pno,$newNum,$type); ?>
<table cellspacing=0 cellpadding=5 width=98% border=0 class=t1>
<?php
	if($type == "n")
	{
?>
	<tbody><tr>
		<td class=t2 width=70>用户名</td>
		<td class=t2 width=120>Blog名称</td>
		<td class=t2 width=70>Blog主题</td>
		<td class=t2 width=40>访问量</td>
		<td class=t2 width=40>文章数</td>
		<td class=t2>文章主题</td>
		<td class=t2 width=120>更新时间</td>
	</tr></tbody>
<?php
		for($i=0;$i < $newNum;$i++)
		{
			echo "<tbody><tr>\n<td class=t4><a href='/bbsqry.php?userid=".$newBlogs["useretems"][$i]["creator"]."'>".$newBlogs["useretems"][$i]["creator"]."</a></td>\n".
				"<td class=t3><span title=\"".html_format($newBlogs["useretems"][$i]["pcdesc"])."\"><a href=\"index.php?id=".$newBlogs["useretems"][$i]["creator"]."\">".html_format($newBlogs["useretems"][$i]["pcname"])."</a>&nbsp;</span></td>\n".
				"<td class=t4><span title=\"点击查看该分类的其它Blog信息\"><a href=\"pcsec.php?sec=".html_format($newBlogs["useretems"][$i]["theme"][0])."\">".
				$pcconfig["SECTION"][$newBlogs["useretems"][$i]["theme"][0]]."</a></span></td>\n".
				"<td class=t3>".$newBlogs["useretems"][$i]["pcvisit"]."</td>\n".
				"<td class=t4>".$newBlogs["useretems"][$i]["pcnodes"]."</td>\n".
				"<td class=t8><a href='pccon.php?id=".$newBlogs["useretems"][$i]["pc"]."&nid=".$newBlogs["useretems"][$i]["nid"]."&s=all&tid=".$newBlogs["useretems"][$i]["tid"]."'>".$newBlogs["useretems"][$i]["subject"]."</a>&nbsp;</td>\n".
				"<td class=t4>".$newBlogs["useretems"][$i]["created"]."</td>\n</tr></tbody>\n";
		}
	}
	else
	{
?>
	<tbody><tr>
		<td class=t2 width=70>发布者</td>
		<td class=t2>评论主题</td>
		<td class=t2 width=125>评论发布时间</td>
		<td class=t2 width=200>原文主题</td>
		<td class=t2 width=20>访</td>
		<td class=t2 width=20>评</td>
	</tr></tbody>
<?php
		for($i=0;$i < $newNum;$i++)
		{
			echo "<tbody><tr>\n<td class=t4><a href='/bbsqry.php?userid=".$newComments[$i]["POSTER"]."'>".$newComments[$i]["POSTER"]."</a></td>\n".
				"<td class=t8><a href=\"pcshowcom.php?cid=".$newComments[$i]["CID"]."\">".$newComments[$i]["CSUBJECT"]."</a>&nbsp;</td>\n".
				"<td class=t4>".$newComments[$i]["CCREATED"]."&nbsp;</td>\n".
				"<td class=t8><span title=\"".$newComments[$i]["NSUBJECT"]."\"><a href=\"pccon.php?id=".$newComments[$i]["UID"]."&nid=".$newComments[$i]["NID"]."&s=all\">".substr($newComments[$i]["NSUBJECT"],0,30)." \n ";
			if(strlen($newComments[$i]["NSUBJECT"]) > 30) echo " ...";
				echo "</a>&nbsp;</span></td>\n".
				"<td class=t3>".$newComments[$i]["VISITCOUNT"]."</td>\n".
				"<td class=t4>".$newComments[$i]["COMMENTCOUNT"]."</td>\n</tr></tbody>\n";
		}
	}	
?>
</table>
<?php display_page_tools($pno,$newNum,$type); ?>
<hr size=1>
<?php 
	display_navigation_bar($type); 
	if($type=="n")
	{
?>
<p align=center class=f1>
<a href="rssnew.php" target="_blank"><img src="images/xml.gif" border="0" align="absmiddle" alt="XML"></a>
</p>
<?php
	}
?>
</center>
<p align="center">
<?php
    /**
     *    水木的web代码bbslib和cgi部分是修改于NJUWWWBBS-0.9，此部分
     * 代码遵循原有的nju www bbs的版权声明（GPL）。php部分的代码（
     * phplib以及php页面）不再遵循GPL，正在考虑使用其他开放源码的版
     * 权声明（BSD或者MPL之类）。
     *
     *   希望使用水木代码的Web站点加上powered by kbs的图标.该图标
     * 位于html/images/poweredby.gif目录,链接指向http://dev.kcn.cn
     * 使用水木代码的站点可以通过dev.kcn.cn获得代码的最新信息.
     *
     */
    powered_by_smth();
?>
</p>
<?php	
	html_normal_quit();
?>
