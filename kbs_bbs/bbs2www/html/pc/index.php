<?php
	/*
	** @id:windinsn dec 3,2003
	*/
	
	@session_start();
	$visitcount = $_SESSION["visitcount"];
	$needlogin=0;
	require("pcfuncs.php");

	function display_calendar()
	{
		
		
	}
	
	function pc_load_nodes($link,$pc)
	{
		$query = "SELECT * FROM `nodes` WHERE `uid` = '".$pc["UID"]."' AND `access` = 0 ORDER BY `nid` DESC LIMIT 0 , 10 ;";
		$result = mysql_query($query,$link);
		$nodes = array();
		$i = 0;
		while($rows=mysql_fetch_array($result))
		{
			$nodes[$i]=$rows;
			$i ++;
		}
		mysql_free_result($result);
		return $nodes;
	}
	
	function display_nodes($link,$pc,$nodes)
	{
		for($i=0;$i<min(5,count($nodes));$i++)
		{
			if($i%2==0)
				$cellclass=array("t14","t11","t8");
			else
				$cellclass=array("f1","t13","t5");
			echo "<table cellspacing=0 cellpadding=5 width=650 class=t15>\n".
			"<tr><td class=\"".$cellclass[0]."\"><img src=\"icon/".$nodes[$i][emote].".gif\" border=0 align=absmiddle>\n".
			"<a href=\"pccon.php?id=".$pc["UID"]."&tid=".$nodes[$i][tid]."&nid=".$nodes[$i][nid]."&s=all\" class=f2>".html_format($nodes[$i][subject])."</a></td>".
			"<td class=\"".$cellclass[1]."\" align=right>[<a href=\"pccom.php?act=pst&nid=".$nodes[$i][nid]."\">Add Comment</a>]\n[<a href=\"/bbspstmail.php?userid=".$pc["USER"]."&title=问候\">Mail ".$pc["USER"]."</a>]</td></tr>\n".
			"<tr><td colspan=2 class=\"".$cellclass[1]."\">".html_format($nodes[$i][body],TRUE)."</td></tr>\n".
			"<tr><td colspan=2 class=\"".$cellclass[2]."\"><font class=\"f7\">\nBy <a href=\"/bbsqry.php?userid=".$pc["USER"]."\">".$pc["USER"]."</a> at ".time_format($nodes[$i][created]).
			"\n|\nViews[".$nodes[$i][visitcount]."]".
			"\n|\n<a href=\"pccon.php?id=".$pc["UID"]."&tid=".$nodes[$i][tid]."&nid=".$nodes[$i][nid]."&s=all\">Comments[".$nodes[$i][commentcount]."]</a>".
			"\n|\nTrackBack[0]".
			"</font></td></tr>\n</table>\n";
		}
	}
	
	function display_newnodes_list($link,$pc,$nodes)
	{
?>
<ul>
<?php		
		for($i=0;$i< count($nodes);$i++)
		{
			echo "<li><a href=\"pccon.php?id=".$pc["UID"]."&nid=".$nodes[$i][nid]."&tid=".$nodes[$i][tid]."\">".html_format($nodes[$i][subject])."</a>(".time_format($nodes[$i][created]).")</li>\n";
		}
		
?>
</ul>
<p class="f1" align="right">
<a href="pcdoc.php?userid=<?php echo $pc["USER"]; ?>">
More Articles
</a>&nbsp;&nbsp;&nbsp;&nbsp;
</p>
<?php
	}
	
	function display_top_bar($link,$pc,$sec,$nodes,$blogs)
	{
		global $loginok,$currentuser,$pcconfig;
		
?>
<table cellpadding=0 cellspacing=0 width=650 border=0 class=f1>
<tr><td valign=middle align=center>
<!--
<img src="<?php echo $pc["STYLE"]["TOPIMG"]; ?>" alt="<?php echo $pc["NAME"]; ?>" border=0>
-->
<object classid="clsid:D27CDB6E-AE6D-11cf-96B8-444553540000" codebase="http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=6,0,29,0" width="700" height="80">
              <param name="movie" value="images/SMTH2.swf">
              <param name="quality" value="high">
              <embed src="images/SMTH2.swf" quality="high" pluginspage="http://www.macromedia.com/go/getflashplayer" type="application/x-shockwave-flash" width="805" height="90"></embed>
</object>
</td></tr></table>
<table cellpadding=0 cellspacing=0 width=650 border=0 class=f1>
<tr>
	<td align="center">
	<table cellpadding=5 cellspacing=0 border=0 class=t1>
		<tr>
<?php
		for($i=0;$i<min(4,count($sec));$i++)
		{
			echo "<td class=t11><a href=\"pcdoc.php?userid=".$pc["USER"]."&tag=".$i."\" class=\"f1\">:".$sec[$i].":</a></td>\n";
		}
?>
			<td class=t11>
			<a href="/bbsdoc.php?board=<?php echo $pcconfig["BOARD"]; ?>" class="f1">:Blog论坛:</a>
			</td>
			<td class=t11>
			<a href="pcsearch2.php" class="f1">:Blog搜索:</a>
			</td><td class=t11>
			<a href="pc.php" class="f1">:Blog首页:</a>
			</td><td class=t11>
			<a href="/<?php echo ($loginok==1 && strcmp($currentuser["userid"],"guest"))?"frames.html":"guest-frames.html"; ?>" class="f1">:<?php echo BBS_FULL_NAME; ?>:</a>
			</td>
		</tr>
	</table>
	</td>
</tr>
</table>
<table cellpadding=10 cellspacing=0 width=650 border=0 class=t1>
	<tr>
		<td align="center" class="t11" colspan="2">
		<table cellpadding=3 cellspacing=0 width="100%" border=0 class=t1>
			<tr><td align="left" class="t2">
			.: Introduction :.
			</td></tr>
			<tr><td class="t8">
			<?php echo $pc["DESC"]; ?>
			</td></tr>
		</table>
		</td>
	</tr>
	<tr>
		<td align="center" class="t14" width="400" valign="TOP">
		<table cellpadding=3 cellspacing=0 width="100%" border=0 class=t1>
			<tr><td align="left" class="t2">
			.: What's New :.
			</td></tr>
			<tr><td class="t8">
			<?php display_newnodes_list($link,$pc,$nodes); ?>
			</td></tr>
		</table>
		</td>
		<td align="center" class="t11" valign="TOP">
		<table cellpadding=3 cellspacing=0 width="100%" border=0 class=t1>
			<form action="pcnsearch.php" method="get" onsubmit="if(this.keyword.value==''){alert('请输入关键字');return false;}">
			<input type="hidden" name="area" value="<?php echo $pc["USER"]; ?>">
			<tr><td align="left" class="t2">
			.: Blog Tools :.
			</td></tr>
			<tr><td align="left" class="t3">
			:: Blog Search ::
			</td></tr>
			<tr><td class="t4">
			<input type="text" name="keyword" id="keyword" class="f1">
			<input type="submit" value="GO" class="f1">
			</td></tr>
			</form>
			<tr><td align="left" class="t3">
			:: Login ::
			</td></tr>
			<?php
				if($loginok==1 && strcmp($currentuser["userid"],"guest"))
				{
?>
<tr><td class="t4">
<strong><?php echo $currentuser["userid"]; ?></strong>
&nbsp;
[<a href="/bbslogout.php" target="_top" class="f1">Logout</a>]
<?php
				}
				else
				{
?>
<form action="/bbslogin.php" method="post" name="form1" >
<tr><td class="t4">
UserName:
<INPUT TYPE=text class="f1" size="12" onMouseOver="this.focus()" onFocus="this.select()" name="id" >
<br>
PassWord:
<INPUT TYPE=password  class="f1" size="12" name="passwd" maxlength="39">
<br>
<INPUT TYPE=submit value="Login" class="f1">
<?php					
				}
			?>
			</td></tr>
		</table>
		</td>
	</tr>
	<tr>
		<td align="center" class="t3" colspan="2">
		<table cellpadding=3 cellspacing=0 width="100%" border=0 class=t1>
			<tr><td class=t2>
			.:Blog List:.
			</td></tr>
			<tr><td class=t4>
<?php
		for($i=0;$i<( count($blogs) - 1);$i++)
		{
			echo "<a href=\"pcdoc.php?userid=".$pc["USER"]."&tag=0&tid=".$blogs[$i]["TID"]."\">".html_format($blogs[$i]["NAME"])."</a>&nbsp;\n";	
			
		}
		
?>			
			</td></tr>
		</table>
		</td>
	</tr>
</table>		
<?php		
	}

	function display_bottom_bar($pc)
	{
		global $pcconfig;
?>	
<table cellpadding=10 cellspacing=0 width=650 border=0 class=t15>
<tr><td align="center" class="t11">
	<table cellpadding=3 cellspacing=0 width=100% border=0 class=t1>
		<tr><td class="t2">
		.: Links :. 
		</td></tr>
		<tr><td class="t4">
		<a href="/bbsdoc.php?board=<?php echo $pcconfig["BOARD"]; ?>" class="f1">:Blog论坛:</a>
		<a href="pcsearch2.php" class="f1">:Blog搜索:</a>
		<a href="pc.php" class="f1">:Blog首页:</a>
		<a href="<?php echo $pcconfig["SITE"]; ?>" class="f1">:<?php echo BBS_FULL_NAME; ?>:</a>
		<br><br>
		<a href="rss.php?userid=<?php echo $pc["USER"]; ?>" target="_blank">
		<img src="images/xml.gif" align="absmiddle" alt="XML" border="0">
		</a>
		</td></tr>
	</table>
</td></tr>
<tr><td class="t3">
	访问量 
	<font class="f4">
	<?php echo $pc["VISIT"]; ?>
	</font>
	&nbsp;&nbsp;&nbsp;&nbsp;
	更新时间:
	<?php echo time_format($pc["MODIFY"]); ?>
	<br>
&copy;All Rights Reserved
&nbsp;&nbsp;
http://<?php echo $pc["USER"]; ?>.mysmth.net
</td></tr>
</table>	
<?php		
	}
	
	
	$userid = addslashes($_GET["id"]);
	$uid = (int)($_GET["id"]);
	
	$link = pc_db_connect();
	$query = "SELECT * FROM users WHERE `username`= '".$userid."' OR `uid` = '".$uid."';";
	$result = mysql_query($query,$link);
	$rows = mysql_fetch_array($result);
	if(!$rows)
	{
		mysql_free_result($result);
		pc_db_close($link);
		html_init("gb2312","个人文集");		
		html_error_quit("对不起，您要查看的个人文集不存在");
		exit();
	}
	
	$pc = array(
			"NAME" => html_format($rows[corpusname]),
			"USER" => $rows[username],
			"UID" => $rows[uid],
			"DESC" => html_format($rows[description]),
			"THEM" => html_format($rows[theme]),
			"TIME" => $rows[createtime],
			"VISIT" => $rows[visitcount],
			"MODIFY" => $rows[modifytime],
			"STYLE" => pc_style_array($rows[stype]),
			"LOGO" => str_replace("<","&lt;",stripslashes($rows[logoimage])),
			"BKIMG" => str_replace("<","&lt;",stripslashes($rows[backimage]))
			);
	mysql_free_result($result);
	
	if(pc_is_admin($currentuser,$pc) && $loginok == 1)
	{
		$sec = array("公开区","好友区","私人区","收藏区","删除区","设定好友","文集管理","参数设定");
		$pur = 3;
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
	$nodes = pc_load_nodes($link,$pc);
	$blogs = pc_blog_menu($link,$pc["UID"],0);
	/*visit count start*/
	if($pur != 3)//文集所有者的访问不进行计数  windinsn dec 10,2003
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
	pc_html_init("gb2312",stripslashes($rows[corpusname]),"","",$pc["BKIMG"]);
	
?>
<center>
<?php
	display_top_bar($link,$pc,$sec,$nodes,$blogs);
	display_nodes($link,$pc,$nodes);
	display_bottom_bar($pc)
?>
</center>
<?php
	pc_db_close($link);
?>