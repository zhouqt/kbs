<?php
/*
**  blog黑名单
**  windinsn Mar 5 , 2004
*/
	require("pcfuncs.php");
	if ($loginok != 1)
		html_nologin();
	elseif(!strcmp($currentuser["userid"],"guest"))
	{
		html_init("gb2312");
		html_error_quit("guest 没有Blog!");
		exit();
	}
	else
	{
		$link = pc_db_connect();
		$pc = pc_load_infor($link,$currentuser["userid"]);
		if(!$pc || !pc_is_admin($currentuser,$pc))
		{
			pc_db_close($link);
			html_error_quit("对不起，您要查看的Blog不存在");
			exit();
		}
		pc_html_init("gb2312",$pc["NAME"]);
		
		$buserid = $_GET["userid"];
		if( $_GET["act"] == "add" && $buserid )
		{
			$lookupuser=array ();
			if(bbs_getuser($buserid, $lookupuser) == 0)
			{
				echo "<script language=\"javascript\">用户".$buserid."不存在！</script>";	
			}
			else
			{
				$buserid = $lookupuser["userid"];
				if(!pc_in_blacklist($link , $buserid , $pc["UID"]))
					pc_add_blacklist($link , $buserid , $pc["UID"]);	
			}
		}
		if( $_GET["act"] == "del" && $buserid )
		{
			pc_del_blacklist($link , $buserid , $pc["UID"]);
		}
		
		$query = "SELECT * FROM blacklist WHERE uid = ".$pc[UID].";";
		$result = mysql_query($query,$link);
		$num = mysql_num_rows($result);
?>
<br/>
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
				"<td class=t3><a href=\"pcblist.php?act=del&userid=".$rows[userid]."\">删除</a></td>\n".
				"</tr>\n";
		}
?>
</table>
</center>
<p align="center">
<form action="pcblist.php" method="get">
添加黑名单：
<input type="hidden" name="act" value="add">
<input type="text" name="userid" id="userid" class="f1">
<input type="submit" value="添加" class="f1">
</form>
</center>
<hr size=1>
<p class=f1 align=center>
[<a href="index.php?id=<?php echo $pc["USER"]; ?>">Blog首页</a>]
[<a href="pcdoc.php?userid=<?php echo $pc["USER"]; ?>&tag=7">Blog参数设定</a>]
[<a href="javascript:history.go(-1)">快速返回</a>]
</p>
<?php
		mysql_free_result($result);
		pc_db_close($link);
		html_normal_quit();
	}
?>