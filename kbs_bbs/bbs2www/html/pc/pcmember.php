<?php
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
		$pc = pc_load_infor($link,$_GET["userid"]);
		if(!$pc)
		{
			pc_db_close($link);
			html_error_quit("对不起，您要查看的Blog不存在");
			exit();
		}
		if(!pc_is_admin($currentuser,$pc))
		{
			pc_db_close($link);
			html_error_quit("对不起，您要查看的Blog不存在");
			exit();
		}
		if($pc["TYPE"] != 1)
		{
			pc_db_close($link);
			html_error_quit("对不起，只有公有BLOG才能编辑成员");
			exit();
		}
		
		if($_GET["act"] == "add" && $_GET["id"])
		{
			pc_add_member($link,$pc,$_GET["id"]);
		}
		if($_GET["act"] == "del" && $_GET["id"])
		{
			pc_del_member($link,$pc,$_GET["id"]);
		}
		
		$members = pc_get_members($link,$pc);
		pc_html_init("gb2312",$pc["NAME"]);
?>
<br /><br />
<p align="center"><b>成员管理</b></p>
<center>
<table cellspacing="0" cellpadding="3" border="0" class="t1">
<tr>
	<td class="t2">成员</td>
	<td class="t2">删除</td>
</tr>
<?php
		foreach($members as $member)
			echo "<tr><td class=t3><a href=\"/bbsqry.php?userid=".$member."\">".$member."</a></td>".
			     "<td class=t4><a href=\"pcmember.php?userid=".$pc["USER"]."&act=del&id=".$member."\">删除</a></td></tr>";	
?>
</table>
<form action="pcmember.php" method="get">
<input type="hidden" name="act" value="add">
<input type="hidden" name="userid" value="<?php echo $pc["USER"]; ?>">
<input type="text" name="id">
<input type="submit" value="添加">
</form>
</center>
<p align="center">
[<a href="index.php?id=<?php echo $pc["USER"]; ?>">Blog首页</a>]
[<a href="pcdoc.php?userid=<?php echo $pc["USER"]; ?>&tag=7">Blog参数设定</a>]
[<a href="javascript:history.go(-1)">快速返回</a>]
</p>
<?php		
	pc_db_close($link);
	html_normal_quit();	
	}
	
?>