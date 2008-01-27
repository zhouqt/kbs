<?php
	/*
	** @id:windinsn dec 3,2003
	*/
	require("pcfuncs.php");
	function pc_edit_link($link,$favlinks,$uid)
	{
		global $pc;
		$links = "";
		foreach($favlinks as $favlink)
		{
			if($links == "")
				$links .= base64_encode($favlink["LINK"])."#".base64_encode($favlink["URL"])."#".(int)($favlink["IMAGE"]);
			else
				$links .= "|".base64_encode($favlink["LINK"])."#".base64_encode($favlink["URL"])."#".(int)($favlink["IMAGE"]);
		}
			
		$query = "UPDATE users SET `createtime` = `createtime` , `modifytime` = '".date("YmdHis")."' , `links` = '".addslashes($links)."' WHERE `uid` = '".$uid."' ";
		mysql_query($query,$link);
		
		if($pc["TYPE"]==1)
			pc_group_logs($link,$pc,"EDIT LINKS");
	}
	
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
		
		$favlinks = $pc["LINKS"];
		$act = isset($_GET["act"])?$_GET["act"]:"";
		
		if($act == "edit")
		{
			$favlinks = array();
			$favlinksnum = count($pc["LINKS"]);
			for($i = 0;$i < $favlinksnum ; $i ++ )
			{
				if(!$_POST["link".$i] || !$_POST["url".$i]) continue;
				$favlinks[$i] = array("LINK" => $_POST["link".$i] , "URL" => $_POST["url".$i] , "IMAGE" => (int)($_POST["image".$i]));
			}
			if($_POST["link".$favlinksnum] && $_POST["url".$favlinksnum])
				$favlinks[$favlinksnum] =array("LINK" => $_POST["link".$favlinksnum] , "URL" => $_POST["url".$favlinksnum] , "IMAGE" => (int)($_POST["image".$favlinksnum]));
			pc_edit_link($link,$favlinks,$pc["UID"]);
		}
		if($act == "del" && $_GET["linkid"])
		{
			$favlinks = array();
			for($i = 0;$i < count($pc["LINKS"]);$i ++)
			{
				if($i != $_GET["linkid"] - 1)
				{
					$isImage = $pc["LINKS"][$i]["IMAGE"]?1:0;
					$favlinks[] = array("LINK" => $pc["LINKS"][$i]["LINK"] ,"URL" => $pc["LINKS"][$i]["URL"] , "IMAGE" => $isImage );
				}
			}
			pc_edit_link($link,$favlinks,$pc["UID"]);
		}
		$favlinksnum = count($favlinks) + 1;
		$favlinks[] = array("URL" => NULL,"LINK" => NULL);
		
		pc_html_init("gb2312",$pc["NAME"]);
?>
<br><br><p align=center class=f2>友情链接管理</p>
<hr size=1>
<center>
<form action="pclinks.php?userid=<?php echo $pc["USER"]; ?>&act=edit" method="post">
<table cellspacing=0 cellpadding=5 width=98% border=0 class=t1>
	<tr>
		<td class=t2 width=30>编号</td>
		<td class=t2 width=120>名称</td>
		<td class=t2>链接</td>
		<td class=t2 width=30>图片</td>
		<td class=t2 width=80>删除</td>
	</tr>
<?php
	
	for($i = 0 ; $i < $favlinksnum ; $i ++)
	{
		echo "<tr>\n<td class=t3><strong>".($i + 1)."</strong></td>".
			"<td class=t8><input size=30 name='link".$i."' value='".$favlinks[$i]["LINK"]."' class=f1></td>\n".
			"<td class=t5>http://<input size=50 name='url".$i."' value='".$favlinks[$i]["URL"]."' class=f1></td>\n".
			"<td class=t3><input type=checkbox name='image".$i."' value=1 ";
		if(@$favlinks[$i]["IMAGE"]) echo " checked ";
		echo "></td>\n";
		if( $i != $favlinksnum - 1 )
			echo "<td class=t4><a href='pclinks.php?userid=".$pc["USER"]."&act=del&linkid=".($i+1)."'>删除</a>\n<a href='http://".$favlinks[$i]["URL"]."'>链接</a></td>";
		else
			echo "<td class=t4>-</td>";
		echo "</tr>\n";
	}
?>
<tr>
	<td class=t4 colspan=5><input type=submit value="修改" class=b1></a>
</tr>
</table>
</form>
说明：若为图片链接，请在“名称”一栏内填入图片的URL地址。
</center>
<hr size=1>
<p class=f1 align=center>
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
