<?php
	/*
	** @id:windinsn dec 3,2003
	*/
	require("pcfuncs.php");

	function pc_get_archfile($pc,$wrap=FALSE)
	{
		$startYear = (int)(time_mysql($pc["CREATED"]) / 10000000000);
		$startMonth = (int)((time_mysql($pc["CREATED"])-$startYear*10000000000) / 100000000);	
		$thisYear = date("Y");
		$thisMonth = date("m");
		$i = 0;
		if ($wrap) echo "<ul>";
		for($yy=$thisYear ; $yy >= $startYear ; $yy --)
		{
			$firstMonth = ($yy == $startYear)?$startMonth:1;
			for($mm = $thisMonth ; $mm >= $firstMonth  ; $mm --)
			{
				if($wrap && $i==0) echo "<li>";
				echo "<a href=\"pcarch.php?userid=".$pc["USER"]."&y=".$yy."&m=".$mm."\" target=\"_blank\">".$yy."年".$mm."月</a>\n";
				if($wrap && $i==1) echo "</li>";
				$i = 1 - $i ;
			}
			$thisMonth = 12;	
		}
		if ($wrap && $i==1) echo "</li>";
		if ($wrap) echo "</ul>";
	}
	
	function get_calendar_array($link,$pc,$pur,&$totalnodes)
	{
?>
<script language="javascript">
var blogCalendarArray = new Array();
var blogNodeUrl = "pccon.php?id=<?php echo $pc["UID"]; ?>&s=all";
<?php	
		$query = "SELECT `nid` , `created` FROM nodes WHERE `uid` = '".$pc["UID"]."' AND `type` = 0 ";
		if($pur == 0)
			$query .= " AND `access` = 0 ";
		elseif($pur > 0)
			$query .= " AND ( `access` = 0 OR `access` = 1 ) ";
		//elseif($pur == 3)
		//	$query .= " AND ( `access` = 0 OR `access` = 1 OR `access` = 2 ) ";
		$query .= " ORDER BY `nid` DESC;";
		$result = mysql_query($query,$link);
		$totalnodes = mysql_num_rows($result); //所有日志数
		$bc = array();
		while($rows = mysql_fetch_array($result))
		{
			$dvlt = substr(time_mysql($rows["created"]),0,8);
			if(!@$bc[$dvlt])
			{
				$bc[$dvlt] = $rows["nid"] ;
				
?>
blogCalendarArray[<?php echo $dvlt; ?>] = <?php echo (int)($rows["nid"]); ?>;
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
	
	function pc_user_infor($pc)
	{
		if($pc["INFOR"]) echo $pc["INFOR"];	
	}
	
	function pc_load_nodes($link,$pc,$pur=0,$pno)
	{
		$start = $pc["INDEX"]["nodeNum"] * ( $pno - 1 );
		$query = "SELECT * FROM `nodes` WHERE `uid` = '".$pc["UID"]."' AND type = 0 ";
		if($pur == 0)
			$query .= " AND `access` = 0 ";
		elseif($pur > 0)
			$query .= " AND ( `access` = 0 OR `access` = 1 ) ";
		//elseif($pur == 3)
		//	$query .= " AND ( `access` = 0 OR `access` = 1 OR `access` = 2 ) ";
		$query .= " ORDER BY `nid` DESC LIMIT ".$start." , 10 ;";
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
	
	function display_nodes($link,$pc,$nodes,$tablewidth="100%",$tablestyle=0,$totalnodes,$pno)
	{
		global $pcconfig;
		for($i=0;$i<min($pc["INDEX"]["nodeNum"],count($nodes));$i++)
		{
			$contentcss = ($nodes[$i]["htmltag"])?"indexcontentwithhtml":"indexcontent";
			if($tablestyle==1)
			{
				$tableclass = "f1";
				if($i%2==1)
					$cellclass=array("t14","t14","t14");
				else
					$cellclass=array("t16","t16","t16");
			}
			else
			{
				$tableclass = "t15";
				if($i%2==1)
					$cellclass=array("t14","t11","t8");
				else
					$cellclass=array("t16","t13","t5");
			}
			echo "<center><table cellspacing=0 cellpadding=10 width=\"".$tablewidth."\" class=".$tableclass.">\n".
			"<tr><td class=\"".$cellclass[0]."\">".time_format_date($nodes[$i]["created"])."</td>".
			"<td class=\"".$cellclass[1]."\" align=right>[<a href=\"pccom.php?act=pst&nid=".$nodes[$i]["nid"]."\">评论</a>]\n".
			"[<a href=\"";
			if($pc["EMAIL"])
				echo "mailto:".$pc["EMAIL"];
			else
				echo "/bbspstmail.php?userid=".$pc["USER"]."&title=问候";
			echo "\">写信问候</a>]</td></tr>\n".
			"<tr><td class=\"".$cellclass[0]."\"><img src=\"icon/".$nodes[$i]["emote"].".gif\" border=0 align=absmiddle>\n".
			"<a href=\"pccon.php?id=".$pc["UID"]."&nid=".$nodes[$i]["nid"]."&s=all\" class=f2>".html_format($nodes[$i]["subject"])."</a></td>".
			"<td align=right class=\"".$cellclass[1]."\">&nbsp;</td>".
			"</tr>";
			if($pc["INDEX"]["nodeChars"]==0)
				echo "<tr><td colspan=2 class=\"".$cellclass[1]."\"><font class='".$contentcss."'>".
				     html_format($nodes[$i]["body"],TRUE,$nodes[$i]["htmltag"]).
				     "</font></td></tr>\n";
			else
			{
				echo "<tr><td colspan=2 class=\"".$cellclass[1]."\"><font class='".$contentcss."'>".
				     html_format(substr($nodes[$i]["body"],0,$pc["INDEX"]["nodeChars"])." ",TRUE,$nodes[$i]["htmltag"]); 
                        	if (strlen($nodes[$i]["body"])>$pc["INDEX"]["nodeChars"]) 
                        		echo " <br class=\"\" /> ......<br class=\"\" /><br class=\"\" />".
                        		     "<strong><A href=\"pccon.php?id=".$pc["UID"]."&nid=".$nodes[$i]["nid"]."&s=all\">>> 阅读全文</A></strong>".
                        		     "</font></td></tr>\n";; 
                        }
            
            if (pc_is_groupwork($pc) && $nodes[$i]["publisher"])
                $publisher = $nodes[$i]["publisher"];
            else
                $publisher = $pc["USER"];
                
                        echo "<tr><td colspan=2 class=\"".$cellclass[2]."\"><font class=\"f7\">\n&nbsp; <a href=\"/bbsqry.php?userid=".$publisher."\">".$publisher."</a> 发布于 ".time_format($nodes[$i]["created"]).
			"\n|\n浏览[".$nodes[$i]["visitcount"]."]".
			"\n|\n<a href=\"pccon.php?id=".$pc["UID"]."&nid=".$nodes[$i]["nid"]."&s=all\">评论[".$nodes[$i]["commentcount"]."]</a>";
			if($nodes[$i]["trackback"]) {
				echo "\n|\n<a href=\"javascript:openScript('pctb.php?nid=".$nodes[$i]["nid"]."&uid=".$pc["UID"]."&subject=".base64_encode($nodes[$i]["subject"])."' , 460, 480)\">引用[".$nodes[$i]["trackbackcount"]."]</a>";
			    echo "&nbsp;<a href=\"#\" onClick=\"javascript: holdtext.innerText = 'http://".$pcconfig["SITE"]."/pc/tb.php?id=".$nodes[$i]["nid"]."'; Copied = holdtext.createTextRange(); Copied.execCommand('Copy'); alert('该引用地址已经复制到剪贴板'); return false; \">Trackback Ping URL</a>";
			}
			echo "</font></td></tr>\n</table></center>\n";
		}
		echo "<center><table cellspacing=0 cellpadding=10 width=\"".$tablewidth."\" class=".$tableclass.">\n".
		     "<tr><td class=\"".$cellclass[2]."\" align=\"center\">";
		display_blog_page_tool($pc,$totalnodes,$pno);
		echo "</td></tr></table></center>";
	}
	
	function display_newnodes_list($link,$pc,$nodes)
	{
		echo "<ul>";
		for($i=0;$i< count($nodes);$i++)
		{
			echo "<li><a href=\"pccon.php?id=".$pc["UID"]."&nid=".$nodes[$i]["nid"]."\">".html_format($nodes[$i]["subject"])."</a>(".time_format($nodes[$i]["created"]).")</li>\n";
		}
		echo "</ul>";
		
?>
<p class="f1" style="text-align:right;">
<a href="pcdoc.php?userid=<?php echo $pc["USER"]; ?>">
更多
</a>&nbsp;&nbsp;&nbsp;&nbsp;
</p>
<?php
	}
	
	function display_blog_area_links($sec,$pc,$tags)
	{
		global $loginok,$currentuser,$pcconfig;
?>
	<table cellpadding=5 cellspacing=0 border=0 class=t1>
		<tr>
<?php
		$i = 0;
		while(list($secTag,$secTagValue) = each($tags))
		{
			if($i > 4 ) break;
			if(!$secTagValue) continue;
			echo "<td class=t11><a href=\"pcdoc.php?userid=".$pc["USER"]."&tag=".$secTag."\" class=\"f1\">".$sec[$secTag]."</a></td>\n";
			$i ++ ;
		}
?>
			<td class=t11>
			<a href="/bbsdoc.php?board=<?php echo $pcconfig["BOARD"]; ?>" class="f1">Blog论坛</a>
			</td>
			<td class=t11>
			<a href="pcsearch2.php" class="f1">Blog搜索</a>
			</td><td class=t11>
			<a href="index.html" class="f1">Blog首页</a>
			</td><td class=t11>
			<a target="_top" href="/frames.html" class="f1"><?php echo $pcconfig["BBSNAME"]; ?></a>
			</td>
		</tr>
	</table>
<?php
	}

	function display_blog_intro()
	{
		global $pc;
?>	
<table cellpadding=5 cellspacing=0 width="100%" border=0 class=t1>
			<tr><td align="left" class="t2">
			.: 欢迎访问<?php echo $pc["NAME"]; ?> :.
			</td></tr>
			<tr><td class="t8">
			<?php 
				if($pc["LOGO"])
					echo "<img alt=\"".$pc["DESC"]."\" hspace=\"5\" src=\"".$pc["LOGO"]."\" align=\"left\" vspace=\"5\" border=\"0\" />";
				echo $pc["DESC"]; 
			?>
	</td></tr>
</table>	
<?php		
	}
	
	function display_blog_page_tool($pc,$totalnodes,$pno)
	{
		if(!$pc["INDEX"]["nodeNum"])
			return;
		$totalpages =( ( $totalnodes - 1 ) / $pc["INDEX"]["nodeNum"] ) + 1 ;
		$totalpages = intval($totalpages);
		echo "[\n";
		for($i = 1 ; $i < $pno ; $i ++)
			echo "<a href=\"".$_SERVER["PHP_SELF"]."?id=".$pc["USER"]."&pno=".$i."\">".$i."</a>\n";
		echo "<strong>".$pno."</strong>\n";
		for($i = $pno + 1 ; $i < $totalpages + 1 ; $i ++)
			echo "<a href=\"".$_SERVER["PHP_SELF"]."?id=".$pc["USER"]."&pno=".$i."\">".$i."</a>\n";
		echo "]\n";
	}
		
	function display_blog_tools($pc,$pur)
	{
		global $loginok,$currentuser,$pcconfig;
?>
<table cellpadding=3 cellspacing=0 width="100%" border=0 class=t1>
			<form action="pcnsearch.php" method="get" onsubmit="if(this.keyword.value==''){alert('请输入关键字');return false;}">
			<input type="hidden" name="area" value="<?php echo $pc["USER"]; ?>">
			<tr><td align="left" class="t3">
			<strong>Blog搜索</strong>
			</td></tr>
			<tr><td class="t4">
			<input type="text" name="keyword" id="keyword" class="f1">
			<input type="submit" value="GO" class="f1">
			</td></tr>
			</form>
			<tr><td align="left" class="t3">
			<strong>登录</strong>
			</td></tr>
			<?php
				if($loginok==1 && strcmp($currentuser["userid"],"guest"))
				{
?>
<tr><td class="t4">
<strong><?php echo $currentuser["userid"]; ?></strong>
&nbsp;
[<a href="/bbslogout.php" target="_top" class="f1">注销</a>]<br/>
[<?php bbs_add_super_fav ('[BLOG] '.undo_html_format($pc['NAME']), '/pc/index.php?id='.$pc['USER']); ?>]
</td></tr>
<?php
				}
				else
				{
?>
<form action="/bbslogin.php?mainurl=/pc/index.php?id=<?php echo $pc["USER"]; ?>" method="post" name="form1" target="_top" >
<tr><td class="t4">
用户名:
<INPUT TYPE=text class="f1" size="12" onMouseOver="this.focus()" onFocus="this.select()" name="id" >
<br>
密&nbsp;&nbsp;码:
<INPUT TYPE=password  class="f1" size="12" name="passwd" maxlength="39">
<br>
<INPUT TYPE=submit value="Login" class="f1"></form>
</td></tr>
<?php					
				}
			?>
			
<?php
			if($pur == 3)
			{
?>			
			<tr><td align="left" class="t3">
			[
			<a href="pcmanage.php?userid=<?php echo $pc["USER"]; ?>&act=post&tag=0&pid=0">添加文章</a>
			]
			[
			<a href="pcdoc.php?userid=<?php echo $pc["USER"]; ?>&tag=7">参数设定</a>
			]
			</td></tr>
<?php
			}
?>
		</table>
<?php
	}
	
	function display_blog_list($pc,$blogs)
	{
		echo "<ul>";
		for($i=0;$i<( count($blogs));$i++)
		{
			echo "<li><a href=\"pcdoc.php?userid=".$pc["USER"]."&tag=0&tid=".$blogs[$i]["TID"]."\">".html_format($blogs[$i]["NAME"])."</a></li>\n";	
			
		}
		echo "</ul>";
	}
	
	function display_blog_calendar()
	{
?>
<span id='bc'></span>
<script language=javascript>
blogCalendar(<?php echo date("Y,n,j"); ?>);
</script>
<?php		
	}
	
	function display_blog_friend_links($pc,$wrap=FALSE)
	{
		if($wrap) echo "<ul>";
			for($i = 0 ; $i < count($pc["LINKS"]) ; $i ++)
			{
				if($wrap) echo "<li>";
				if($pc["LINKS"][$i]["IMAGE"])
					echo "<a href='http://".htmlspecialchars($pc["LINKS"][$i]["URL"])."'><img alt=\"".htmlspecialchars($pc["LINKS"][$i]["URL"])."\" src=\"http://".htmlspecialchars($pc["LINKS"][$i]["LINK"])."\" border=\"0\" align=\"absmiddle\"></a>\n";
				else
					echo "<a href='http://".htmlspecialchars($pc["LINKS"][$i]["URL"])."'>".htmlspecialchars($pc["LINKS"][$i]["LINK"])."</a>\n";
				if($wrap) echo "</li>";
			}
		if($wrap) echo "</ul>";
	}

	function display_blog_out_rss($pc)
	{
?>
<a href="rss.php?userid=<?php echo $pc["USER"]; ?>"><img src="images/xml.gif" align="absmiddle" alt="XML" border="0"></a>
<?php
	}

	function display_klip_out($pc)
	{
?>
<a href="klip.php?id=<?php echo $pc["USER"]; ?>"><img src="images/KlipFolio.gif" align="absmiddle" alt="Klip Folio" border="0"></a>
<?php
	}


	
	function display_trackback_links($link,$pc)
	{
		$query = "SELECT * FROM trackback WHERE uid = '".$pc["UID"]."' GROUP BY url ORDER BY tbid DESC LIMIT 0 , 10;";	
		$result = mysql_query($query,$link);
		echo "<ul>";
		while($rows = mysql_fetch_array($result))
		{
			echo "<li><a href=\"".htmlspecialchars(stripslashes($rows[url]))."\">".html_format($rows[title])."</a>"
				."\n(".time_format($rows[time]).")</li>";
		}
		echo "</ul>";
		mysql_free_result($result);
	}
	
	function display_new_comments($link,$pc,$pur=0)
	{
		$query = "SELECT cid , comments.subject , comments.created , comments.username FROM comments, nodes WHERE comments.nid = nodes.nid ";
		if($pur == 0)
			$query .= " AND access = 0 ";
		elseif($pur > 0)
			$query .= " AND ( access = 0 OR access = 1 ) ";
		//elseif($pur == 3)
		//	$query .= " AND ( `access` = 0 OR `access` = 1 OR `access` = 2 ) ";
		$query .= " AND comments.uid = ".$pc["UID"]." AND comment = 1 ORDER BY cid DESC LIMIT 0 , 10 ;";
		$result = mysql_query($query,$link);
		echo "<ul>";
		for($i = 0;$i < mysql_num_rows($result) ; $i++)
		{
			$rows = mysql_fetch_array($result);
			echo "<li>[<a href=\"/bbsqry.php?userid=".$rows["username"]."\">".$rows["username"]."</a>]<a href=\"pcshowcom.php?cid=".$rows["cid"]."\">".html_format($rows["subject"])."</a>(".time_format($rows["created"]).")</li>";
		}
		echo "</ul>";
		mysql_free_result($result);
	}
	
	
	function display_blog_earthsong($link,$pc,$sec,$nodes,$blogs,$pur,$tags,$pno,$totalnodes)
	{
		global $pcconfig;
?>
<center>
<tbody>
<table cellspacing=0 cellpadding=0 border=0 width=780 class="ll">

	<tr><td colspan=2 height=167><img src="style/earthsong/es_r1_c1.jpg" border=0></td></tr>
	<tr>
	<td width=414 height=232><img src="style/earthsong/es_r2_c1.jpg" border=0></td>
	<td width=366 height=232><img src="style/earthsong/es_r2_c2.jpg" border=0></td>
	</tr>
	<tr>
	<td colspan=2 class="blogarea"><center>
	<table cellspacing=0 cellpadding=0 border=0 class=f1>
	<tr><td class="blogarea" align="center">
	<?php echo $pc["NAME"]; ?>&nbsp;&gt;&gt;
	</td><td class="blogarea">
	<?php display_blog_area_links($sec,$pc,$tags); ?></td>
	</tr>
	</table></center></td></tr>
</table>
</tbody>
<tbody>
<table cellspacing=0 cellpadding=5 border=0 width=780 class="ll">

	<tr>
		<td class="bloglists"><?php pc_user_infor($pc); ?></td>
	</tr>
</table>
</tbody>
<tbody>
<table cellspacing=0 cellpadding=5 border=0 width=780 class="ll">
	<tr>
		<td width="40%" class="bloglists">
			<table cellpadding=3 cellspacing=0 width="100%" border=0 class=t1>
			<tr><td align="center" class="t8">
			:+: 最近更新文章 :+:
			</td></tr>
			<tr><td class="t5">
			<?php display_newnodes_list($link,$pc,$nodes); ?>
			</td></tr></table>
		</td>
		<td width="35%" class="bloglists">
			<table cellpadding=3 cellspacing=0 width="100%" border=0 class=t1>
			<tr><td align="center" class="t8">
			:+: 最近收到的评论 :+:
			</td></tr>
			<tr><td class="t5">
			<?php display_new_comments($link,$pc,$pur); ?>
			</td></tr></table>
		</td>
		<td width="25%" class="bloglists">
			<table cellpadding=3 cellspacing=0 width="100%" border=0 class=t1>
			<tr><td align="center" class="t8">
			:+: 日历 :+:
			</td></tr>
			<tr><td class="t5" align="center">
			<?php display_blog_calendar(); ?>
			</td></tr></table>
		</td>
	</tr>
</table>
</tbody>
<tbody>
<?php		
		display_nodes($link,$pc,$nodes,780,0,$totalnodes,$pno);
?>
</tbody>
<tbody>
<table cellspacing=0 cellpadding=5 border=0 width=780 class="ll">
	<tr>
		<td width="40%" class="bloglists">
			<table cellpadding=3 cellspacing=0 width="100%" border=0 class=t1>
			<tr><td align="center" class="t8">
			:+: 栏目分类 :+:
			</td></tr>
			<tr><td class="t5">
			<?php display_blog_list($pc,$blogs); ?>
			</td></tr></table>
		</td>
		<td width="35%" class="bloglists">
			<table cellpadding=3 cellspacing=0 width="100%" border=0 class=t1>
			<tr><td align="center" class="t8">
			:+: 最近收到的引用通告 :+:
			</td></tr>
			<tr><td class="t5">
			<?php display_trackback_links($link,$pc); ?>
			</td></tr></table>
		</td>
		<td width="25%" class="bloglists">
			<table cellpadding=3 cellspacing=0 width="100%" border=0 class=t1>
			<tr><td align="center" class="t8">
			:+: 工具箱 :+:
			</td></tr>
			<tr><td class="t5" align="center">
			<?php display_blog_tools($pc,$pur); ?>
			</td></tr></table>
		</td>
	</tr>
</table>
</tbody>
<tbody>
<table cellspacing=0 cellpadding=5 border=0 width=780 class="ll">
	<tr>
		<td class="bloglists2" id="bloglists2">
		<strong>每月档案 &gt;&gt; </strong><?php pc_get_archfile($pc); ?>
		</td>
	</tr>
</table>
</tbody>
<tbody>
<table cellspacing=0 cellpadding=5 border=0 width=780 class="ll">
	<tr>
		<td class="bloglists2">
		<strong>友情链接 &gt;&gt; <?php display_blog_friend_links($pc); ?>&nbsp;
		</td>
	</tr>
</table>
</tbody>
<tbody>
<table cellpadding=3 cellspacing=0 width=780 border=0 class=f1>
		<tr><td class="t14" align=center>
	<?php display_blog_out_rss($pc); ?>
	<?php display_klip_out($pc); ?>
<br />
	访问量 
	<font class="f2">
	<?php echo $pc["VISIT"]; ?>
	</font>
	&nbsp;&nbsp;&nbsp;&nbsp;
	更新时间:
	<?php echo time_format($pc["MODIFY"]); ?>
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
<br /><span class="copystyle">&copy;</span>All Rights Reserved
&nbsp;&nbsp;
<?php echo pc_personal_domainname($pc["USER"]); ?>
</td></tr>
</table>
</tbody>
<tbody>
<table cellspacing=0 cellpadding=0 border=0 width=780>
	<tr><td height=186><img src="style/earthsong/es_r3_c1.jpg" border=0></td></tr>
</table>
</tbody>
</center>
<?php		
		
	}
	
	function display_blog_smth($link,$pc,$sec,$nodes,$blogs,$pur,$tags,$pno,$totalnodes)
	{
		global $pcconfig;
?>	
<table cellspacing=0 cellpadding=0 width=100% border=0 class=f1>
	<tr><td colspan=2 bgcolor="#718BD6" height="5" align=right>
	<?php display_blog_area_links($sec,$pc,$tags); ?>
	</td></tr>
	
	<tr>
		<td colspan=2 bgcolor="#BCCAF2" style="border-width: 2px;border-top-style: solid;border-color: #999999;"><img src="<?php echo $pc["LOGO"]?$pc["LOGO"]:"style/smth/topimg.gif"; ?>" border=0 alt="Welcome to <?php echo $pc["USER"]; ?>'s Blog" align=absmiddle></td>
	</tr>
	
	
	<tr>
		<td colspan=2 valign=middle bgcolor="#F6F6F6" style="border-bottom-width: 1px;border-top-width: 2px;border-top-style: solid;border-bottom-style: dashed;border-color: #718BD6;" height="30">
		&nbsp;&nbsp;
		<?php echo $pc["DESC"]?$pc["DESC"]:"惠风荡繁囿,白云屯曾阿,景昃鸣禽集,水木湛清华"; ?>
		</td>
	</tr>
	
	<tr>
		
		<td width="25%" align="middle" valign="top" style="border-width: 1px;border-right-style: dashed;border-color: #336699;">
			<table width="98%" cellpadding=5 cellspacing=0 border=0>
				
				<tr><td class=t17>
				&gt;&gt; 日历
				</td></tr>
				
				
				<tr>
					<td align=middle class=t14><?php display_blog_calendar(); ?></td>
				</tr>
				<tr><td class=t17>
				&gt;&gt; 最近更新文章
				</td></tr>
				<tr>
					<td align=middle class=t14>
					<table cellspacing=0 cellpadding=3 width=100% border=0 style="line-height:20px;font-size:12px"><tr><td>
					<?php display_newnodes_list($link,$pc,$nodes); ?>
					</td></tr></table>
					</td>
				</tr>
				
				
				<tr><td class=t17>
				&gt;&gt; 栏目分类
				</td></tr>
				<tr>
					<td align=middle class=t14>
					<table cellspacing=0 cellpadding=3 width=100% border=0 style="line-height:20px;font-size:12px"><tr><td>
					<?php display_blog_list($pc,$blogs); ?>
					</td></tr></table>
					</td>
				</tr>
				
				
				<tr><td class=t17>
				&gt;&gt; 每月档案
				</td></tr>
				<tr>
					<td align=middle class=t14>
					<table cellspacing=0 cellpadding=3 width=100% border=0 style="line-height:20px;font-size:12px"><tr><td>
					<?php pc_get_archfile($pc,TRUE); ?>
					</td></tr></table>
					</td>
				</tr>
				
				
				<tr><td class=t17>
				&gt;&gt; 最近收到的评论
				</td></tr>
				<tr>
					<td align=middle class=t14>
					<table cellspacing=0 cellpadding=3 width=100% border=0 style="line-height:20px;font-size:12px"><tr><td>
					<?php display_new_comments($link,$pc,$pur); ?>
					</td></tr></table>
					</td>
				</tr>
				
				
				<tr><td>
				<?php display_blog_tools($pc,$pur); ?>
				</tr></tr>
				<tr><td class=t17>
				&gt;&gt; 友情链接
				</td></tr>
				<tr>
					<td class=t14><font class=f1>
					<?php display_blog_friend_links($pc,TRUE); ?>
					</font></td>
				</tr>
				
				
				<tr><td class=t17>
				&gt;&gt; 最近收到的引用通告
				</td></tr>
				<tr>
					<td class=t14><font class=f1>
					<?php display_trackback_links($link,$pc); ?>
					</font></td>
				</tr>
				<tr><td class=t17>
				&gt;&gt; 访问人数
				</td></tr>
				<tr><td style="text-align:center;color:#FF6600;font-weight:bolder;background-color:#F6F6F6;font-family: Verdana, Arial, Helvetica, sans-serif;font-size: 14px;font-style: italic;line-height: 22px;">
				<?php echo $pc["VISIT"]; ?>
				</td></tr>
				
				<tr><td class=t17><?php pc_user_infor($pc); ?></td></tr>
				<tr><td class=t17><center>
				<?php display_blog_out_rss($pc); ?>
				<?php display_klip_out($pc); ?></center>
				</td></tr>
			</table>
		</td>
		
		<td width="80%" align="right" valign="top">
		<?php display_nodes($link,$pc,$nodes,"99%",1,$totalnodes,$pno); ?>
		<p align=center class=f1>
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
		<br />
	<span style="text-align:center;display:block;">更新时间: <?php echo time_format($pc["MODIFY"]); ?>
		&nbsp;
		<span class="copystyle">&copy;</span>All Rights Reserved
		&nbsp;&nbsp;
		<?php echo pc_personal_domainname($pc["USER"]); ?></span>
		</p>
		</td>
	</tr>
</table>	
<?php		
	}
	
	function display_blog_simple($link,$pc,$sec,$nodes,$blogs,$pur,$tags,$pno,$totalnodes)
	{
	    global $loginok,$currentuser,$pcconfig;
	    
?>
<center>
<table width="780" cellpadding="0" cellspacing="0">
<tbody>
<tr>
    <td>
        <table width="100%" cellspacing="0" cellpadding="3">
            <tr>
                <td align="right">
                <font style="font size: 16px;color:#333333;font-weight:bold">:+:&nbsp;&nbsp;<?php echo $pc['NAME']; ?>&nbsp;&nbsp;:+:</font>
                </td>
                <td width="150">&nbsp;</td>
            </tr>
            <tr><td colspan="2" align="center">
<?php
    if ($pc['LOGO'])
        echo '<img src="'.$pc['LOGO'].'" border="0" alt="'.$pc['DESC'].'" />';
    else
        echo '&nbsp;';
?>
            </td></tr>
            <tr><td colspan="2" height="5"> </td></tr>
            <tr><td colspan="2" align="center">
<?php
            $i = 0;
            foreach ($blogs as $blog) {
                if ($i!=0) echo '&nbsp;|&nbsp;';
                echo '<a href="/pc/pcdoc.php?userid='.$pc['USER'].'&tag=0&tid='.$blog['TID'].'">'.html_format($blog['NAME']).'</a>';
                $i ++;    
            }
?>            
            </td></tr>
            <tr><td colspan="2" height="5"> </td></tr>
        </table>
    </td>
</tr>
</tbody>
</table>
<?php	        
	    
	}
	
		
	function display_blog_default($link,$pc,$sec,$nodes,$blogs,$pur,$tags,$pno,$totalnodes)
	{
		global $loginok,$currentuser,$pcconfig;
		
?>
<center>
<tbody>
<table cellpadding=0 cellspacing=0 width=700 border=0 class=f1>
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
</tbody>
<tbody>
<table cellpadding=0 cellspacing=0 width=700 border=0 class=f1>
<tr>
	<td align="center">
	<?php display_blog_area_links($sec,$pc,$tags); ?>
	</td>
</tr>
</table>
</tbody>
<tbody>
<table cellpadding=10 cellspacing=0 width=700 border=0 class=t1>

	<tr>
		<td align="center" class="t11" colspan="2">
		<?php display_blog_intro(); ?>
		</td>
	</tr>
	
	<tr>
		<td class=t11 colspan=2><?php pc_user_infor($pc); ?></td>
	</tr>
	
	<tr>
		<td align="center" class="t14" width="400" valign="TOP">
		<table cellpadding=3 cellspacing=0 width="100%" border=0 class=t1>
<tr><td align="left" class="t2">
.: 最近更新文章 :.
</td></tr>
<tr><td class="t8">
		<?php display_newnodes_list($link,$pc,$nodes); ?>
		</td></tr></table>
		</td>
		<td align="center" class="t11" valign="TOP">
		<?php display_blog_tools($pc,$pur); ?>
		</td>
	</tr>
	<tr>
		<td align="center" class="t10" valign="top">
		<table cellpadding=3 cellspacing=0 width="100%" border=0 class=t1>
			<tr><td class=t2>
			.: 栏目分类 :.
			</td></tr>
			<tr><td class=t5 align=left>
		<?php display_blog_list($pc,$blogs); ?>
		</td></tr></table>
		</td>
		<td align="center" class="t3" valign="top">
		<?php display_blog_calendar(); ?>
		</td>
	</tr>
</table>
</tbody><tbody>
<?php		
		display_nodes($link,$pc,$nodes,700,0,$totalnodes,$pno);
?>
</tbody>
<tbody>
<table cellpadding=10 cellspacing=0 width=700 border=0 class=t15>
<tr>
<td align="center" class="t14" width="50%" valign="top">
<table cellpadding=3 cellspacing=0 width=100% border=0 class=t1>
		<tr>
		<td class="t2">
		.: 最近收到的评论 :. 
		</td></tr>
		<tr><td class="t8">
<?php display_new_comments($link,$pc,$pur); ?>
</td></tr></table>
</td>
<td align="center" class="t11" width="50%" valign="top">
<table cellpadding=3 cellspacing=0 width=100% border=0 class=t1>
		<tr>
		<td class="t2">
		.: 最近收到的引用通告 :. 
		</td></tr>
		<tr><td class="t8">
<?php display_trackback_links($link,$pc); ?>
</td></tr></table>
</td></tr>
<tr><td colspan="2" align="center" class="t11">
<table cellpadding=3 cellspacing=0 width=100% border=0 class=f1>
<tr><td class=f1>
<strong>每月档案&gt;&gt;</strong>
<?php pc_get_archfile($pc); ?>
</td></tr></table>
</td></tr>
<tr><td colspan="2" align="center" class="t11">
<table cellpadding=3 cellspacing=0 width=100% border=0 class=t1>
		<tr><td class="t2">
		.: 友情链接 :. 
		</td></tr>
		<tr><td class="t4">
	<?php display_blog_friend_links($pc); ?>
	<br /><br />
	<?php display_blog_out_rss($pc); ?>
	<?php display_klip_out($pc); ?>
		</td></tr></table>
</td></tr>
<tr><td class="t3" colspan="2">
	访问量 
	<font class="f4">
	<?php echo $pc["VISIT"]; ?>
	</font>
	&nbsp;&nbsp;&nbsp;&nbsp;
	更新时间:
	<?php echo time_format($pc["MODIFY"]); ?>
	<br />
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
	<br />
<span class="copystyle">&copy;</span>All Rights Reserved
&nbsp;&nbsp;
<?php echo pc_personal_domainname($pc["USER"]); ?>
</td></tr>
</table></tbody></center>	
<?php		
	}

	$userid = addslashes($_GET["id"]);
	$uid = intval($_GET["id"]);
	if(isset($_GET["pno"]))
		$pno = intval($_GET["pno"]);
	else
		$pno = 0;
	if($pno < 1) $pno = 1;
	
	$link = pc_db_connect();
	$pc = pc_load_infor($link,$userid,$uid);
	if(!$pc)
	{
		pc_db_close($link);
		header("HTTP/1.1 404 NOT FOUND");
		html_init("gb2312",$pcconfig["BBSNAME"]."Blog");		
		html_error_quit("对不起，您要查看的Blog不存在");
		exit();
	}
	if($pc["STYLE"]["SID"] == 9)
		header("Location: indexxml.php?id=".$pc["USER"]);//自定义模板
	
	$userPermission = pc_get_user_permission($currentuser,$pc);
	$sec = $userPermission["sec"];
	$pur = $userPermission["pur"];
	$tags = $userPermission["tags"];
	
	$nodes = pc_load_nodes($link,$pc,$pur,$pno);
	$blogs = pc_blog_menu($link,$pc,0);
	
	if(!($pur == 3 && !pc_is_groupwork($pc)))//Blog所有者的访问不进行计数  windinsn dec 10,2003
		pc_counter($link);
	
	//if( pc_cache( $pc["MODIFY"] ) )
	//	return;
	pc_html_init("gb2312",$pc["NAME"],"",$pc["STYLE"]["CSSFILE"],$pc["BKIMG"]);
	$totalnodes = 0;
	get_calendar_array($link,$pc,$pur,$totalnodes);
?>
<script src="bc.js"></script>
<?php
	if (function_exists($pc["STYLE"]["INDEXFUNC"]))
	    $pc["STYLE"]["INDEXFUNC"]($link,$pc,$sec,$nodes,$blogs,$pur,$tags,$pno,$totalnodes);
	pc_db_close($link);
?>
