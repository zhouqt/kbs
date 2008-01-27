<?php
	/*
	** article search in personal corp.
	** @id:windinsn nov 30,2003
	*/
	require("pcfuncs.php");
	
	function pc_search_display_pagetool($tpage,$cpage,$keyword,$area)
	{
		echo "<p align=center class=f5>\n[ ";
		for($i=0;$i<$tpage;$i++)
		{
			if($i + 1 == $cpage)
				echo $cpage." ";
			else
				echo 	"<a href=\"pcnsearch.php?keyword=".urlencode($keyword)."&area=".$area."&pno=".($i+1)."\">".($i+1)."</a> ";
		}
			echo "]\n</p>";
	}
	
	function pc_search_special_keyword($str,$keyword)
	{
		foreach($keyword as $key)
			$str = str_replace($key,"<font color=\"#FF0000\">".$key."</font>",$str);	
		return $str;
	}
	
	function pc_search_display_result($rows,$keyword)
	{
		global $pcconfig;
		if($rows[htmltag]) $rows[body] = undo_html_format(strip_tags($rows[body]));
		$bodylen = strlen($rows[body]);
		if($bodylen < 500)
			$body = $rows[body];
		else
		{
			$keypos = array();
			foreach($keyword as $key)
				$keypos[$key]=  strpos($rows[body],$key) - 100;
			$start = min($keypos);
			if($start > 0)
				$body = "... ".substr($rows[body],$start,500)." ...";
			else
				$body = substr($rows[body],0,500)." ...";
		}
		
		@$body = pc_search_special_keyword(html_format($body),$keyword);
?>
<table cellspacing="0" cellpadding="3" border="0" class="f1" width="500">
<tr><td>
<font class="f5">
<a href="pccon.php?<?php echo "id=".$rows[uid]."&nid=".$rows[nid]."&tid=".$rows[tid]; ?>">
<?php echo pc_search_special_keyword(html_format($rows[subject]),$keyword); ?>&nbsp;
</a>
</font>
</td></tr>
<tr><td class="f6">
<?php echo $body; ?>
</td></tr>
<tr><td>
<font color="#336600">
http://<?php echo $pcconfig["SITE"]; ?>/pc/pccon.php?<?php echo "id=".$rows[uid]."&nid=".$rows[nid]."&tid=".$rows[tid]; ?>
&nbsp;-<?php echo (int)(strlen($rows[body].$rows[subject])/100)/10; ?>K-
<?php echo time_format($rows[created]); ?>-
</font>
</td></tr>
</table><br><br>	
<?php		
	}
	
	pc_html_init("gb2312","Blog文章搜索");
?>
<br><br>
<p align="center" class="f2">
Blog日志搜索
</p>
<?php

	$userid = addslashes($_GET["userid"]);
	@$keyword = myAddslashes(trim($_GET["keyword"]));
	@$area = addslashes($_GET["area"]);
	@$pno = (int)($_GET["pno"]);
	
	if($keyword)
	{
		$link = pc_db_connect();
		
		$query_i = "SELECT COUNT(*) FROM nodes WHERE `access` = 0 ";
		$query = "SELECT `nid`,`tid`,`uid`,`subject`,`body`,`created`,`htmltag` FROM nodes WHERE `access` = 0 ";
		if($area != "_all")
		{
			$query0 = "SELECT `uid` FROM users WHERE `username` = '".$area."' LIMIT 0 , 1 ;";
			$result0 = mysql_query($query0,$link);
			if($rows0=mysql_fetch_array($result0))
			{
				$query .= " AND `uid` = '".$rows0[uid]."' ";
				$query_i.= " AND `uid` = '".$rows0[uid]."' ";
			}
			mysql_free_result($result0);
		}
		$keys = explode(" ",$keyword);
		$keyword = "";
		$keyword1 = array();
		$j = 0;
		foreach($keys as $key)
		{
			if($key =="" || $key == " ")
				continue;
			if(stristr($pcconfig["SEARCHFILTER"],$key))
				continue;
			$keyword1[$j] = $key;
			$j ++;
			$keyword .= $key." ";
			$query .= " AND ( `body` LIKE '%".$key."%' OR `subject` LIKE '%".$key."%' ) ";
			$query_i.=  " AND ( `body` LIKE '%".$key."%' OR `subject` LIKE '%".$key."%' ) ";
		}
		if(!$keyword)
		{
?>
<hr size="1">
<p align="left" class="f5"><strong>
对不起，您输入的关键字经过滤后为空，请重新输入。
</strong></p>
<?php
		}
		else
		{
			$result_i = mysql_query($query_i,$link);
			$rows_i = mysql_fetch_row($result_i);
			mysql_free_result($result_i);
			$totalno = $rows_i[0];
			$totalpage = (int)(($totalno - 1)/$pcconfig["SEARCHNUMBER"])+1;
			if($pno < 1 || $pno > $totalpage)
				$pno = 1;
			$searchstartno = ($pno - 1)*$pcconfig["SEARCHNUMBER"];
			$query .= " ORDER BY `nid` DESC LIMIT ".$searchstartno." , ".$pcconfig["SEARCHNUMBER"].";";
			$result = mysql_query($query,$link);
			$num_rows = mysql_num_rows($result);
			
			
?>	
<hr size="1">
<p align="left" class="f5"><strong>
您输入的关键字是 <font class="f4"><?php echo $keyword; ?></font> ，系统共为您找到 <font class="f4"><?php echo $totalno; ?></font> 条记录，
这是 <font class="f4"><?php echo $searchstartno + 1; ?> - <?php echo $searchstartno +  $num_rows; ?></font> 条记录：
</strong></p>		
<?php			
			for($i = 0;$i < $num_rows;$i++)
				pc_search_display_result(mysql_fetch_array($result),$keyword1);
			pc_search_display_pagetool($totalpage,$pno,$keyword,$area);
?>
<br>
<hr size="1">
<br>
<?php		
			mysql_free_result($result);
		}
	}
	
	
?>
<form action="pcnsearch.php" method="get" onsubmit="if(this.keyword.value==''){alert('请输入关键字');return false;}">
<p align="center" class="f1">
请输入关键字：
<input type="text" name="keyword" size="60" id="keyword" class="f1" value="<?php echo $keyword; ?>">
<?php
	if($userid || ( $area && $area != "_all"))
	{
?>
</p>
<p align="center" class="f1">
搜索范围：
<input type="radio" name="area" value="<?php echo $userid; ?>" checked>
<?php echo $userid?$userid:$area; ?>的Blog
<input type="radio" name="area" value="_all">
全站
</p>
<p align="center" class="f1">
<?php
	}
?>
<input type="submit" value="开始搜" class="b1">
</p>
<p align="center" class="f1">
(请用空格隔开多个关键字)
</p>
</form>
<p align="center">
<?php pc_main_navigation_bar(); ?>
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
