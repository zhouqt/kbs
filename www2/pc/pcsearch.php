<?php
	/*
	** look for a personal corp.
	** @id:windinsn Nov 19,2003
	*/
	require("pcfuncs.php");
	
	$keyword = addslashes(trim($_GET["keyword"]));
	switch($_GET["key"])
	{
		case "c":
			$key = "corpusname";
			$keyname = "Blog名称";
			break;
		case "t":
			$key = "theme";
			$keyname = "Blog主题";
			break;
		case "d":
			$key = "description";
			$keyname = "Blog描述";
			break;
		default:
			$key = "username";
			$keyname = "Blog所有者";
	}
	
	$query = "SELECT `uid` , `username` , `corpusname` , `description` , `theme` , `createtime`,`modifytime`,`nodescount`,`visitcount` ".
		" FROM users WHERE ";
		
	if($_GET["exact"]==0)
	{
		$keyword = explode(" ",$keyword);
		$query .= " `uid` = 0 ";
		$keyword1 = "";
		for($i=0;$i < count($keyword) ; $i++)
		{
			if($keyword[$i] == " " || $keyword[$i] == "")
				continue;
			else
			{
				$query .= " OR `".$key."` LIKE '%".myAddslashes($keyword[$i])."%' ";
				$keyword1 .= " ".$keyword[$i];
			}	
		}
	}
	else
	{
		$query.= " `".$key."` = '".addslashes($keyword)."'  ";
		$keyword1 = $keyword;
	}
	
	$query .= " ORDER BY `username`;";
	$link = pc_db_connect();
	$result = mysql_query($query,$link);
	$num_rows = mysql_num_rows($result);
	
	if($num_rows == 0)
	{
		mysql_free_result($result);
		pc_db_close($link);
		pc_html_init("gb2312","Blog搜索");
		html_error_quit("对不起，没有符合条件的Blog，请尝试缩减关键字重新查询");
	}
	elseif($num_rows==1)
	{
		$rows = mysql_fetch_array($result);
		header("Location: index.php?id=".$rows["username"]);
	}
	else
	{
		pc_html_init("gb2312","Blog搜索");
		echo "<br>按照 <font class=f2>".$keyname."</font> 查询，关键字为 <font class=f2>".$keyword1."</font> 。<br>".
			"系统共为您查到 <font class=f2>".$num_rows."</font> 笔记录：";
?>
<center><br><br><br>
<table cellspacing="0" cellpadding="3" width="95%" class="t1">
<tr>
	<td class="t2" width="30">编号</td>
	<td class="t2" width="70">用户名</td>
	<td class="t2">Blog名称</td>
	<td class="t2" width="50">类别</td>
	<td class="t2" width="50">文章数</td>
	<td class="t2" width="50">访问量</td>
	<td class="t2" width="120">创建时间</td>
	<td class="t2" width="120">更新时间</td>
</tr>
<?php
		for($i=0 ; $i < $num_rows ; $i++)
		{
			$rows = mysql_fetch_array($result);
			$pcThem = pc_get_theme($rows[theme]);
			echo "<tr>\n<td class=t3>".($startno + $i + 1)."</td>\n".
				"<td class=t4><a href=\"/bbsqry.php?userid=".html_format($rows[username])."\">".html_format($rows[username])."</a></td>\n".
				"<td class=t3><span title=\"".html_format($rows[description])."\"><a href=\"index.php?id=".$rows[username]."\">".html_format($rows[corpusname])."</a></span></td>\n".
				"<td class=t3><span title=\"点击查看该分类的其它Blog信息\"><a href=\"pcsec.php?sec=".html_format($pcThem[0])."\">".html_format($pcconfig["SECTION"][$pcThem[0]])."</a></span></td>\n".
				"<td class=\"t4\">".$rows[nodescount]."</a>".
				"<td class=\"t3\">".$rows[visitcount]."</a>".
				"<td class=\"t4\">".time_format($rows[createtime])."</a>".
				"<td class=\"t3\">".time_format($rows[modifytime])."</td>\n</tr>\n";
		}
?>
</table>
</center>
<p align="center">
<?php pc_main_navigation_bar(); ?>
</p>		

<?php
		mysql_free_result($result);
		pc_db_close($link);
	}
?>
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
