<?php
	/*
	** this file display article  in personal corp.
	** @id:windinsn  Nov 19,2003
	*/
	@session_start();
	$needlogin=0;
	require("pcfuncs.php");
	
	function pc_add_new_comment($pc,$nid,$alert)
	{
?>
<center>
<form name="postform" action="pccom.php?act=add&nid=<?php echo $nid; ?>" method="post" onsubmit="if(this.subject.value==''){alert('请输入评论主题!');return false;}">
<table cellspacing="0" cellpadding="5" width="500" border="0" class="t1">
<tr>
	<td class="t5"><strong>发表评论 </strong>
	<?php if($alert){ ?>
	<font class=f4>
	注意：仅有本站登录用户才能发表评论[<a href="/">点击登录</a>]。
	</font>
	<?php } ?>
	</td>
</tr>
<tr>
	<td class="t8">
	主题
	<input type="text" name="subject" maxlength="200" size="60" class="f1">
	</td>
</tr>
<tr>
	<td class="t13">心情符号</td>
</tr>
<tr>
	<td class="t5"><?php @require("emote.html"); ?></td>
</tr>
<tr>
	<td class="t11">内容
	<input type="hidden" name="htmltag" value=0>
	</td>
</tr>
<tr>
	<td class="t8"><textarea name="blogbody" class="f1" cols="60" rows="10" id="blogbody"  onkeydown='if(event.keyCode==87 && event.ctrlKey) {document.postform.submit(); return false;}'  onkeypress='if(event.keyCode==10) return document.postform.submit()' wrap="physical"></textarea></td>
</tr>
<tr>
	<td class="t5">
	<input type="submit" value="发表评论" class="f1">
	<input type="button" value="返回上页" class="f1" onclick="history.go(-1)">
	<input type="button" value="使用HTML编辑器" class="f1" onclick="window.location.href='pccom.php?act=pst&nid=<?php echo $nid; ?>';">
</tr>
</table>
</form></center>
<?php		
	}
	
	function pc_node_counter($link,$nid)
	{
		$query = "UPDATE nodes SET visitcount = visitcount + 1 , changed  = changed  WHERE `nid` = '".$nid."' ;";
		mysql_query($query,$link);
	}
	
	function display_navigation_bar($link,$pc,$nid,$pid,$tag,$spr,$order,$comment,$tid=0,$pur,$trackback , $subject)
	{
		$query = "SELECT `nid` FROM nodes WHERE `nid` < ".$nid." AND `uid` = '".$pc["UID"]."' AND `pid` = '".$pid."' AND `access` = '".$tag."' AND `tid` = '".$tid."' AND `type` != '1' ORDER BY `nid` DESC LIMIT 0 , 1 ;";
		$result = mysql_query($query,$link);
		$rows = mysql_fetch_array($result);
		if($rows)
			echo " <a href=\"pccon.php?id=".$pc["UID"]."&nid=".$rows[nid]."&pid=".$pid."&tag=".$tag."&tid=".$tid."\">上一篇</a>\n";
		else
			echo " 上一篇\n";
		mysql_free_result($result);
		$query = "SELECT `nid` FROM nodes WHERE `nid` > ".$nid." AND `uid` = '".$pc["UID"]."' AND `pid` = '".$pid."' AND `access` = '".$tag."' AND `tid` = '".$tid."' AND `type` != '1' ORDER BY `nid` ASC LIMIT 0 , 1 ;";
		$result = mysql_query($query,$link);
		$rows = mysql_fetch_array($result);
		if($rows)
			echo " <a href=\"pccon.php?id=".$pc["UID"]."&nid=".$rows[nid]."&pid=".$pid."&tag=".$tag."&tid=".$tid."\">下一篇</a>\n";
		else
			echo " 下一篇\n";
		mysql_free_result($result);
		
		if($comment != 0)
		{
			if($spr)
				echo "<a href=\"pccon.php?id=".$pc["UID"]."&nid=".$nid."\">仅显示评论主题</a>\n";
			else
				echo "<a href=\"pccon.php?id=".$pc["UID"]."&nid=".$nid."&s=all\">展开所有评论</a>\n";
			echo "<a href=\"pccom.php?act=pst&nid=".$nid."\">发表评论</a>\n";
		}
		if($pur == 3)
			echo "<a href=\"pcmanage.php?act=edit&nid=".$nid."\">修改</a>\n";
		if($trackback)
			echo 	"<a href=\"javascript:openScript('pctb.php?nid=".$nid."&uid=".$pc["UID"]."&subject=".base64_encode($subject)."',460 , 480)\">引用</a>\n";
		echo 	"<a href=\"/bbspstmail.php?userid=".$pc["USER"]."&title=问候\">写信问候</a>\n".
			//"<a href=\"pccon.php?id=".$id."&nid=".$nid."\">转寄</a>\n".
			//"<a href=\"pccon.php?id=".$id."&nid=".$nid."\">转载</a>\n".
			"<a href=\"pcdoc.php?userid=".$pc["USER"]."&pid=".$pid."&tag=".$tag."&order=".$order."&tid=".$tid."\">返回目录</a>\n".
			"<a href=\"javascript:history.go(-1);\">快速返回</a>\n";
	}
	
	function display_pc_comments($link,$uid,$nid,$spr)
	{
		global $pc;
		global $currentuser;
		
		if(strtolower($pc["USER"]) == strtolower($currentuser["userid"]))
			$perm = TRUE;
		else
			$perm = FALSE;
		
		if($spr)
			$query = "SELECT * FROM comments WHERE `nid` = '".$nid."' AND `uid` = '".$uid."' ORDER BY `cid` ASC ;";
		else
			$query = "SELECT `username` , `emote` , `subject` , `created`,`cid` FROM comments WHERE `nid` = '".$nid."' AND `uid` = '".$uid."' ORDER BY `cid` ASC ;";
		
		$result = mysql_query($query,$link);
		$re_num = mysql_num_rows($result);
?>
<table cellspacing="0" cellpadding="3" border="0" width="90%" class="t1">
<tr>
	<td class="t9" colspan="2">共有 <?php echo $re_num; ?> 条评论</td>
</tr>
<?php
		for($i = 0;$i < $re_num ;$i++)
		{
			$contentcss = ($rows[htmltag])?"contentwithhtml":"content";
			if($i%2==0)
				$tdclass= array("t8","t10","t11");
			else
				$tdclass= array("t5","t12","t13");
			$rows = mysql_fetch_array($result);
			echo "<tr>\n<td class=\"".$tdclass[1]."\">&nbsp;".
				"<img src=\"icon/".$rows[emote].".gif\" border=\"0\" align=\"absmiddle\">\n".
				"<a href=\"pcshowcom.php?cid=".$rows[cid]."\">".
				html_format($rows[subject]).
				"</a>".
				"[<a href=\"/bbsqry.php?userid=".$rows[username]."\">".$rows[username]."</a> 于 ".time_format($rows[created])." 提到]\n";
			if($perm || strtolower($rows[username]) == strtolower($currentuser["userid"]))
				echo "[<a href=\"#\" onclick=\"bbsconfirm('pceditcom.php?act=del&cid=".$rows[cid]."','确认删除?')\">删</a>]\n";
			if(strtolower($rows[username]) == strtolower($currentuser["userid"]))
				echo "[<a href=\"pceditcom.php?act=edit&cid=".$rows[cid]."\">改</a>]\n";
			echo "</td><td width=\"100\" align=\"right\" class=\"".$tdclass[0]."\"><font class=\"f4\">".($i+1)."</font>&nbsp;&nbsp;</td>\n</tr>\n";
			if($spr)
			{
				echo "<tr>\n<td colspan='2' class=\"".$tdclass[2]."\"><font class='".$contentcss."'>".
					html_format($rows[body],TRUE,$rows[htmltag])."</font></td>\n</tr>\n".
					"<tr>\n<td colspan='2' align='right' class=\"".$tdclass[0]."\">[FROM: ".$rows[hostname]."]".
					"</td>\n</tr>\n";
			}	
		}
?>
</table>
<?php		
		mysql_free_result($result);
		return $re_num;
	}
	
	$id = (int)($_GET["id"]);
	$nid = (int)($_GET["nid"]);
	$pid = (int)($_GET["pid"]);
	$tag = (int)($_GET["tag"]);
	if($_GET["s"]=="all")
		$spr = TRUE;
	else
		$spr = FALSE;
	
	$link = pc_db_connect();
	$pc = pc_load_infor($link,"",$id);
	if(!$pc)
	{
		pc_db_close($link);
		html_init("gb2312","Blog");		
		html_error_quit("对不起，您要查看的Blog不存在");
		exit();
	}
	
	pc_html_init("gb2312",$pc["NAME"],"","",$pc["BKIMG"]);
	
	if(pc_is_admin($currentuser,$pc) && $loginok == 1)
		$pur = 3;
	elseif(pc_is_friend($currentuser["userid"],$pc["USER"]) || bbs_is_bm($pcconfig["BRDNUM"], $currentuser["index"]))
		$pur = 1;
	else
		$pur = 0;
		
	$query = "SELECT * FROM nodes WHERE `nid` = '".$nid."' AND `uid` = '".$id."' LIMIT 0 , 1 ;";
	$result = mysql_query($query,$link);
	$rows = mysql_fetch_array($result);
	mysql_free_result($result);
	
	if(!$rows)
	{
		html_error_quit("对不起，您要查看的个人文章不存在");
		exit();
	}
	if( $rows[access] == 1 && $pur < 1)
	{
		html_error_quit("对不起，只有好友列表中的用户才能查看好友区文章!");
		exit();
	}
	if( $rows[access] > 1 && $pur < 2 )
	{
		html_error_quit("对不起，Blog所有者才能查看该文章!");
		exit();
	}
	$nid = $rows[nid];
	$tid = $rows[tid];
	
	if(!session_is_registered("readnodes"))
	{
		$readnodes = ",".$nid.",";
		session_register("readnodes");
		pc_node_counter($link,$nid);
		$rows[visitcount] ++;
	}
	elseif(!stristr($_SESSION["readnodes"],",".$nid.","))
	{
		$_SESSION["readnodes"] .= $nid.",";
		pc_node_counter($link,$nid);
		$rows[visitcount] ++;
	}
?>
<a name="top"></a>
<table cellspacing="0" cellpadding="0" border="0" width="100%">
<tr>
	<td>
	<table cellspacing="0" cellpadding="3" border="0" width="100%" class="tt1">
		<tr>
			<td>&nbsp;&nbsp;&nbsp;&nbsp;<?php echo "<a href=\"/\" class=f1>".BBS_FULL_NAME."</a> - <a href='pc.php' class=f1>Blog</a> - <a href=\"pcdoc.php?userid=".$pc["USER"]."&tag=".$rows[access]."&pid=".$pid."\" class=f1>".$pc["NAME"]."</a>"; ?></td>
			<td align="right">http://<?php echo $pc["USER"].$pcconfig["DOMAIN"]; ?>&nbsp;&nbsp;&nbsp;&nbsp;</td>
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
		<td align="right">[主题:<?php echo $pc["THEM"]; ?>]&nbsp;</td>
	</tr>
	</table>
	</td>
</tr>
<tr>
	<td align="center">
	<table cellspacing="0" cellpadding="5" border="0" width="90%" class="t1">
	<tr>
		<td colspan="2" class="t9">
		<img src="icon/<?php echo $rows[emote]; ?>.gif" border="0" align="absmiddle">
		<?php echo html_format($rows[subject]); ?></td>
	</tr>
	<tr>
		<td width="20%" align="left" valign="top" class="t8">
		作者：<?php echo "<a href=\"/bbsqry.php?userid=".$pc["USER"]."\">".$pc["USER"]."</a>"; ?><br/>
		发表时间：<br/>
		<?php echo time_format($rows[created]); ?><br/>
		更新时间：<br/>
		<?php echo time_format($rows[changed]); ?><br/>
		浏览：<?php echo $rows[visitcount]; ?>次<br>
		<?php
			if($rows[comment]==0)
				echo "锁定主题<br>";
			else
				echo "评论：".$rows[commentcount]."篇<br>";
			
			if($rows[trackback])
				echo "引用：".$rows[trackbackcount]."次<br/>";
		?>
		地址：<?php echo $rows[hostname]; ?>
		</td>
		<td width="80%" height="300" align="left" valign="top" class="t5">
		<font class="<?php echo ($rows[htmltag])?"contentwithhtml":"content"; ?>">
		<?php echo html_format($rows[body],TRUE,$rows[htmltag]); ?>&nbsp;
		</font>
		</td>
	</tr>
	<tr>
		<td colspan="2" align="right" class="t8">
		<?php display_navigation_bar($link,$pc,$nid,$rows[pid],$rows[access],$spr,addslashes($_GET["order"]),$rows[comment],$tid,$pur,$rows[trackback],$rows[subject]); ?>
		</td>
	</tr>
	</table>
	</td>
</tr>
<?php
		if($rows[comment]!=0)
		{
?>
<tr>
	<td align="center"><?php $re_num = display_pc_comments($link,$rows[uid],$rows[nid],$spr); ?></td>
</tr>
<?php
		}
?>
<tr>
	<td align="middle" class="f1" height="40" valign="middle">
	<?php
		if($re_num != 0)
			display_navigation_bar($link,$pc,$nid,$rows[pid],$rows[access],$spr,addslashes($_GET["order"]),$rows[comment],$tid,$pur,$rows[trackback],$rows[subject]); 
	?>
	&nbsp;</td>
</tr>
<tr>
	<td>
	<?php 
		if($rows[comment] && $rows[type] == 0)
		{
			$alert = ($loginok != 1 || !strcmp($currentuser["userid"],"guest"))?TRUE:FALSE;
			pc_add_new_comment($pc,$nid,$alert); 
		}
	?>
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
	</td>
</tr>
</table>
<?php
	pc_db_close($link);
	html_normal_quit();
?>