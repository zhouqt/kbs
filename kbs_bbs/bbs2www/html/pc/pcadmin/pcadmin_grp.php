<?php
require("pcadmin_inc.php");
function pc_load_special($link)
{
	$query = "SELECT uid , username , pctype , corpusname , description , visitcount , nodescount FROM users WHERE pctype != 0 ORDER BY username ASC ;";
	$result = mysql_query($query,$link);
	$pcs = array();
	while($rows = mysql_fetch_array($result))
		$pcs[] = $rows;
	return $pcs;
}

pc_admin_check_permission();
$link = pc_db_connect();
pc_html_init("gb2312",$pcconfig["BBSNAME"]."特殊BLOG管理");
pc_admin_navigation_bar();

if($_GET["userid"] && $_GET["conv"])
{
	$pcc = pc_load_infor($link,$_GET["userid"]);
	if(!$pcc)
	{
		html_error_quit($_GET["userid"]."尚无BLOG");
		exit();
	}
	
	if(!$_GET["statnew"])
		$newtype = 7;
	elseif(!$_GET["statnodes"])
		$newtype = 5;
	elseif(!$_GET["statusers"])
		$newtype = 3;
	else
		$newtype = 1;
		
	if(!$_GET["isgroup"] && !pc_is_groupwork($pcc))
		$newtype -- ;
	
	$query = "UPDATE users SET createtime = createtime , pctype = ".$newtype."  WHERE uid = ".$pcc["UID"]." LIMIT 1;";
	mysql_query($query,$link);
	
	if($_GET["isgroup"] && !pc_is_groupwork($pcc))
		pc_convertto_group($link,$pcc);
	$action = "调整 " . $pcc["USER"] . " 的BLOG参数(N:".$newtype.";O:".$pcc["TYPE"].")";
	pc_logs($link , $action , "" , $pcc["USER"] );
	
	unset($_GET["userid"]);
}

if($_GET["userid"])
{
	$pcc = pc_load_infor($link,$_GET["userid"]);
	if(!$pcc)
	{
		html_error_quit($_GET["userid"]."尚无BLOG");
		exit();
	}
?>	
<form action="<?php echo $_SERVER["PHP_SELF"]; ?>" method="get">
<input type="hidden" name="conv" value="1">
处理的用户名：<input type="text" name="userid" value="<?php echo $pcc["USER"]; ?>"><br />
是否为公有BLOG:
<input type="checkbox" name="isgroup" value="1" <?php if(pc_is_groupwork($pcc)) echo "checked"; ?> />
(<font color=red>注意:公有BLOG无法转化为普通BLOG</font>)
<br />
是否进行用户统计排行
<input type="checkbox" name="statusers" value="1" <?php if($pcc["TYPE"]< 2) echo "checked"; ?> /><br />
是否进行日志(含评论)统计排行
<input type="checkbox" name="statnodes" value="1" <?php if($pcc["TYPE"]< 4) echo "checked"; ?> /><br />
是否进行新日志(含评论,RSS)统计
<input type="checkbox" name="statnew" value="1" <?php if($pcc["TYPE"]< 6) echo "checked"; ?> /><br />
<input type="submit" value="变更" />
<input type="button" value="返回" onclick="history.go(-1)" />
</form>
<?php	
}
else
{
	$pcs = pc_load_special($link);
?>
<br /><br />
<p align="center"><b>特殊BLOG管理</b></p><center>
<b>注意：</b>
<font color=red>公</font>表示是否为公有BLOG
<font color=red>人</font>表示是否进行用户统计排行
<font color=red>文</font>表示是否进行日志(含评论)统计排行
<font color=red>新</font>表示是否进行新日志(含评论,RSS)统计<br /><br />
<table cellspacing="0" cellpadding="3" class="t1">
<tr>
<td class="t2">用户名</td>
<td class="t2">BLOG名</td>
<td class="t2">访问数</td>
<td class="t2">日志数</td>
<td class="t2">公</td>
<td class="t2">人</td>
<td class="t2">文</td>
<td class="t2">新</td>
<td class="t2">操作</td>
</tr>
<?php
	foreach($pcs as $pc)
	{
		echo "<tr><td class=t4><a href=\"/bbsqry.php?userid=".$pc[username]."\">".$pc[username]."</a></td>".
		     "<td class=t5><a href=\"index.php?id=".$pc[username]."\" title=\"".html_format($pc[description])."\">".html_format($pc[corpusname])."</a></td>".
		     "<td class=t4>".$pc[visitcount]."</td>".
		     "<td class=t3>".$pc[nodescount]."</td><td class=t4>";
		echo (pc_is_groupwork(array("TYPE"=>$pc[pctype])))?"<font color=red><b>Y</b></font>":"n";
		echo "</td><td class=t4>";
		echo ($pc[pctype]<2)?"y":"<font color=red><b>N</b></font>";
		echo "</td><td class=t4>";
		echo ($pc[pctype]<4)?"y":"<font color=red><b>N</b></font>";
		echo "</td><td class=t4>";
		echo ($pc[pctype]<6)?"y":"<font color=red><b>N</b></font>";
		echo "</td><td class=t3><a href=\"".$_SERVER["PHP_SELF"]."?userid=".$pc[username]."\">操作</a></td></tr>";
	}
?>
</table>
<br />
<form action="<?php echo $_SERVER["PHP_SELF"]; ?>" method="get">
处理BLOG
<input type="text" name="userid">
<input type="submit" value="处理">
</form></center>
<?php	
}
pc_db_close($link);
pc_admin_navigation_bar();
html_normal_quit();
?>