<?php
/*
**  blog黑名单
**  windinsn Mar 5 , 2004
*/
require("pcadmin_inc.php");
pc_admin_check_permission();
$link = pc_db_connect();

pc_html_init("gb2312",$pcconfig["BBSNAME"]."Blog黑名单管理");
$buserid = $_GET["userid"];
if( $_GET["act"] == "add" && $buserid )
{
	$lookupuser=array();
	if(bbs_getuser($buserid, $lookupuser) == 0)
	{
		echo "<script language=\"javascript\">alert('用户".$buserid."不存在！');</script>";	
	}
	else
	{
		$buserid = $lookupuser["userid"];
		pc_add_blacklist($link , $buserid );
		$action = $currentuser[userid]." 封禁 ".$buserid." 在Blog中的评论权限";
		pc_logs($link , $action , "" , $buserid );
	}
}
if( $_GET["act"] == "del" && $buserid )
{
	pc_del_blacklist($link , $buserid );
	$action = $currentuser[userid]." 恢复 ".$buserid." 在Blog中的评论权限";
	pc_logs($link , $action , "" , $buserid );
}

$query = "SELECT * FROM blacklist WHERE uid = 0;";
$result = mysql_query($query,$link);
$num = mysql_num_rows($result);
pc_admin_navigation_bar();
?>
<br />
<p align="center">Blog黑名单</p>
<center>
<table cellspacing=0 cellpadding=5 width=90% class=t1 border=0>
	<tr>
		<td class=t2 width=50>序号</td>
		<td class=t2>用户名</td>
		<td class=t2 width=80>添加者</td>
		<td class=t2 width=100>添加时间</td>
		<td class=t2 width=50>删除</td>
	</tr>
<?php
		for($i = 0;$i < $num ; $i ++)
		{
			$rows = mysql_fetch_array($result);
			echo "<tr>\n".
				"<td class=t3>".($i + 1)."</td>\n".
				"<td class=t4><a href=\"/bbsqry.php?userid=".$rows[userid]."\">".$rows[userid]."</a></td>\n".
				"<td class=t3><a href=\"/bbsqry.php?userid=".$rows[manager]."\">".$rows[manager]."</a></td>\n".
				"<td class=t4>".time_format($rows[addtime])."</td>\n".
				"<td class=t3><a href=\"pcadmin_bla.php?act=del&userid=".$rows[userid]."\">删除</a></td>\n".
				"</tr>\n";
		}
?>
</table>
</center>
<p align="center">
<form action="pcadmin_bla.php" method="get">
添加黑名单：
<input type="hidden" name="act" id="act" value="add">
<input type="text" name="userid" id="userid" class="f1">
<input type="submit" value="添加" class="f1">
</form>
</center>
<?php
mysql_free_result($result);
pc_db_close($link);
pc_admin_navigation_bar();
html_normal_quit();
?>