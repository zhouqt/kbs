<?php
	
	/*
	** personal corp.
	** @id:windinsn Nov 19,2003
	*/
	$needlogin=0;
	require("pcfuncs.php");
	
	function display_user_list($link,$listorder="username",$listorder1="ASC",$char=FALSE,$startno=0,$pagesize=10)
	{
		if($char)
			$query = "SELECT  `uid` , `username` , `corpusname` , `description` , `theme` , `createtime`,`modifytime`,`nodescount`,`visitcount` ".
				" FROM users WHERE `username` LIKE '".$char."%' ORDER BY ".$listorder." ".$listorder1." LIMIT ".$startno." , ".$pagesize.";";
		else
			$query = "SELECT  `uid` , `username` , `corpusname` , `description` , `theme` , `createtime`,`modifytime`,`nodescount`,`visitcount` ".
				" FROM users ORDER BY ".$listorder." ".$listorder1." LIMIT ".$startno." , ".$pagesize.";";
		$result = mysql_query($query,$link);
		$list_user_num = mysql_num_rows($result);
?>
<table border="0" cellspacing="0" cellpadding="5" width="99%" class="t1">
<tr>
	<td class="t2" width="30">编号</td>
	<td class="t2" width="70">
		用户名
		<a href="pc.php?order=username&order1=ASC&char=<?php echo $char; ?>"><img src="images/desc_order.png" border="0" align="absmiddle" alt="按用户名递增排序"></a>
		<a href="pc.php?order=username&order1=DESC&char=<?php echo $char; ?>"><img src="images/asc_order.png" border="0" align="absmiddle" alt="按用户名递减排序"></a>
	</td>
	<td class="t2" width="130">文集名称</td>
	<!--
	<td class="t2">描述</td>
	-->
	<td class="t2">主题</td>
	<td class="t2" width="70">
		文章数
		<a href="pc.php?order=nodescount&order1=ASC&char=<?php echo $char; ?>"><img src="images/desc_order.png" border="0" align="absmiddle" alt="按文章数递增排序"></a>
		<a href="pc.php?order=nodescount&order1=DESC&char=<?php echo $char; ?>"><img src="images/asc_order.png" border="0" align="absmiddle" alt="按文章数递减排序"></a>
	</td>
	<td class="t2" width="70">访问量
		<a href="pc.php?order=visitcount&order1=ASC&char=<?php echo $char; ?>"><img src="images/desc_order.png" border="0" align="absmiddle" alt="按访问量递增排序"></a>
		<a href="pc.php?order=visitcount&order1=DESC&char=<?php echo $char; ?>"><img src="images/asc_order.png" border="0" align="absmiddle" alt="按访问量递减排序"></a>
	</td>
	<td class="t2" width="120">
		创建时间
		<a href="pc.php?order=createtime&order1=ASC&char=<?php echo $char; ?>"><img src="images/desc_order.png" border="0" align="absmiddle" alt="按创建时间递增排序"></a>
		<a href="pc.php?order=createtime&order1=DESC&char=<?php echo $char; ?>"><img src="images/asc_order.png" border="0" align="absmiddle" alt="按创建时间递减排序"></a>
	<td class="t2" width="120">
		更新时间
		<a href="pc.php?order=modifytime&order1=ASC&char=<?php echo $char; ?>"><img src="images/desc_order.png" border="0" align="absmiddle" alt="按更新时间递增排序"></a>
		<a href="pc.php?order=modifytime&order1=DESC&char=<?php echo $char; ?>"><img src="images/asc_order.png" border="0" align="absmiddle" alt="按更新时间递减排序"></a>
	</td>
</tr>
<?php
		for($i=0;$i < $list_user_num;$i++)
		{
			$rows = mysql_fetch_array($result);
			$themekey = urlencode(stripslashes($rows[theme]));
			echo "<tr>\n<td class=\"t3\">".($startno + $i + 1)."</td>\n".
				"<td class=\"t4\"><a href=\"/bbsqry.php?userid=".html_format($rows[username])."\">".html_format($rows[username])."</a></td>\n".
				"<td class=\"t3\"><span title=\"".html_format($rows[description])."\"><a href=\"index.php?id=".$rows[username]."\">".html_format($rows[corpusname])."</a>&nbsp;</span></td>\n".
				//"<td class=\"t5\"><a href=\"pcdoc.php?userid=".$rows[username]."\">".html_format($rows[description])."</a>&nbsp;</td>\n".
				"<td class=\"t3\"><a href=\"pcsearch.php?exact=0&key=t&keyword=".$themekey."\">".html_format($rows[theme])."</a>&nbsp;</td>\n".
				"<td class=\"t4\">".$rows[nodescount]."</a>".
				"<td class=\"t3\">".$rows[visitcount]."</a>".
				"<td class=\"t4\">".time_format($rows[createtime])."</a>".
				"<td class=\"t3\">".time_format($rows[modifytime])."</td>\n</tr>\n";
		}
?>
</table>	
<?php		
		mysql_free_result($result);	
	}
	
	function display_navigation_bar($total,$page,$char,$order="username",$order1="ASC")
	{
		$listno = 7;
		$pre = min($listno,$page-1);
		$next = min($listno,$total-$page);
		if($char)
			$url = "pc.php?order=".$order."&order1=".$order1."&char=".$char;
		else
			$url = "pc.php?order=".$order."&order1=".$order1;
		echo "<p align=\"center\">\n[ ";
		if($pre < $page - 1)
			echo "<a href=\"".$url."&pno=".($page - $pre - 1)."\">...</a> ";
		for($i = $pre ;$i > 0;$i--)
			echo "<a href=\"".$url."&pno=".($page - $i)."\">".($page - $i)."</a> ";
		echo $page." ";
		for($i = 0;$i < $next ;$i++)
			echo "<a href=\"".$url."&pno=".($page + $i + 1)."\">".($page + $i + 1)."</a> ";
		if($next < $total - $page)
			echo "<a href=\"".$url."&pno=".($page + $next + 1)."\">...</a> ";
		echo "]\n</p>";
	}
	
	function display_char_bar($char=FALSE)
	{
		$all = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";	
		echo "<p align=\"center\">\n[ ";
		for($i=0;$i < 26;$i++)
		{
			if($char == $all[$i])
				echo $char." ";
			else
				echo "<a href=\"pc.php?char=".$all[$i]."\">".$all[$i]."</a> ";
		}
		if($char)
			echo "<a href=\"pc.php\">一般模式</a> ";
		echo "]\n</p>";
	}
	
	
	
	$listorder = $_GET["order"];
	$listorder1 = $_GET["order1"];
	$pageno = (int)($_GET["pno"]);
	if(isset($_GET["char"]))
		$char = addslashes($_GET["char"]);
	else
		$char = FALSE;
		
	switch($_GET["order"])
	{
		case "createtime":
			$listorder = "createtime";
			break;
		case "modifytime":
			$listorder = "modifytime";
			break;
		case "nodescount":
			$listorder = "nodescount";
			break;
		case "visitcount":
			$listorder = "visitcount";
			break;
		case "username":
			$listorder = "username";
			break;
		default:
			$listorder = "modifytime";	
	}
	
	if($listorder1 != "ASC")
		$listorder1 = "DESC";
		
	$link = pc_db_connect();
	if($char)
		$query = "SELECT COUNT(*) FROM users WHERE `username` LIKE '".$char."%';";
	else
		$query = "SELECT COUNT(*) FROM users;";
	$result = mysql_query($query);
	$rows = mysql_fetch_row($result);
	mysql_free_result($result);
	$total = $rows[0];
	$pagesize = $pcconfig["LIST"];
	$totalpage = (($total - 1) / $pagesize) + 1;
	$totalpage = (int)($totalpage);
	if( $pageno < 1 || $pageno > $totalpage )
		$pageno = 1;
	$startno = ($pageno - 1)*$pagesize;
	pc_html_init("gb2312","个人文集");
?>
<center><br><br>
<p align="center" class="f2">
<?php echo BBS_FULL_NAME; ?>个人文集
</p>
<hr size=1>
<p class="f1">
[<a href="pcsearch.php?keyword=<?php echo $currentuser["userid"]; ?>&exact=1&key=u">自己的文集</a>]
[<a href="pcnew.php">最新文章列表</a>]
<a href="rssnew.php" target="_blank"><img src="images/xml.gif" border="0" align="absmiddle" alt="XML"></a>
</p>
<?php
	display_user_list($link,$listorder,$listorder1,$char,$startno,$pagesize);
	display_navigation_bar($totalpage,$pageno,$char,$listorder,$listorder1);
	display_char_bar($char);
?>
<hr size=1>
<p class="f1">
[<a href="pcsearch.php?keyword=<?php echo $currentuser["userid"]; ?>&exact=1&key=u">自己的文集</a>]
[<a href="pcnew.php">最新文章列表</a>]
<a href="rssnew.php" target="_blank"><img src="images/xml.gif" border="0" align="absmiddle" alt="XML"></a>
</p>
<form action="pcsearch.php" method="get" onsubmit="if(this.keyword.value==''){alert('请输入关键字');return false;}">
个人文集搜索:<br>
<input type="text" name="keyword" size="20" class="b2">
(进行模糊搜索时,请用空格隔开多个关键字)
<br>
方式:
<input type="radio" name="exact" value="1" class="b2" checked>精确
<input type="radio" name="exact" value="0" class="b2">模糊<br>
类型:
<input type="radio" name="key" value="u" class="b2" checked>用户名
<input type="radio" name="key" value="c" class="b2">文集名
<input type="radio" name="key" value="t" class="b2">主题
<input type="radio" name="key" value="d" class="b2">文集描述<br>
<input type="submit" value="开始搜" class="b1">
</form>
</center>
<?php
	html_normal_quit();
	pc_db_close($link);
?>