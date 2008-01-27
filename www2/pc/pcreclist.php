<?php
/*
** @id:windinsn Mar 5, 2004
*/
require("pcfuncs.php");
//20min更新一次
if(pc_update_cache_header())
	return;

$pno = $_GET["pno"];
$pno = intval( $pno );
if( $pno < 1) $pno = 1;
$start = ($pno - 1)*$pcconfig["LIST"];

pc_html_init("gb2312",$pcconfig["BBSNAME"]."Blog");
$pcuser = array();
$link = pc_db_connect();
$query = "SELECT * FROM recommend ";
if ($_GET['topic'] && $pcconfig["SECTION"][$_GET['topic']])
    $query .= " WHERE topic = '".addslashes($_GET['topic'])."' ";
$query .= " ORDER BY state DESC , rid DESC LIMIT ".$start." , ".$pcconfig["LIST"].";";
$result = mysql_query($query,$link);
$num = mysql_num_rows($result);
?>
<br/>
<p align="center">
<strong><?php echo $pcconfig["BBSNAME"]; ?>Blog推荐文章 </strong>
<p>
<p align="center"><?php pc_main_navigation_bar(); ?></p>
<hr size=1>
<center>
<table cellpadding=5 cellspacing=0 class=t1 width=98% border=0>
	<tbody><tr>
		<td class=t2 width=20> </td>
		<td class=t2>文章主题</td>
		<td class=t2 width=160>Blog名称</td>
		<td class=t2 width=80>作者</td>
		<td class=t2 width=80>类别</td>
		<td class=t2 width=80>推荐人</td>
	</tr></tbody>
<?php
	for( $i = 0;$i<$num;$i ++)
	{
		$rows = mysql_fetch_array($result);
		if( !$pcuser[$rows[uid]] )
			$pcuser[$rows[uid]] = pc_load_infor($link,"",$rows[uid]);
		echo "<tbody><tr>\n".
			"<td class=t3>".($i + $start + 1)."</td>".
			"<td class=t5><a href=\"pccon.php?id=".$rows[uid]."&nid=".$rows[nid]."&s=all\">".html_format($rows[subject])."</a></td>".
			"<td class=t3><span title=\"".$pcuser[$rows[uid]]["DESC"]."\"><a href=\"index.php?id=".$pcuser[$rows[uid]]["USER"]."\">".$pcuser[$rows[uid]]["NAME"]."</a></span></td>".
			"<td class=t4><a href=\"/bbsqry.php?userid=".$pcuser[$rows[uid]]["USER"]."\">".$pcuser[$rows[uid]]["USER"]."</a></td>".
			"<td class=t3><span title=\"点击查看该分类的其它Blog信息\"><a href=\"pcreclist.php?topic=".$rows[topic]."\">".html_format($pcconfig["SECTION"][$rows[topic]])."</a></span></td>".
			"<td class=t4><a href=\"/bbsqry.php?userid=".$rows[recuser]."\">".$rows[recuser]."</a></td>".
			"</tr></tbody>\n";
	}
?>
</table>
<p align="center">
<?php
	if( $pno > 1 )
		echo "[<a href=\"pcreclist.php?pno=".($pno-1)."\">上一页</a>]\n";
	if( $num == $pcconfig["LIST"] )
		echo "[<a href=\"pcreclist.php?pno=".($pno+1)."\">下一页</a>]\n";
?>
</p>
<hr size=1>
<p align="center"><?php pc_main_navigation_bar(); ?></p>
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
<p align="center">版权所有 &copy; <?php echo $pcconfig["BBSNAME"]; ?></p>
<?php
mysql_free_result($result);
pc_db_close($link);
html_normal_quit();
?>
