<?php
	/*
	** this file display article list in personal corp.
	** @id:windinsn  Nov 19,2003
	*/
	require("pcfuncs.php");
	
	function display_blog_menu($pc,$tid,$tag,$blogMenus)
	{
		$blogs = $blogMenus;
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
	
	function display_action_bar($tag,$tid=0,$pid=0)
	{
		global $pc,$sec,$blogMenus;
?>
<table cellspacing="0" cellpadding="5" border="0" width="95%" class="b2">
<tr>
<td>
<a href="pcmanage.php?userid=<?php echo $pc["USER"]; ?>&act=post&<?php echo "tag=".$tag."&pid=".$pid."&tid=".$tid; ?>">
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
		for( $i = 0 ; $i < count($blogMenus) ; $i ++ )
		{
			if($blogMenus[$i]["TID"]!=$tid)
				echo "<option value=\"T".$blogMenus[$i]["TID"]."\">[".$sec[$tag]."]".html_format($blogMenus[$i]["NAME"])."</option>\n";
		}
?>
</select>
<input type="submit" value="GO" class="b1">
</td>
<td align="right">
<a href="#" onclick="bbsconfirm('pcmanage.php?userid=<?php echo $pc["USER"]; ?>&act=clear&ret=<?php echo urlencode($_SERVER["REQUEST_URI"]); ?>','清空删除区的文章吗(无法恢复)?')">清空删除区</a>
</td>
</tr>
</table>
<?php		
	}
	
	function display_art_list($link,$pc,$tag,$pur,$tid=0,$order="",$pno)
	{
		global $currentuser;
		if ($pc['USER'] == '_filter' )
	    	$query = "SELECT `fid` , `pid` , `nid` , `state` , `username`, `uid` , `recuser`, `created` , `emote` , `changed` , `comment` , `commentcount` , `subject` , `visitcount` , `htmltag` ,`trackbackcount` , `trackback` ".
	    		" FROM filter WHERE `state` = '".$tag."' ";
		else
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
		$query .= "  `created` DESC  ";
		
		$pno = intval($pno);
		if ($pno < 1) $pno = 1; 
		$cnt = 40; //cnt
		$start = ($pno-1)*$cnt;
		
		$query .= " LIMIT ".$start.",".$cnt." ;";
				
		$result = mysql_query($query,$link);
		$i = 0;
		$is_admin = pc_is_manager($currentuser);
?>
<form action="pcmanage.php?userid=<?php echo $pc["USER"]; ?>" method="post">	
<table cellspacing="0" cellpadding="3" border="0" width="99%" class="t1">
<?php
		if ($pc['USER'] == '_filter' )
		{
?>
<tr>
	<td class="t2" width="30">序号</td>
	<td class="t2" width="40">类别</td>
	<td class="t2" width="80">作者</td>
	<td class="t2" width="30"><a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=".$tag."&order=co&tid=".$tid; ?>" class="f3">状态</a></td>
	<td class="t2">主题</td>
	<td class="t2" width="120">
	<a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=".$tag."&order=c&tid=".$tid; ?>" class="f3">创建</a>
	<td class="t2" width="30"><a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=".$tag."&order=v&tid=".$tid; ?>" class="f3">浏览</a></td>
	<td class="t2" width="30"><a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=".$tag."&order=r&tid=".$tid; ?>" class="f3">评论</a></td>
<?php
		if ($tag < 3) {
?>
	<td class="t2" width="15">过</td>
	<td class="t2" width="15">滤</td>
<?php
		} else {
?>
	<td class="t2" width="30">恢复</td>
	<td class="t2" width="80">删文屠夫</td>
<?php
		}
?>
</tr>
<?php
		}
		elseif($pur > 2)
		{
?>
<tr>
	<td class="t2" width="30">序号</td>
	<td class="t2" width="30">选中</td>
	<td class="t2" width="30"><a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=".$tag."&order=co&tid=".$tid; ?>" class="f3">状态</a></td>
	<td class="t2">主题</td>
	<td class="t2" width="120">
	<a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=".$tag."&order=c&tid=".$tid; ?>" class="f3">创建</a>
	|
	<a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=".$tag."&order=u&tid=".$tid; ?>" class="f3">更新</a></td>
	<td class="t2" width="30"><a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=".$tag."&order=v&tid=".$tid; ?>" class="f3">浏览</a></td>
	<td class="t2" width="30"><a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=".$tag."&order=r&tid=".$tid; ?>" class="f3">评论</a></td>
<?php
	if($tag == 0)
	{
?>
	<td class="t2" width="30"><a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=".$tag."&order=tb&tid=".$tid; ?>" class="f3">引用</td>
<?php
	}
?>	
	<td class="t2" width="15">改</td>
	<td class="t2" width="15">删</td>
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
	<td class="t2" width="120"><a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=".$tag."&order=c&tid=".$tid; ?>" class="f3">创建</a>
	|
	<a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=".$tag."&order=u&tid=".$tid; ?>" class="f3">更新</a></td>
	<td class="t2" width="30"><a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=".$tag."&order=v&tid=".$tid; ?>" class="f3">浏览</a></td>
	<td class="t2" width="30"><a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=".$tag."&order=r&tid=".$tid; ?>" class="f3">评论</a></td>
<?php
	if($tag == 0)
	{
?>
	<td class="t2" width="30"><a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=".$tag."&order=tb&tid=".$tid; ?>" class="f3">引用</td>
<?php
	}
	if ($is_admin) {
?>
	<td class="t2" width="30">滤</td>
<?php
	}
?>
</tr>
<?php
		}
		while($rows = mysql_fetch_array($result))
		{
			$i ++;
			if($rows["comment"] == 0)
				$c = "<img src='images/lock.gif' alt='被锁定的主题' border='0'>";
			else
				$c = "<img src='images/open.gif' alt='开放的主题' border='0'>";
		    
		    if ($pc['USER'] == '_filter' )
		    {
		        echo "<tr>\n<td class='t3'>".($start + $i)."</td>\n".
		            "<td class='t4'>".(($rows[nid]&&$tag<3)?'评论':'文章')."</td>".
					"<td align=\"center\" class='t4'><a href=\"/bbsqry.php?userid=".$rows["username"]."\">".html_format($rows["username"])."</a></td>\n".
					"<td class='t3'>".$c."</td>\n".
					"<td class='t5'>";
				echo ($rows["htmltag"]==1)?"&nbsp;":"#";
				echo "<img src=\"icon/".$rows["emote"].".gif\" border=\"0\" align=\"absmiddle\">\n<a href=\"pccon.php?id=".$pc["UID"]."&nid=".$rows["fid"]."&order=".$order."&tid=".$tid."\">".html_format($rows["subject"])."</a></td>\n".
					"<td class='t3'>\n".time_format($rows["created"])."</td>\n".
					"<td class='t4'>".$rows["visitcount"]."</td>\n".
					"<td class='t3'>".$rows["commentcount"]."</td>\n";
				if ($tag < 3) {
					echo	"<td class='t3'><a href=\"pcadmin_flt.php?fid=".$rows["fid"]."&filter=n\">过</a></td>\n".
					"<td class='t4'><a href=\"pcadmin_flt.php?fid=".$rows["fid"]."&filter=y\">滤</a></td>\n";
				} else {
					echo	"<td class='t3'><a href=\"pcadmin_flt.php?fid=".$rows["fid"]."&filter=r\">恢复</a></td>\n".
					"<td class='t4'>".$rows["recuser"]."</td>\n";
				}
				echo "</tr>\n";
			}
			elseif($pur > 2)
			{
				echo "<tr>\n<td class='t3'>".($start + $i)."</td>\n".
					"<td align=\"center\" class='t4'><input type=\"checkbox\" name=\"art".$i."\" value=\"".$rows["nid"]."\" class=\"b2\"></td>\n".
					"<td class='t3'>".$c."</td>\n".
					"<td class='t5'>";
				echo ($rows["htmltag"]==1)?"&nbsp;":"#";
				echo "<img src=\"icon/".$rows["emote"].".gif\" border=\"0\" align=\"absmiddle\">\n<a href=\"pccon.php?id=".$pc["UID"]."&nid=".$rows["nid"]."&order=".$order."&tid=".$tid."\">".html_format($rows["subject"])."</a></td>\n".
					"<td class='t3'>\n".time_format($rows["created"])."<br/>".time_format($rows["changed"])."\n</td>\n".
					"<td class='t4'>".$rows["visitcount"]."</td>\n".
					"<td class='t3'>".$rows["commentcount"]."</td>\n";
				if($tag == 0)
				{
					
					echo "<td class='t4'>";
					echo $rows["trackback"]?$rows["trackbackcount"]:"-";
					echo "</td>\n";
				}
				echo	"<td class='t3'><a href=\"pcmanage.php?userid=".$pc["USER"]."&act=edit&nid=".$rows["nid"]."\">改</a></td>\n".
					"<td class='t4'><a href=\"#\" onclick=\"bbsconfirm('pcmanage.php?userid=".$pc["USER"]."&act=del&nid=".$rows["nid"]."','确认删除?')\">删</a></td>\n".
					"</tr>\n";
			}
			else
			{
				echo "<tr>\n<td class='t3'>".($start + $i)."</td>\n".
					"<td class='t4'>".$c."</td>\n".
					"<td class='t8'>&nbsp;<img src=\"icon/".$rows["emote"].".gif\" border=\"0\ align=\"absmiddle\">\n<a href=\"pccon.php?id=".$pc["UID"]."&nid=".$rows["nid"]."&order=".$order."&tid=".$tid."\">".html_format($rows["subject"])."</a></td>\n".
					"<td class='t4'>\n".time_format($rows["created"])."<br/>".time_format($rows["changed"])."\n</td>\n".
					"<td class='t3'>".$rows["visitcount"]."</td>\n".
					"<td class='t4'>".$rows["commentcount"]."</td>\n";
				if($tag == 0)
				{
					
					echo "<td class='t3'>";
					echo $rows["trackback"]?$rows["trackbackcount"]:"-";
					echo "</td>\n";
				}
				if ($is_admin) {
					echo "<td class='t4'><a href=\"#\" onclick=\"bbsconfirm('pcadmin_del.php?userid=".$pc["USER"]."&nid=".$rows["nid"]."','确认删除?')\">滤</a></td>\n";
				}
				echo	"</tr>\n";
			}
		}
?>
</table>
<p align="center" class="f1">
<?php
		if ($pno > 1)
		    echo "[<a href=\"pcdoc.php?userid=".$pc["USER"]."&tag=".$tag."&tid=".$tid."\">第一页</a>]&nbsp;[<a href=\"pcdoc.php?userid=".$pc["USER"]."&tag=".$tag."&tid=".$tid."&pno=".($pno-1)."\">上一页</a>]&nbsp;";
		if ($cnt == $i)
		    echo "[<a href=\"pcdoc.php?userid=".$pc["USER"]."&tag=".$tag."&tid=".$tid."&pno=".($pno+1)."\">下一页</a>]";
?>
</p>
<?php		    
		if($pur > 2)
			display_action_bar($tag,$tid);
?>
</form>
<?php		
		mysql_free_result($result);
	}

	function display_fav_folder($link,$pc,$pid=0,$pur,$order="")
	{
		
		
		$rootpid = pc_fav_rootpid($link,$pc["UID"]);
		if(!$rootpid)
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
		$query .= " `type` DESC ;";
		
		$result = mysql_query($query,$link);
		$i = 0;
?>
<form action="pcmanage.php?userid=<?php echo $pc["USER"]; ?>" method="post">	
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
	<td class="t2" width="120">
	<a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=3&pid=".$pid."&order=c"; ?>" class="f3">创建</a>
	|
	<a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=3&pid=".$pid."&order=u"; ?>" class="f3">更新</a>
	</td>
	<td class="t2" width="30"><a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=3&pid=".$pid."&order=v"; ?>" class="f3">浏览</a></td>
	<td class="t2" width="30"><a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=3&pid=".$pid."&order=r"; ?>" class="f3">评论</a></td>
	<td class="t2" width="15">改</td>
	<td class="t2" width="15">删</td>
	<td class="t2" colspan="<?php echo $_COOKIE["BLOGFAVACTION"]?3:2; ?>">操作</a>
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
	<td class="t2" width="120">
	<a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=3&pid=".$pid."&order=c"; ?>" class="f3">创建</a>
	|
	<a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=3&pid=".$pid."&order=u"; ?>" class="f3">更新</a></td>
	<td class="t2" width="30"><a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=3&pid=".$pid."&order=v"; ?>" class="f3">浏览</a></td>
	<td class="t2" width="30"><a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=3&pid=".$pid."&order=r"; ?>" class="f3">评论</a></td>
</tr>
<?php
		}
		while($rows = mysql_fetch_array($result))
		{
			$i ++;
			if($rows["comment"] == 1 && $rows["type"] == 0)
				$c = "<img src='images/open.gif' alt='开放的主题' border='0'>";
			else
				$c = "<img src='images/lock.gif' alt='被锁定的主题' border='0'>";
			if($rows["type"]==1)
			{
				$type = "<img src='images/dir.gif' alt='目录' border='0'>";
				$url = "pcdoc.php?userid=".$pc["USER"]."&tag=3&pid=".$rows["nid"];
			}
			else
			{
				$type = "<img src='images/art.gif' alt='文章' border='0'>";
				$url = "pccon.php?id=".$pc["UID"]."&nid=".$rows["nid"]."&order=".$order;
			}
			if( $pur > 2)
			{
				echo "<tr>\n<td class='t3'>".$i."</td>\n<td align=\"center\" class='t4'>";
				if($rows["type"]==0)
					echo "<input type=\"checkbox\" name=\"art".$i."\" value=\"".$rows["nid"]."\" class=\"b2\">";
				else
					echo "&nbsp;";
				echo	"</td>\n<td class='t3'>".$type."</td>\n".
					"<td class='t4'>".$c."</td>\n".
					"<td class='t8'>";
				echo   ($rows["htmltag"]==1)?"&nbsp;":"#";	
				echo    "<img src=\"icon/".$rows["emote"].".gif\" border=\"0\" align=\"absmiddle\">\n<a href=\"".$url."\">".html_format($rows["subject"])."</a></td>\n".
					"<td class='t4'>".time_format($rows["created"])."<br/>".time_format($rows["changed"])."</td>\n".
					"<td class='t3'>".$rows["visitcount"]."</td>\n".
					"<td class='t4'>".$rows["commentcount"]."</td>\n".
					"<td class='t3'><a href=\"pcmanage.php?userid=".$pc["USER"]."&act=edit&nid=".$rows["nid"]."\">改</a></td>\n".
					"<td class='t4'><a href=\"#\" onclick=\"bbsconfirm('pcmanage.php?userid=".$pc["USER"]."&act=del&nid=".$rows["nid"]."','确认删除?')\">删</a></td>\n";
				if($rows["type"]==0)
					echo "<td class='t3' width=20><a href=\"pcmanage.php?userid=".$pc["USER"]."&act=favcut&nid=".$rows["nid"]."\">剪</a></td>".
					      "<td class='t3' width=20><a href=\"pcmanage.php?userid=".$pc["USER"]."&act=favcopy&nid=".$rows["nid"]."\">复</a></td>";
				else
					echo "<td class='t3' width=20>-</td>\n<td class='t3'>-</td>\n";
				if(isset($_COOKIE["BLOGFAVACTION"]) && $_COOKIE["BLOGFAVACTION"])
				{
					if($rows["type"]==1)
						echo 	"<td class='t3' width=20><a href=\"pcmanage.php?userid=".$pc["USER"]."&act=favpaste&pid=".$rows["nid"]."\">贴</a></td>";
					else
						echo "<td class='t3' width=20>-</td>";
				}
				echo 	"</tr>\n";
			}
			else
				echo "<tr>\n<td class='t3'>".$i."</td>\n".
					"<td class='t4'>".$type."</td>\n".
					"<td class='t3'>".$c."</td>\n".
					"<td class='t5'>&nbsp;<img src=\"icon/".$rows["emote"].".gif\" border=\"0\" align=\"absmiddle\">\n<a href=\"".$url."\">".html_format($rows["subject"])."</a></td>\n".
					"<td class='t3'>".time_format($rows["created"])."<br/>".time_format($rows["changed"])."</td>\n".
					"<td class='t4'>".$rows["visitcount"]."</td>\n".
					"<td class='t3'>".$rows["commentcount"]."</td>\n".
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
			$prepid = ($rows["pid"]>$rootpid)?$rows["pid"]:$rootpid;
?>
<p align="center"  class="b2">
[<a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=3&pid=".$rows["pid"]; ?>">返回上层目录</a>]
[<a href="pcdoc.php?<?php echo "userid=".$pc["USER"]."&tag=3"; ?>">返回根目录</a>]
</p>
<?php		
		}
		if( $pur > 2 )
		{
			display_action_bar(3,0,$pid);			
?>
</form>
<?php
		if(isset($_COOKIE["BLOGFAVACTION"]) && $_COOKIE["BLOGFAVACTION"])
			echo "<p align='center' class='b2'>[<a href=\"pcmanage.php?userid=".$pc["USER"]."&act=favpaste&pid=".$rootpid."\">粘贴到根目录</a>]</p>\n";
?>
<form action="pcmanage.php?userid=<?php echo $pc["USER"]; ?>&act=adddir" method="post" onsubmit="if(this.dir.value==''){alert('请输入目录名!');return false;}">
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
		}
		
		if(pc_is_groupwork($pc))
			pc_group_logs($link,$pc,"ADD FRIEND: ".$id);
	}
	
	
	function del_friend($pc)
	{
		$id = $_GET["id"];	
		pc_del_friend($id,$pc["USER"]);
		if(pc_is_groupwork($pc))
			pc_group_logs($link,$pc,"DEL FRIEND: ".$id);
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
		$blog = pc_blog_menu($link,$pc);	
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
		for($i = 0; $i < count($blog); $i ++)
		{
			echo "<tr>\n<td class='t3'>".($i+1)."</td>\n".
				"<td class='t5'>&nbsp;<a href=\"pcdoc.php?userid=".$pc["USER"]."&tag=".$blog[$i]["TAG"]."&tid=".$blog[$i]["TID"]."\">《".html_format($blog[$i]["NAME"])."》</a></td>\n".
				"<td class='t3'><a href=\"pcdoc.php?userid=".$pc["USER"]."&tag=".$blog[$i]["TAG"]."\">".$sec[$blog[$i]["TAG"]]."</a></td>\n".
				"<td class='t4'><a href=\"pcmanage.php?userid=".$pc["USER"]."&act=tedit&tid=".$blog[$i]["TID"]."\">修改</a></td>\n".
				"<td class='t3'><a href=\"pcmanage.php?userid=".$pc["USER"]."&act=tdel&tid=".$blog[$i]["TID"]."\">"."删除</a></td>\n".
				"</tr>\n";	
			
		}
		
?>
</table>
<form action="pcmanage.php?userid=<?php echo $pc["USER"]; ?>&act=tadd" method="post" onsubmit="if(this.topicname.value==''){alert('请输入Blog名称!');return false;}">
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
	    global $pcconfig;
?>
<form action="pcmanage.php?userid=<?php echo $pc["USER"]; ?>&act=sedit" method="post" onsubmit="if(this.pcname.value==''){alert('请输入Blog名称!');return false;}">	
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
	<?php pc_select_blogtheme($pc["THEM"]); ?>
	[<a href="pcsec.php?act=list">查看本站可用的主题</a>]
	</td>
</tr>
<tr>
	<td class="t3">启用站外信箱</td>
	<td class="t5">&nbsp;
	<input type="text" maxlength="200" name="pcuseremail" value="<?php echo $pc["EMAIL"]; ?>" class="f1">
	(留空表示使用本站信箱)
	</td>
</tr>
<tr>
	<td class="t3">收藏夹模式</td>
	<td class="t5">&nbsp;
	<input type="radio" name="pcfavmode" value="0" <?php if($pc["FAVMODE"]==0) echo "checked"; ?> >
	私有
	<input type="radio" name="pcfavmode" value="1" <?php if($pc["FAVMODE"]==1) echo "checked"; ?> >
	对好友开放
	<input type="radio" name="pcfavmode" value="2" <?php if($pc["FAVMODE"]==2) echo "checked"; ?> >
	完全开放
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
	<td class="t3">公开区默认分类</td>
	<td class="t5">&nbsp;
	<input type="text" name="pcdefaulttopic" maxlength="100" value="<?php echo htmlspecialchars($pc["DEFAULTTOPIC"]); ?>" class="f1">
	(留空表示取消公开区默认分类，建议在取消前移走该分类的所有文章)
	</td>
</tr>
<tr>
	<td class="t3">友情链接管理</td>
	<td class="t5">&nbsp;
	<a href="pclinks.php?userid=<?php echo $pc["USER"]; ?>">点击此处</a>
	</td>
</tr>
<?php
	if(pc_is_groupwork($pc))
	{
?>
<tr>
	<td class="t3">成员管理</td>
	<td class="t5">&nbsp;
	<a href="pcmember.php?userid=<?php echo $pc["USER"]; ?>">点击此处</a>
	</td>
</tr>
<?php
	}
	if($pcconfig["USERFILES"])
	    if($pc["FILELIMIT"] && $pc["FILENUMLIMIT"])
	    {
?>
<tr>
	<td class="t3">个人空间</td>
	<td class="t5">&nbsp;
	<a href="pcfile.php?userid=<?php echo $pc["USER"]; ?>">管理我的个人空间</a>
	</td>
</tr>
<?php
	    }
?>
<tr>
	<td class="t3">HTML编辑器</td>
	<td class="t5">
	<?php /*
	<input type="radio" name="htmleditor" value="0" <?php if($pc["EDITOR"]==0) echo "checked"; ?>>HTMLArea编辑器
	<input type="radio" name="htmleditor" value="9" <?php if($pc["EDITOR"]==9) echo "checked"; ?>>不使用编辑器
	*/
	?>
	<input type="radio" name="htmleditor" value="1" <?php if($pc["EDITOR"]==1) echo "checked"; ?>>HTMLArea编辑器
	<input type="radio" name="htmleditor" value="2" <?php if($pc["EDITOR"]==2) echo "checked"; ?>>UBB编辑器
	<input type="radio" name="htmleditor" value="0" <?php if($pc["EDITOR"]==0) echo "checked"; ?>>不使用编辑器
	<input type="radio" name="htmleditor" value="3" <?php if($pc["EDITOR"]==3) echo "checked"; ?>>HTMLArea编辑器(rc1)
	</td>
</tr>
<tr>
	<td class="t3">Blog模板</td>
	<td class="t5">
	<input type="radio" name="template" value="0" <?php if($pc["STYLE"]["SID"]==0) echo "checked"; ?>>默认模板
	<input type="radio" name="template" value="1" <?php if($pc["STYLE"]["SID"]==1) echo "checked"; ?>>水木社区
	<input type="radio" name="template" value="2" <?php if($pc["STYLE"]["SID"]==2) echo "checked"; ?>>Earth Song
	<input type="radio" name="template" value="9" <?php if($pc["STYLE"]["SID"]==9) echo "checked"; ?>>自定义模板
	<a href="pcstyle.php?userid=<?php echo $pc["USER"]; ?>">管理自定义模板(XML/XSL/CSS)</a>
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
<?php
		if(defined("_BLOG_ANONY_COMMENT_")) {
?>
<tr>
	<td class="t3">是否允许匿名用户评论</td>
	<td class="t5">
		<input type="checkbox" name="anonycomment" value="yes" class=f1<?php echo $pc["ANONYCOMMENT"]?" checked":""; ?>>勾选表示允许匿名评论
	</td>
</tr>
<?php
		}
?>
<tr>
	<td class="t3">设定黑名单</td>
	<td class="t5">
	<a href="pcblist.php">设定Blog黑名单</a>
	</td>
</tr>
<tr>
	<td class="t3">首页其他信息(支持HTML语法和JavaScript)</td>
	<td class="t5">
	<textarea name="userinfor" id="userinfor" class="f1" rows=10 cols=50><?php echo htmlspecialchars($pc["INFOR"]); ?></textarea>
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
	if(isset($_GET["pid"]))
		$pid = intval($_GET["pid"]);
	else
		$pid = 0;
	if(isset($_GET["tag"]))
		$tag = intval($_GET["tag"]);
	else
		$tag = 0;
	if(isset($_GET["tid"]))
		$tid = intval($_GET["tid"]);
	else
		$tid = 0;
		
	$link = pc_db_connect();
	$pc = pc_load_infor($link,$userid);
	if(!$pc)
	{
		pc_db_close($link);
		html_init("gb2312","Blog");		
		html_error_quit("对不起，您要查看的Blog不存在");
		exit();
	}
	$f_err = "";
	if(pc_is_admin($currentuser,$pc) && $loginok == 1)
	{
		if(isset($_GET["act"])) {
			if($_GET["act"] == "addfriend")
				$f_err = add_friend($pc);
			if($_GET["act"] == "delfriend")
				del_friend($pc);
		}
	}	
	
	$isfriend = pc_is_friend($currentuser["userid"],$pc["USER"]);
	$userPermission = pc_get_user_permission($currentuser,$pc);
	$sec = $userPermission["sec"];
	$pur = $userPermission["pur"];
	$tags = $userPermission["tags"];
		
	$secnum = count($sec);
	if(!$tags[$tag]) $tag = 0;
	
	/*visit count start*/
	if(!($pur == 3 && !pc_is_groupwork($pc)))//Blog所有者的访问不进行计数  windinsn dec 10,2003
		pc_counter($link);
	/*visit count end*/
	
	//if( pc_cache( $pc["MODIFY"] ) )
	//	return;
	
	if($tag == 0 || $tag ==1 || $tag ==2)
	{
		$blogMenus = pc_blog_menu($link,$pc,$tag);
		if(!$pc["DEFAULTTOPIC"] && $tag == 0 && !$tid)
			$tid = $blogMenus[0]["TID"];
	}
	else
	{
		$blogMenus = NULL;
	}
	
	pc_html_init("gb2312",$pc["NAME"],"","",$pc["BKIMG"]);
?>
<a name="top"></a>
<table cellspacing="0" cellpadding="0" border="0" width="100%">
<tr>
	<td>
	<table cellspacing="0" cellpadding="3" border="0" class="t0" width="100%" class="tt1">
		<tr>
			<td class="tt1">&nbsp;&nbsp;&nbsp;&nbsp;<?php echo "<a href=\"/\" class=\"f1\">".$pcconfig["BBSNAME"]."</a> - <a href='index.html' class=\"f1\">Blog</a> - <a href=\"index.php?id=".$pc["USER"]."\" class=\"f1\">".$pc["NAME"]."</a>"; ?></td>
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

<tr>
<td  valign="top">
<table cellspacing="0" cellpadding="0" border="0" width="100%">
<tr>
<?php
	if ($pc['USER']!='_filter')
	    if($tag == 0 || $tag == 1 || $tag == 2 || $tag == 4)
	    {
?>
	<td rowspan="2" align="middle" valign="top" width="150">
	<?php display_blog_menu($pc,$tid,$tag,$blogMenus); ?>
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
	while(list($secTag,$secTagValue) = each($tags))
	{
		if(!$secTagValue) continue;
		if($secTag == $tag)
			echo "<td width='70' class='t23'>".$sec[$tag]."</td>\n";
		else
			echo "<td width='70' class='t22'><a href=\"pcdoc.php?userid=".$pc["USER"]."&tag=".$secTag."\" class='b22'>".$sec[$secTag]."</a></td>\n";
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
	if(isset($_GET["order"]))
		$order = $_GET["order"];
	else
		$order = "";
	if(isset($_GET["pno"]))
		$pno = $_GET["pno"];
	else
		$pno = 0;
	if($tag == 3)
		display_fav_folder($link,$pc,$pid,$pur,addslashes($order));
	elseif($tag < 5 )
		display_art_list($link,$pc,$tag,$pur,$tid,addslashes($order),$pno);
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
	[
	<?php bbs_add_super_fav ($pc['NAME']); ?>
	]
	<br>&nbsp;
	</td>
</tr>
<tr>
	<td align="center" class="tt3" valign="middle" height="25">
	[<a href="#top" class=f1>返回顶部</a>]
	[<a href='javascript:location.reload()' class=f1>刷新</a>]
	[<?php 
		echo "<a href=\"";
		if($pc["EMAIL"])
			echo "mailto:".$pc["EMAIL"];
		else
			echo "/bbspstmail.php?userid=".$pc["USER"]."&title=问候";
		echo "\" class=f1>给".$pc["USER"]."写信</a>"; 
	?>]
	[<a href="index.php?id=<?php echo $pc["USER"]; ?>" class=f1><?php echo $pc["NAME"]; ?>首页</a>]
	[<a href="index.html" class=f1>Blog首页</a>]
	[<a href="/frames.html" class=f1 target="_top"><?php echo $pcconfig["BBSNAME"]; ?>首页</a>]
	<a href="rss.php?userid=<?php echo $pc["USER"]; ?>"><img src="images/xml.gif" border="0" alt="XML" align="absmiddle"></a>
	</td>
</tr>
</table>
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
?>
