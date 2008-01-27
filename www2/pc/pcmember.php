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
		if(!pc_is_groupwork($pc))
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
	pc_db_close($link);
	html_normal_quit();	
	}
	
?>
