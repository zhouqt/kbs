<?php
	/*
	** @id:windinsn dec 3,2003
	*/
	
	@session_start();
	$visitcount = $_SESSION["visitcount"];
	$needlogin=0;
	require("pcfuncs.php");

	function get_calendar_array($link,$pc)
	{
?>
<script language="javascript">
var blogCalendarArray = new Array();
var blogNodeUrl = "pccon.php?id=<?php echo $pc["UID"]; ?>&s=all";
<?php	
		$query = "SELECT `nid` , `created` FROM nodes WHERE `uid` = '".$pc["UID"]."' AND `access` = 0 ORDER BY `nid` DESC;";
		$result = mysql_query($query,$link);
		$bc = array();
		while($rows = mysql_fetch_array($result))
		{
			if(!$bc[substr($rows[created],0,8)])
			{
				$bc[substr($rows[created],0,8)] = $rows[nid] ;
				
?>
blogCalendarArray[<?php echo substr($rows[created],0,8); ?>] = <?php echo (int)($rows[nid]); ?>;
<?php			
			}
			else
				continue;
		}
?>
</script>
<?php
		mysql_free_result($result);
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
			$contentcss = ($nodes[$i][htmltag])?"contentwithhtml":"content";
			if($i%2==0)
				$cellclass=array("t14","t11","t8");
			else
				$cellclass=array("t16","t13","t5");
			echo "<table cellspacing=0 cellpadding=5 width=650 class=t15>\n".
			"<tr><td class=\"".$cellclass[0]."\"><img src=\"icon/".$nodes[$i][emote].".gif\" border=0 align=absmiddle>\n".
			"<a href=\"pccon.php?id=".$pc["UID"]."&tid=".$nodes[$i][tid]."&nid=".$nodes[$i][nid]."&s=all\" class=f2>".html_format($nodes[$i][subject])."</a></td>".
			"<td class=\"".$cellclass[1]."\" align=right>[<a href=\"pccom.php?act=pst&nid=".$nodes[$i][nid]."\">评论</a>]\n[<a href=\"/bbspstmail.php?userid=".$pc["USER"]."&title=问候\">写信问候</a>]</td></tr>\n".
			"<tr><td colspan=2 class=\"".$cellclass[1]."\"><font class='".$contentcss."'>".html_format(substr($nodes[$i][body],0,600)." ",TRUE,$nodes[$i][htmltag]); 
                        if (strlen($nodes[$i][body])>600) 
                        	echo " ......<A href=\"pccon.php?id=".$pc["UID"]."&tid=".$nodes[$i][tid]."&nid=".$nodes[$i][nid]."&s=all\">阅读全文</A>"; 
                        echo "</font></td></tr>\n". 
			"<tr><td colspan=2 class=\"".$cellclass[2]."\"><font class=\"f7\">\nBy <a href=\"/bbsqry.php?userid=".$pc["USER"]."\">".$pc["USER"]."</a> at ".time_format($nodes[$i][created]).
			"\n|\nViews[".$nodes[$i][visitcount]."]".
			"\n|\n<a href=\"pccon.php?id=".$pc["UID"]."&tid=".$nodes[$i][tid]."&nid=".$nodes[$i][nid]."&s=all\">Comments[".$nodes[$i][commentcount]."]</a>";
			if($nodes[$i][trackback])
				echo "\n|\n<a href=\"javascript:openScript('pctb.php?nid=".$nodes[$i][nid]."&uid=".$pc["UID"]."&subject=".base64_encode($nodes[$i][subject])."' , 460, 480)\">TrackBack[".$nodes[$i][trackbackcount]."]</a>";
			echo "</font></td></tr>\n</table>\n";
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
	
	function display_top_bar($link,$pc,$sec,$nodes,$blogs,$pur)
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
		<table cellpadding=5 cellspacing=0 width="100%" border=0 class=t1>
			<tr><td align="left" class="t2">
			.: Introduction :.
			</td></tr>
			<tr><td class="t8">
			<?php 
				if($pc["LOGO"])
					echo "<img alt=\"".$pc["DESC"]."\" hspace=\"5\" src=\"".$pc["LOGO"]."\" align=\"left\" vspace=\"5\" border=\"0\" />";
				echo $pc["DESC"]; 
			?>
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
<?php
			if($pur == 3)
			{
?>			
			<tr><td align="left" class="t3">
			[
			<a href="pcmanage.php?act=post&tag=0&pid=0">添加文章</a>
			]
			</td></tr>
<?php
			}
?>
		</table>
		</td>
	</tr>
	<tr>
		<td align="center" class="t10" valign="top">
		<table cellpadding=3 cellspacing=0 width="100%" border=0 class=t1>
			<tr><td class=t2>
			.:Blog List:.
			</td></tr>
			<tr><td class=t5 align=left>
			<ul>
<?php
		for($i=0;$i<( count($blogs) - 1);$i++)
		{
			echo "<li><a href=\"pcdoc.php?userid=".$pc["USER"]."&tag=0&tid=".$blogs[$i]["TID"]."\">".html_format($blogs[$i]["NAME"])."</a></li>\n";	
			
		}
		
?>			
			</ul>
			</td></tr>
		</table>
		</td>
		<td align="center" class="t3" valign="top">
		<span id='bc'></span>
		<script language=javascript>
		blogCalendar(<?php echo date("Y,m,d"); ?>);
		</script>
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
		<?php
			foreach($pc["LINKS"] as $link)
				echo "<a href='http://".$link["URL"]."'>".htmlspecialchars($link["LINK"])."</a>\n";
		?>
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
	$pc = pc_load_infor($link,$userid,$uid);
	if(!$pc)
	{
		pc_db_close($link);
		html_init("gb2312","个人文集");		
		html_error_quit("对不起，您要查看的个人文集不存在");
		exit();
	}
	
	
	if(pc_is_admin($currentuser,$pc) && $loginok == 1)
	{
		$sec = array("公开区","好友区","私人区","收藏区","删除区","设定好友","文集管理","参数设定");
		$pur = 3;
	}
	elseif(pc_is_friend($currentuser["userid"],$pc["USER"]) || bbs_is_bm($pcconfig["BRDNUM"], $currentuser["index"]))
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
	pc_html_init("gb2312",$pc["NAME"],"","",$pc["BKIMG"]);
	get_calendar_array($link,$pc);
?>
<script src="bc.js"></script>
<center>
<?php
	display_top_bar($link,$pc,$sec,$nodes,$blogs,$pur);
	display_nodes($link,$pc,$nodes);
	display_bottom_bar($pc)
?>
</center>
<?php
	pc_db_close($link);
?>
