<?php
	/*
	** this file display article list in personal corp.
	** @id:windinsn  Nov 19,2003
	*/
	@session_start();
	/*
	**	对收藏夹的剪切、复制操作,计数器需要 session 支持 windinsn nov 25,2003
	*/
	//$needlogin=0;
	/*
	** ../funcs.php中将未登录用户自动初始化为guest，这里不需要传递$needlogin=0，否则不能进行管理 windinsn dec 24,2003
	*/
	require("pcfuncs.php");
	
	function display_blog_menu($link,$pc,$tag,$tid=0)
	{
		$blogs = pc_blog_menu($link,$pc["UID"],$tag);
?>
<table cellspacing="0" cellpadding="5" border="0" width="95%">
<tr>
	<td align="right" class="f2">Blog分类</td>
</tr>
<tr>
	<td align="right"><hr align="right" width="90%" class="hr"></td>
</tr>
<?php
		for($i=0;$i < count($blogs) ;$i++)
		{
			if($blogs[$i]["TID"] == $tid)
				echo "<tr>\n<td align=\"right\" class='t6'>\n".html_format($blogs[$i]["NAME"])."\n</td>\n</tr>\n";
			else
				echo "<tr>\n<td align=\"right\" class='t7'>\n<a href=\"pcdoc.php?userid=".$pc["USER"]."&tag=".$tag."&tid=".$blogs[$i]["TID"]."\" class='t7'>".html_format($blogs[$i]["NAME"])."</a>\n</td>\n</tr>\n";
		}
?>
</table>
<?php		
	}
	
	function display_action_bar($tag,$pid=0)
	{
		global $sec;
?>
<table cellspacing="0" cellpadding="5" border="0" width="95%" class="b2">
<tr>
<td>
<a href="pcmanage.php?act=post&<?php echo "tag=".$tag."&pid=".$pid; ?>">
<img src="images/post.gif" border="0" alt="发表文章">
</a>
</td>
<td align="right">
<input type="hidden" name="access" value="<?php echo $tag; ?>">
<input onclick="checkall(this.form)" type="checkbox" value="on" name="chkall" align="absmiddle" class="b2">
选中本目录下所有文章
&nbsp;&nbsp;&nbsp;&nbsp;
将选中文章
<select name="act" class="b2">
	<option value="cut" selected>移动</option>
	<option value="copy">复制</option>
</select>
到
<select name="target" class="b2">
<?php
		for( $i = 0 ; $i < 5 ; $i ++)
		{
			if($i!=$tag)
				echo "<option value=\"".$i."\">".$sec[$i]."</option>\n";
		}
?>
</select>
<input type="submit" value="GO" class="b1">
</td>
<td align="right">
<a href="#" onclick="bbsconfirm('pcmanage.php?act=clear','清空删除区的文章吗(无法恢复)?')">清空删除区</a>
</td>
</tr>
</table>
<?php		
	}
	
	function display_art_list($link,$pc,$tag,$pur,$tid=0,$order="")
	{
		$query = "SELECT `nid` , `pid` ,  `created` , `emote` , `changed` , `comment` , `commentcount` , `subject` , `visitcount` , `htmltag` ,`trackbackcount` , `trackback` ".
			" FROM nodes WHERE `access` = '".$tag."' AND `uid` = '".$pc["UID"]."'  AND `tid` = '".$tid."' ";
		switch($order)
		{
			case "c":
				$query.=" ORDER BY `created` DESC , ";
				break;
			case "u":
				$query.=" ORDER BY `changed` DESC , ";
				break;
			case "v":
				$query.=" ORDER BY `visitcount`  DESC , ";
				break;
			case "r":
				$query.=" ORDER BY `commentcount`  DESC , ";
				break;
			case "co":
				$query.=" ORDER BY `comment`  ASC , ";
				break;
			case "tb":
				$query.=" ORDER BY `trackbackcount` DESC , ";
				break;
			default:
				$query.=" ORDER BY ";
				
		}	
		$query .= "  `created` DESC ;";
		
		$result = mysql_query($query,$link);
		$i = 0;
?>
<form action="pcmanage.php" method="post">	
<table cellspacing="0" cellpadding="3" border="0" width="99%" class="t1">
<?php
		if($pur > 2)
		{
?>
<tr>
	<td class="t2" width="30">序号</td>
	<td class="t2" width="30">选中</td>
	<td class="t2" width="30"><a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=".$tag."&order=co&tid=".$tid; ?>" class="f3">状态</a></td>
	<td class="t2">主题</td>
	<td class="t2" width="260">
	<a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=".$tag."&order=c&tid=".$tid; ?>" class="f3">创建时间</a>
	|
	<a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=".$tag."&order=u&tid=".$tid; ?>" class="f3">更新时间</a></td>
	<td class="t2" width="40"><a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=".$tag."&order=v&tid=".$tid; ?>" class="f3">浏览</a></td>
	<td class="t2" width="40"><a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=".$tag."&order=r&tid=".$tid; ?>" class="f3">评论</a></td>
<?php
	if($tag == 0)
	{
?>
	<td class="t2" width="40"><a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=".$tag."&order=tb&tid=".$tid; ?>" class="f3">引用</td>
<?php
	}
?>	
	<td class="t2" width="30">修改</td>
	<td class="t2" width="30">删除</td>
</tr>
<?php
		}
		else
		{
?>
<tr>
	<td class="t2" width="30">序号</td>
	<td class="t2" width="30"><a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=".$tag."&order=co&tid=".$tid; ?>" class="f3">状态</a></td>
	<td class="t2">主题</td>
	<td class="t2" width="260"><a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=".$tag."&order=c&tid=".$tid; ?>" class="f3">创建时间</a>
	|
	<a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=".$tag."&order=u&tid=".$tid; ?>" class="f3">更新时间</a></td>
	<td class="t2" width="40"><a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=".$tag."&order=v&tid=".$tid; ?>" class="f3">浏览</a></td>
	<td class="t2" width="40"><a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=".$tag."&order=r&tid=".$tid; ?>" class="f3">评论</a></td>
<?php
	if($tag == 0)
	{
?>
	<td class="t2" width="40"><a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=".$tag."&order=tb&tid=".$tid; ?>" class="f3">引用</td>
<?php
	}
?>
</tr>
<?php
		}
		while($rows = mysql_fetch_array($result))
		{
			$i ++;
			if($rows[comment] == 0)
				$c = "<img src='images/lock.gif' alt='被锁定的主题' border='0'>";
			else
				$c = "<img src='images/open.gif' alt='开放的主题' border='0'>";
			if($pur > 2)
			{
				echo "<tr>\n<td class='t3'>".$i."</td>\n".
					"<td align=\"center\" class='t4'><input type=\"checkbox\" name=\"art".$i."\" value=\"".$rows[nid]."\" class=\"b2\"></td>\n".
					"<td class='t3'>".$c."</td>\n".
					"<td class='t5'>";
				echo ($rows[htmltag]==1)?"&nbsp;":"#";
				echo "<img src=\"icon/".$rows[emote].".gif\" border=\"0\" align=\"absmiddle\">\n<a href=\"pccon.php?id=".$pc["UID"]."&nid=".$rows[nid]."&order=".$order."&tid=".$tid."\">".html_format($rows[subject])."</a></td>\n".
					"<td class='t3'>\n".time_format($rows[created])."\n|\n".time_format($rows[changed])."\n</td>\n".
					"<td class='t4'>".$rows[visitcount]."</td>\n".
					"<td class='t3'>".$rows[commentcount]."</td>\n";
				if($tag == 0)
				{
					
					echo "<td class='t4'>";
					echo $rows[trackback]?$rows[trackbackcount]:"-";
					echo "</td>\n";
				}
				echo	"<td class='t3'><a href=\"pcmanage.php?act=edit&nid=".$rows[nid]."\">修改</a></td>\n".
					"<td class='t4'><a href=\"#\" onclick=\"bbsconfirm('pcmanage.php?act=del&nid=".$rows[nid]."','确认删除?')\">删除</a></td>\n".
					"</tr>\n";
			}
			else
			{
				echo "<tr>\n<td class='t3'>".$i."</td>\n".
					"<td class='t4'>".$c."</td>\n".
					"<td class='t8'>&nbsp;<img src=\"icon/".$rows[emote].".gif\" border=\"0\ align=\"absmiddle\">\n<a href=\"pccon.php?id=".$pc["UID"]."&nid=".$rows[nid]."&order=".$order."&tid=".$tid."\">".html_format($rows[subject])."</a></td>\n".
					"<td class='t4'>\n".time_format($rows[created])."\n|\n".time_format($rows[changed])."\n</td>\n".
					"<td class='t3'>".$rows[visitcount]."</td>\n".
					"<td class='t4'>".$rows[commentcount]."</td>\n";
				if($tag == 0)
				{
					
					echo "<td class='t3'>";
					echo $rows[trackback]?$rows[trackbackcount]:"-";
					echo "</td>\n";
				}
				echo	"</tr>\n";
			}
		}
?>
</table>
<?php
		if($pur > 2)
			display_action_bar($tag);
?>
</form>
<?php		
		mysql_free_result($result);
	}

	function display_fav_folder($link,$pc,$pid=0,$pur,$order="")
	{
		$query = "SELECT `nid` FROM nodes WHERE `access` = '3' AND  `uid` = '".$pc["UID"]."' AND `pid` = '0' AND `type` = '1' LIMIT 0 , 1 ;";
		$result = mysql_query($query,$link);
		if($rows = mysql_fetch_array($result))
		{
			$rootpid = $rows[nid];
			mysql_free_result($result);
		}
		else
		{
			$pif = pc_init_fav($link,$pc["UID"]);
			if($pif)
			{
?>
<script language="javascript">window.location.href="pcdoc.php?userid=<?php echo $pc["USER"]; ?>&tag=3";</script>
<?php
			}
			else
			{
				html_error_quit("对不起，Blog收藏夹初始化错误!");
				exit();
			}
		}	
		
		if($pid == 0)
			$pid = $rootpid;
		else
		{
			$query = "UPDATE nodes SET `visitcount` = visitcount + 1 WHERE  `access` = '3' AND `nid` = '".$pid."' AND `uid` = '".$pc["UID"]."';";
			mysql_query($query,$link);
		}
		
		$query = "SELECT `nid` , `type` , `created` , `changed` , `emote` , `comment` , `commentcount` , `subject` , `visitcount`,`pid`,`htmltag` ".
			" FROM nodes WHERE `access` = '3' AND `uid` = '".$pc["UID"]."' AND `pid` = '".$pid."' ";
		switch($order)
		{
			case "c":
				$query.=" ORDER BY `created` DESC , ";
				break;
			case "u":
				$query.=" ORDER BY `changed` DESC  ,";
				break;
			case "v":
				$query.=" ORDER BY `visitcount`  DESC  ,";
				break;
			case "r":
				$query.=" ORDER BY `commentcount`  DESC  ,";
				break;
			case "co":
				$query.=" ORDER BY `comment`  ASC ,";
				break;
			default:
				$query.=" ORDER BY ";
				
		}	
		$query .= " `type` ASC ;";
		
		$result = mysql_query($query,$link);
		$i = 0;
?>
<form action="pcmanage.php" method="post">	
<table cellspacing="0" cellpadding="5" border="0" width="99%" class="t1">
<?php
		if($pur > 2)
		{
?>
<tr>
	<td class="t2" width="30">序号</td>
	<td class="t2" width="30">选中</td>
	<td class="t2" width="30">类型</td>
	<td class="t2" width="30"><a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=3&pid=".$pid."&order=co"; ?>" class="f3">状态</a></td>
	<td class="t2">主题</td>
	<td class="t2" width="250">
	<a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=3&pid=".$pid."&order=c"; ?>" class="f3">创建时间</a>
	|
	<a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=3&pid=".$pid."&order=u"; ?>" class="f3">更新时间</a>
	</td>
	<td class="t2" width="40"><a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=3&pid=".$pid."&order=v"; ?>" class="f3">浏览</a></td>
	<td class="t2" width="40"><a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=3&pid=".$pid."&order=r"; ?>" class="f3">评论</a></td>
	<td class="t2" width="30">修改</td>
	<td class="t2" width="30">删除</td>
	<td class="t2" colspan="<?php echo session_is_registered('favaction')?3:2; ?>">操作</a>
</tr>
<?php
		}
		else
		{
?>
<tr>
	<td class="t2" width="30">序号</td>
	<td class="t2" width="30">类型</td>
	<td class="t2" width="30"><a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=3&pid=".$pid."&order=co"; ?>" class="f3">状态</a></td>
	<td class="t2">主题</td>
	<td class="t2" width="260">
	<a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=3&pid=".$pid."&order=c"; ?>" class="f3">创建时间</a>
	|
	<a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=3&pid=".$pid."&order=u"; ?>" class="f3">更新时间</a></td>
	<td class="t2" width="40"><a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=3&pid=".$pid."&order=v"; ?>" class="f3">浏览</a></td>
	<td class="t2" width="40"><a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=3&pid=".$pid."&order=r"; ?>" class="f3">评论</a></td>
</tr>
<?php
		}
		while($rows = mysql_fetch_array($result))
		{
			$i ++;
			if($rows[comment] == 1 && $rows[type] == 0)
				$c = "<img src='images/open.gif' alt='开放的主题' border='0'>";
			else
				$c = "<img src='images/lock.gif' alt='被锁定的主题' border='0'>";
			if($rows[type]==1)
			{
				$type = "<img src='images/dir.gif' alt='目录' border='0'>";
				$url = "pcdoc.php?userid=".$pc["USER"]."&tag=3&pid=".$rows[nid];
			}
			else
			{
				$type = "<img src='images/art.gif' alt='文章' border='0'>";
				$url = "pccon.php?id=".$pc["UID"]."&nid=".$rows[nid]."&order=".$order;
			}
			if( $pur > 2)
			{
				echo "<tr>\n<td class='t3'>".$i."</td>\n<td align=\"center\" class='t4'>";
				if($rows[type]==0)
					echo "<input type=\"checkbox\" name=\"art".$i."\" value=\"".$rows[nid]."\" class=\"b2\">";
				else
					echo "&nbsp;";
				echo	"</td>\n<td class='t3'>".$type."</td>\n".
					"<td class='t4'>".$c."</td>\n".
					"<td class='t8'>";
				echo   ($rows[htmltag]==1)?"&nbsp;":"#";	
				echo    "<img src=\"icon/".$rows[emote].".gif\" border=\"0\" align=\"absmiddle\">\n<a href=\"".$url."\">".html_format($rows[subject])."</a></td>\n".
					"<td class='t4'>".time_format($rows[created])."|".time_format($rows[changed])."</td>\n".
					"<td class='t3'>".$rows[visitcount]."</td>\n".
					"<td class='t4'>".$rows[commentcount]."</td>\n".
					"<td class='t3'><a href=\"pcmanage.php?act=edit&nid=".$rows[nid]."\">修改</a></td>\n".
					"<td class='t4'><a href=\"#\" onclick=\"bbsconfirm('pcmanage.php?act=del&nid=".$rows[nid]."','确认删除?')\">删除</a></td>\n";
				if($rows[type]==0)
					echo "<td class='t3' width=20><a href=\"pcmanage.php?act=favcut&nid=".$rows[nid]."\">剪</a></td>".
					      "<td class='t3' width=20><a href=\"pcmanage.php?act=favcopy&nid=".$rows[nid]."\">复</a></td>";
				else
					echo "<td class='t3' width=20>-</td>\n<td class='t3'>-</td>\n";
				if(session_is_registered("favaction"))
				{
					if($rows[type]==1)
						echo 	"<td class='t3' width=20><a href=\"pcmanage.php?act=favpaste&pid=".$rows[nid]."\">贴</a></td>";
					else
						echo "<td class='t3' width=20>-</td>";
				}
				echo 	"</tr>\n";
			}
			else
				echo "<tr>\n<td class='t3'>".$i."</td>\n".
					"<td class='t4'>".$type."</td>\n".
					"<td class='t3'>".$c."</td>\n".
					"<td class='t5'>&nbsp;<img src=\"icon/".$rows[emote].".gif\" border=\"0\" align=\"absmiddle\">\n<a href=\"".$url."\">".html_format($rows[subject])."</a></td>\n".
					"<td class='t3'>".time_format($rows[created])."|".time_format($rows[changed])."</td>\n".
					"<td class='t4'>".$rows[visitcount]."</td>\n".
					"<td class='t3'>".$rows[commentcount]."</td>\n".
					"</tr>\n";
		}
		mysql_free_result($result);
?>
</table>
<?php
		if($pid != $rootpid)
		{
			$query = "SELECT `pid` FROM nodes WHERE `nid` = '".$pid."' LIMIT 0 , 1 ;";
			$result = mysql_query($query);
			$rows = mysql_fetch_array($result);
			mysql_free_result($result);
			$prepid = ($rows[pid]>$rootpid)?$rows[pid]:$rootpid;
?>
<p align="center"  class="b2">
[<a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=3&pid=".$rows[pid]; ?>">返回上层目录</a>]
[<a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=3"; ?>">返回根目录</a>]
</p>
<?php		
		}
		if( $pur > 2 )
		{
			display_action_bar(3,$pid);			
?>
</form>
<?php
		if(session_is_registered("favaction"))
			echo "<p align='center' class='b2'>[<a href=\"pcmanage.php?act=favpaste&pid=".$rootpid."\">粘贴到根目录</a>]</p>\n";
?>
<form action="pcmanage.php?act=adddir" method="post" onsubmit="if(this.dir.value==''){alert('请输入目录名!');return false;}">
<input type="hidden" name="pid" value="<?php echo $pid; ?>">
<p class="b2" align="center">
新建目录:
<input type="text" name="dir" maxlength="200" size="40" id="dir" class="b2">
<input type="submit" value="新建目录" class="b1">
</p>
</form>
<?php			
		}
	}
	
	function add_friend($pc)
	{
		$id = $_GET["id"];
		$lookupuser=array ();
		
		if($friendid = pc_is_friend($id,$pc["USER"]))
			return $friendid."已在好友列表中!";
		elseif( $id=="" || bbs_getuser($id, $lookupuser) == 0 )
			return "用户 ".$id." 不存在!";				
		else
		{
			$id = $lookupuser["userid"];
			pc_add_friend($id,$pc["USER"]);
			return "";
		}
	}
	
	
	function del_friend($pc)
	{
		$id = $_GET["id"];	
		pc_del_friend($id,$pc["USER"]);
	}
		
	
	function display_friend_manage($pc,$err="")
	{
		$friendlist = pc_friend_list($pc["USER"]);
?>
<table cellspacing="0" cellpadding="5" border="0" width="99%" class="t1">
<tr>
	<td class="t2" width="25%">序号</td>
	<td class="t2">好友</td>
	<td class="t2">Blog</td>
	<td class="t2" width="25%">删除</td>
</tr>
<?php
		for($i = 0;$i < count($friendlist) ; $i ++)
		{
			echo "<tr>\n<td class='t3'>".($i+1)."</td>\n".
				"<td class='t4'><a href=\"/bbsqry.php?userid=".$friendlist[$i]."\">".$friendlist[$i]."</a></td>\n".
				"<td class='t3'><a href=\"pcsearch.php?exact=1&key=u&keyword=".$friendlist[$i]."\">".$friendlist[$i]."</a></td>\n".
				"<td class='t4'><a href=\"pcdoc.php?userid=".$pc["USER"]."&tag=5&act=delfriend&id=".urlencode($friendlist[$i])."\">删除</a></td>\n</tr>\n";
			
		}
?>
<form action="pcdoc.php" method="get" onsubmit="if(this.id.value==''){alert('请输入好友用户名！');return false;}">
<tr>
	<td colspan="3" align="center">
	<input type="hidden" name="act" value="addfriend">
	<input type="hidden" name="tag" value="5">
	<input type="hidden" name="userid" value="<?php echo $pc["USER"]; ?>">
	<input type="text" size="12" name="id" id="id" class="b2">
	<input type="submit" value="添加好友" class="b1">
<?php
	echo $err;
?>
	</td>
</tr>
</form>
</table>
<?php		
	}
	
	function display_blog_settings($link,$pc,$tag)
	{
		global $sec;
		$blog = pc_blog_menu($link,$pc["UID"]);	
?>
<table cellspacing="0" cellpadding="5" border="0" width="99%" class="t1">
<tr>
	<td class="t2" width="50">序号</td>
	<td class="t2">Blog</td>
	<td class="t2" width="200">分区</td>
	<td class="t2" width="50">修改</td>
	<td class="t2" width="50">删除</td>
</tr>
<?php
		for($i = 0; $i < count($blog) - 1; $i ++)
		{
			echo "<tr>\n<td class='t3'>".($i+1)."</td>\n".
				"<td class='t5'>&nbsp;<a href=\"pcdoc.php?userid=".$pc["USER"]."&tag=".$blog[$i]["TAG"]."&tid=".$blog[$i]["TID"]."\">《".html_format($blog[$i]["NAME"])."》</a></td>\n".
				"<td class='t3'><a href=\"pcdoc.php?userid=".$pc["USER"]."&tag=".$blog[$i]["TAG"]."\">".$sec[$blog[$i]["TAG"]]."</a></td>\n".
				"<td class='t4'><a href=\"pcmanage.php?act=tedit&tid=".$blog[$i]["TID"]."\">修改</a></td>\n".
				"<td class='t3'><a href=\"pcmanage.php?act=tdel&tid=".$blog[$i]["TID"]."\">"."删除</a></td>\n".
				"</tr>\n";	
			
		}
		
?>
</table>
<form action="pcmanage.php?act=tadd" method="post" onsubmit="if(this.topicname.value==''){alert('请输入Blog名称!');return false;}">
<input type="hidden" name="tag" value="<?php echo $tag; ?>">
<p align="center" class="b2">
新建Blog：
分区
<select name="access" class="b2">
	<option value="0">公开区</option>
	<option value="1">好友区</option>
	<option value="2">私人区</option>
</select>
Blog名
<input type="text" name="topicname" maxlength="200" size="30" class="b2">
<input type="submit" value="新建Blog" class="b1">
</p>
</form>
<?php
	}
	
	function display_pc_settings($pc)
	{
?>
<form action="pcmanage.php?act=sedit" method="post" onsubmit="if(this.pcname.value==''){alert('请输入Blog名称!');return false;}">	
<table cellspacing="0" cellpadding="3" border="0" width="99%" class="t1">		
<tr>
	<td class="t2" colspan="2">参数设定</td>
</tr>
<tr>
	<td class="t3">Blog名称</td>
	<td class="t5">&nbsp;
	<input type="text" maxlength="40" name="pcname" id="pcname" value="<?php echo $pc["NAME"]; ?>" class="f1">
	</td>
</tr>
<tr>
	<td class="t3">Blog描述</td>
	<td class="t5">&nbsp;
	<input type="text" maxlength="200" name="pcdesc" value="<?php echo $pc["DESC"]; ?>" class="f1">
	</td>
</tr>
<tr>
	<td class="t3">Blog主题</td>
	<td class="t5">&nbsp;
	<input type="text" maxlength="20" name="pcthem" value="<?php echo $pc["THEM"]; ?>" class="f1">
	</td>
</tr>
<tr>
	<td class="t3">Logo图片</td>
	<td class="t5">&nbsp;
	<input type="text" maxlength="255" name="pclogo" value="<?php echo htmlspecialchars($pc["LOGO"]); ?>" class="f1">
	(请填写Logo图片所在的URL地址，留空表示无LOGO图片)
	</td>
</tr>
<tr>
	<td class="t3">背景图片</td>
	<td class="t5">&nbsp;
	<input type="text" name="pcbkimg" maxlength="255" value="<?php echo htmlspecialchars($pc["BKIMG"]); ?>" class="f1">
	(请填写背景图片所在的URL地址，留空表示无背景图片)
	</td>
</tr>
<tr>
	<td class="t3">友情链接管理</td>
	<td class="t5">&nbsp;
	<a href="pclinks.php">点击此处</a>
	</td>
</tr>
<tr>
	<td class="t3">HTML编辑器</td>
	<td class="t5">
	<input type="radio" name="htmleditor" value="0" <?php if($pc["EDITOR"]==0) echo "checked"; ?>>HTMLArea编辑器
	<input type="radio" name="htmleditor" value="9" <?php if($pc["EDITOR"]==9) echo "checked"; ?>>不使用编辑器
	</td>
</tr>
<tr>
	<td class="t3">Blog模板</td>
	<td class="t5">
	<input type="radio" name="template" value="0" <?php if($pc["STYLE"]["SID"]==0) echo "checked"; ?>>默认模板
	<input type="radio" name="template" value="1" <?php if($pc["STYLE"]["SID"]==1) echo "checked"; ?>>水木清华
	<input type="radio" name="template" value="2" <?php if($pc["STYLE"]["SID"]==2) echo "checked"; ?>>Earth Song
	
	</td>
</tr>
<tr>
	<td class="t3">Blog首页显示文章数</td>
	<td class="t5">
	<input type="input" name="indexnodes" class=f1 size=1 maxlength=1 value="<?php echo $pc["INDEX"]["nodeNum"]; ?>">篇(至多9篇)
	</td>
</tr>
<tr>
	<td class="t3">Blog首页每篇文章显示字节</td>
	<td class="t5">
	<input type="input" name="indexnodechars" class=f1 size=5 maxlength=5 value="<?php echo $pc["INDEX"]["nodeChars"]; ?>">字节(设定0表示显示整篇文章)
	</td>
</tr>
<tr>
	<td class="t4" colspan="2">
	<input type="submit" value="修改Blog参数" class="b1">
	<input type="reset" value="恢复原始参数" class="b1">
	</td>
</tr>
<tr>
</table>
</form>
<br>
<?php		
	}
	
	$userid = addslashes($_GET["userid"]);
	$pid = (int)($_GET["pid"]);
	$tag = (int)($_GET["tag"]);
	$visitcount = $_SESSION["visitcount"];
	
	$link = pc_db_connect();
	$pc = pc_load_infor($link,$userid);
	if(!$pc)
	{
		pc_db_close($link);
		html_init("gb2312","Blog");		
		html_error_quit("对不起，您要查看的Blog不存在");
		exit();
	}
		
	$isfriend = pc_is_friend($currentuser["userid"],$pc["USER"]);
	if(pc_is_admin($currentuser,$pc) && $loginok == 1)
	{
		$sec = array("公开区","好友区","私人区","收藏区","删除区","设定好友","Blog管理","参数设定");
		$pur = 3;
		if($_GET["act"] == "addfriend")
			$f_err = add_friend($pc);
		if($_GET["act"] == "delfriend")
			del_friend($pc);
	}
	elseif($isfriend || bbs_is_bm($pcconfig["BRDNUM"], $currentuser["index"]))
	{
		$sec = array("公开区","好友区");
		$pur = 1;
	}
	else
	{
		$sec = array("公开区");
		$pur = 0;
	}
	$secnum = count($sec);
	if($tag < 0 || $tag > $secnum )
		$tag = 0;
	
	/*visit count start*/
	if($pur != 3)//Blog所有者的访问不进行计数  windinsn dec 10,2003
	{
		if(!session_is_registered("visitcount"))
		{
			pc_visit_counter($link,$pc["UID"]);//计数器加1
			$pc["VISIT"] ++;
			$visitcount = ",".$pc["UID"].",";
			session_register("visitcount");
		}
		elseif(!stristr($visitcount,",".$pc["UID"].","))
		{
			pc_visit_counter($link,$pc["UID"]);//计数器加1
			$pc["VISIT"] ++;
			$visitcount .= $pc["UID"].",";
			$_SESSION["visitcount"] .= $pc["UID"].",";
		}
	}
	/*visit count end*/
	
	pc_html_init("gb2312",$pc["NAME"],"","",$pc["BKIMG"]);
?>
<a name="top"></a>
<table cellspacing="0" cellpadding="0" border="0" width="100%">
<tr>
	<td>
	<table cellspacing="0" cellpadding="3" border="0" class="t0" width="100%" class="tt1">
		<tr>
			<td class="tt1">&nbsp;&nbsp;&nbsp;&nbsp;<?php echo "<a href=\"/\" class=\"f1\">".BBS_FULL_NAME."</a> - <a href='pc.php' class=\"f1\">Blog</a> - <a href=\"index.php?id=".$pc["USER"]."\" class=\"f1\">".$pc["NAME"]."</a>"; ?></td>
			<td align="right" class="tt1"><?php echo pc_personal_domainname($pc["USER"]); ?>&nbsp;&nbsp;&nbsp;&nbsp;</td>
		</tr>
	</table>
	</td>
</tr>
<tr>
	<td class="f2" align="center" height="40" valign="middle">
	<?php echo $pc["USER"]; ?> 的Blog
	-
	<?php echo $pc["NAME"]; ?>
	</td>
</tr>
<tr>
	<td>
	<table cellspacing="0" cellpadding="10" border="0" width="100%" class="tt2">
	<tr>
<?php
	if($pc["LOGO"])
		echo "<td><img src=\"".$pc["LOGO"]."\" border=\"0\" alt=\"".$pc["DESC"]."\"></td>\n";

?>	
		<td align="left">&nbsp;<?php echo $pc["DESC"]; ?></td>
	</tr>
	</table>
	</td>
</tr>
<?php /*
<tr>
	<td align="center">
<?php
		display_calendar($link,$pc);
?>	
	</td>
</tr>
*/
?>
<tr>
<td  valign="top">
<table cellspacing="0" cellpadding="0" border="0" width="100%">
<tr>
<?php
	if($tag == 0 || $tag == 1 || $tag ==2 )
	{
?>
	<td rowspan="2" align="middle" valign="top" width="150">
	<?php display_blog_menu($link,$pc,$tag,(int)($_GET["tid"])); ?>
	</td>
<?php
	}
?>
	<td align="left" valign="top">
	<table cellspacing="0" cellpadding="0" border="0" width="100%">
	<tr>
		<td><img src="images/empty.gif"></td>
		<td align="right" valign="top">
	<table cellspacing="0" cellpadding="3" border="0" class='t21'>
	<tr>
<?php
	for($i=0 ; $i < $secnum ;$i ++)
	{
		if($i == $tag)
			echo "<td width='70' class='t23'>".$sec[$i]."</td>\n";
		else
			echo "<td width='70' class='t22'><a href=\"pcdoc.php?userid=".$pc["USER"]."&tag=".$i."\" class='b22'>".$sec[$i]."</a></td>\n";
	}
?>	
	</tr>
	</table>
	</td>
	<td width="10"><img src="images/empty.gif"></td>
	</tr>
	</table>
	</td>
</tr>
<tr>
	<td align="left" valign="top">
<?php
	if($tag == 3)
		display_fav_folder($link,$pc,$pid,$pur,addslashes($_GET["order"]));
	elseif($tag < 5 )
		display_art_list($link,$pc,$tag,$pur,(int)($_GET["tid"]),addslashes($_GET["order"]));
	elseif($tag == 5)
		display_friend_manage($pc,$f_err);
	elseif($tag == 6)
		display_blog_settings($link,$pc,$tag);
	elseif($tag == 7)
		display_pc_settings($pc);
?>
	</td>
</tr>
</table>
</td></tr>
<tr>
	<td align="center" valign="middle" class="f1">
	[访问量 
	<font class="f4">
	<?php echo $pc["VISIT"]; ?>
	</font>
	]
	&nbsp;&nbsp;&nbsp;&nbsp;
	[更新时间:
	<?php echo time_format($pc["MODIFY"]); ?>
	]
	&nbsp;&nbsp;&nbsp;&nbsp;
	[
	<a href="pcnsearch.php?userid=<?php echo $pc["USER"]; ?>">文章搜索</a>
	]
	&nbsp;&nbsp;&nbsp;&nbsp;
	[
	<a href="index.php?id=<?php echo $pc["USER"]; ?>"><?php echo $pc["NAME"]; ?></a>
	]
	<br>&nbsp;
	</td>
</tr>
<tr>
	<td align="center" class="tt3" valign="middle" height="25">
	[<a href="#top" class=f1>返回顶部</a>]
	[<a href='javascript:location=location' class=f1>刷新</a>]
	[<?php echo "<a href=\"/bbspstmail.php?userid=".$pc["USER"]."&title=问候\" class=f1>给".$pc["USER"]."写信</a>"; ?>]
	[<a href="index.php?id=<?php echo $pc["USER"]; ?>" class=f1><?php echo $pc["NAME"]; ?>首页</a>]
	[<a href="pc.php" class=f1>Blog首页</a>]
	[<a href="
<?php
	if(!strcmp($currentuser["userid"],"guest"))
		echo "/guest-frames.html";
	else
		echo "/frames.html";
?>	
	" class=f1 target="_top"><?php echo BBS_FULL_NAME; ?>首页</a>]
	<a href="rss.php?userid=<?php echo $pc["USER"]; ?>"><img src="images/xml.gif" border="0" alt="XML" align="absmiddle"></a>
	</td>
</tr>
</table>
<?php
	pc_db_close($link);
	html_normal_quit();
?>