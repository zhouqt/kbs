<?php
	/*
	** look for a personal corp.
	** @id:windinsn Nov 19,2003
	*/
	$needlogin=0;
	require("pcfuncs.php");
	
	$keyword = trim($_GET["keyword"]);
	switch($_GET["key"])
	{
		case "c":
			$key = "corpusname";
			$keyname = "个人文集名称";
			break;
		case "t":
			$key = "theme";
			$keyname = "个人文集主题";
			break;
		case "d":
			$key = "description";
			$keyname = "个人文集描述";
			break;
		default:
			$key = "username";
			$keyname = "个人文集所有者";
	}
	
	$query = "SELECT `uid` , `username` , `corpusname` , `description` , `theme` , `createtime` ".
		" FROM users WHERE ";
		
	if($_GET["exact"]==0)
	{
		$keyword = explode(" ",$keyword);
		$query .= " `".$key."` != '' ";
		$keyword1 = "";
		for($i=0;$i < count($keyword) ; $i++)
		{
			if($keyword[$i] == " " || $keyword[$i] == "")
				continue;
			else
			{
				$query .= " AND `".$key."` LIKE '%".addslashes($keyword[$i])."%' ";
				$keyword1 .= " ".$keyword[$i];
			}	
		}
	}
	else
	{
		$query.= " `".$key."` = '".addslashes($keyword)."'  ";
		$keyword1 = $keyword;
	}
	
	$query .= " ORDER BY `username` ; ";
	$link = pc_db_connect();
	$result = mysql_query($query,$link);
	$num_rows = mysql_num_rows($result);
	
	html_init("gb2312","个人文集搜索","",1);
	if($num_rows == 0)
	{
		mysql_free_result($result);
		pc_db_close($link);
		html_error_quit("对不起，没有符合条件的个人文集，请尝试缩减关键字重新查询");
	}
	else
	{
		echo "按照 ".$keyname." 查询，关键字为 ".$keyword1." 。<br>".
			"系统共为您查到 ".$num_rows." 笔记录：";
?>
<table border="1">
<tr>
	<td>编号</td>
	<td>用户名</td>
	<td>文集名称</td>
	<td>描述</td>
	<td>主题</td>
	<td>创建时间</td>
</tr>
<?php
		for($i=0 ; $i < $num_rows ; $i++)
		{
			$rows = mysql_fetch_array($result);
			$t = $rows[createtime];
			$t= $t[0].$t[1].$t[2].$t[3]."-".$t[4].$t[5]."-".$t[6].$t[7]." ".$t[8].$t[9].":".$t[10].$t[11].":".$t[12].$t[13];
			$themekey = urlencode(stripslashes($rows[theme]));
			echo "<tr>\n<td>".($startno + $i + 1)."</td>\n".
				"<td><a href=\"/bbsqry.php?userid=".html_format($rows[username])."\">".html_format($rows[username])."</a></td>\n".
				"<td><a href=\"pcdoc.php?userid=".$rows[userid]."\">".html_format($rows[corpusname])."</a></td>\n".
				"<td><a href=\"pcdoc.php?userid=".$rows[userid]."\">".html_format($rows[description])."</a></td>\n".
				"<td><a href=\"pcsearch.php?exact=0&key=t&keyword=".$themekey."\">".html_format($rows[theme])."</a></td>\n".
				"<td>".$t."</td>\n</tr>\n";
		}
?>
</table>
<p align="center">
<a href="pc.php">返回个人文集首页</a>
</p>		
<?php
		mysql_free_result($result);
		pc_db_close($link);
	}
	
	html_normal_quit();	
?>