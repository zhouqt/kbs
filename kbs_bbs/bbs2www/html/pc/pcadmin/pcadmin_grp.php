<?php
require("pcadmin_inc.php");
function pc_load_groupworks($link)
{
	$query = "SELECT uid , username FROM users WHERE pctype = 1;";
	$result = mysql_query($query,$link);
	$pcs = array();
	while($rows = mysql_fetch_array($result))
		$pcs[] = $rows;
	return $pcs;
}

pc_admin_check_permission();
$link = pc_db_connect();

if($_GET["act"] == "convert2" && $_GET["userid"])
{
	$pcc = pc_load_infor($link,$_GET["userid"]);
	if(!$pcc)
	{
		html_error_quit($_GET["userid"]."尚无BLOG");
		exit();
	}
	$ret = pc_convertto_group($link,$pcc);
	switch($ret)
	{
		case -1:
			html_error_quit($_GET["userid"]."的BLOG参数错误");
			exit();
		case -2:
			html_error_quit($_GET["userid"]."的BLOG已是公有BLOG");
			exit();
		case -3:
			html_error_quit($_GET["userid"]."公有BLOG的LOG目录初始化错误");
			exit();
		case -4:
			html_error_quit("系统错误");
			exit();
		case -5:
			html_error_quit("LOG错误");
			exit();
		default:	
	}
}

$pcs = pc_load_groupworks($link);

pc_html_init("gb2312",$pcconfig["BBSNAME"]."公有BLOG管理");
pc_admin_navigation_bar();

if($_GET["act"] == "convert" && $_GET["userid"])
{
?>
<form action="<?php echo $_SERVER["PHP_SELF"]; ?>" method="get">
确认要将<?php echo $_GET["userid"]; ?>的BLOG转化为公有BLOG?
<br />
一旦转化成功，将不能转化成普通BLOG
<input type="hidden" name="userid" value="<?php echo $_GET["userid"]; ?>">
<input type="hidden" name="act" value="convert2">
<input type="submit" value="确认转化">
<input type="button" value="取消" onclick="window.location.href='pcadmin_grp.php'">
</form>
<?php	
}
else
{
?>
<br /><br />
<p align="center"><b>公有BLOG管理</b></p><center>
<table cellspacing="0" cellpadding="3" class="t1">
<tr>
<td class="t2">BLOG</td>
<td class="t2">撤销为普通BLOG</td>
</tr>
<?php
	foreach($pcs as $pc)
		echo "<tr><td class=t3><a href=\"index.php?id=".$pc[username]."\">".$pc[username]."</a></td><td class=t4>-</td></tr>";
?>
</table>
<br />
<form action="<?php echo $_SERVER["PHP_SELF"]; ?>" method="get">
转化一个BLOG为公有BLOG(不可逆)
<input type="text" name="userid">
<input type="hidden" name="act" value="convert">
<input type="submit" value="转化">
</form></center>
<?php	
}
pc_db_close($link);
pc_admin_navigation_bar();
html_normal_quit();
?>