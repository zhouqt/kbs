<?php
	/*
	** @id:windinsn dec 21,2003
	*/
	require("pcfuncs.php");
	
	if(pc_update_cache_header())
		return;
	
	function display_section_blogs($sec)
	{
		global $pcconfig;
		if(!$pcconfig["SECTION"][$sec])
		{
			echo "对不起，目前尚无该分类。[<a href=\"javascript:history.go(-1);\">返回</a>]";
			return;	
		}
		echo "下面是<font class=f4>".html_format($pcconfig["SECTION"][$sec])."</font>分类的Blog：<br /><hr size=1>";
		
		@$pno = (int)($_GET["pno"]);
		if($pno < 1 ) $pno = 1;
		
		$link = pc_db_connect();
		if($sec == "others")
			$query = "SELECT username , corpusname , description , createtime  FROM users WHERE theme LIKE 'others%' OR theme = '' ORDER BY modifytime DESC LIMIT ".($pcconfig["THEMLIST"]*($pno - 1)).",".$pcconfig["THEMLIST"].";";
		else
			$query = "SELECT username , corpusname , description , createtime  FROM users WHERE theme LIKE '".addslashes($sec)."%' ORDER BY modifytime DESC LIMIT ".($pcconfig["THEMLIST"]*($pno - 1)).",".$pcconfig["THEMLIST"].";";
		$result = mysql_query($query,$link);
		$numRows = mysql_num_rows($result);
?><ul style="margin-left: 20px;"><?php		
		for($i = 0 ; $i < $numRows ; $i ++)
		{
			$rows = mysql_fetch_array($result);
			echo "<li><strong><a href=\"index.php?id=".$rows["username"]."\">".html_format($rows["corpusname"])."</a></strong>[".time_format_date1($rows["createtime"])."]：".html_format($rows["description"],TRUE)."</li>";
		}
?></ul>
<p align=center class=f1>
<?php
		if($pno > 1) echo "<a href=\"pcsec.php?sec=".$sec."&pno=".($pno - 1)."\">上一页</a>\n";
		if($numRows == $pcconfig["THEMLIST"]) echo "<a href=\"pcsec.php?sec=".$sec."&pno=".($pno + 1)."\">下一页</a>\n";
		pc_db_close($link);
?></p><?php
	}
	
	function display_section_list()
	{
		global $pcconfig;	
?>
<center>
<table cellspacing=0 cellpadding=5 border=0 class=t1>
<tr>
	<td class=t2>分类名</td>
	<td class=t2>分类主题</td>
</tr>
<?php
		$keys = array_keys($pcconfig["SECTION"]);
		for($i = 0 ; $i < count($keys) ; $i ++)
		{
			echo "<tr><td class=t4><a href=\"".$_SERVER["PHP_SELF"]."?sec=".$keys[$i]."\">".html_format($pcconfig["SECTION"][$keys[$i]])."</a></td>".
				"<td class=t8>".$keys[$i]."</td></tr>";
		}
?>
</table></center>
<?php		
	}
	
	pc_html_init("gb2312",$pcconfig["BBSNAME"]."Blog");
	
	
?>
<br /><a name="top">
<p align=center><strong>
<?php echo $pcconfig["BBSNAME"]; ?>Blog目录
</strong></p>
<?php
	display_blog_catalog();
?>
<font class=content>
<?php
	if(@$_GET["sec"]) display_section_blogs(@$_GET["sec"]);
	if(@$_GET["act"]=="list") display_section_list();
?>
</font>
<hr size=1>
<p align=center class=f1>
<?php pc_main_navigation_bar(); ?>
</p>
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
