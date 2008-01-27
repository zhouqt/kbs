<?php
	/*
	** this file display article  in personal corp.
	** @id:windinsn  Nov 19,2003
	*/
	require("pcfuncs.php");
	
	function pc_add_new_comment($pc,$nid,$alert)
	{
		global $currentuser;
?>
<center>
<table cellspacing="0" cellpadding="5" width="500" border="0" class="t1">
<tr>
	<td class="t5"><strong>发表评论 </strong>
<?php
	if($alert && !$pc["ANONYCOMMENT"]) {
?>
	<font class=f4>
	注意：仅有本站登录用户才能发表评论<br />
	<?php bbs_login_form(); ?>
	</font>
	<?php } ?>
	</td>
</tr>
<?php
		if($alert && !$pc["ANONYCOMMENT"]) {
			echo '</table><br/><br/>';
			return;
		}
?>
<form name="postform" action="pccom.php?act=add&nid=<?php echo $nid; ?>" method="post" onsubmit="return submitwithcopy();">
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
<?php
		if(strtolower($currentuser["userid"]) == "guest")
			print("<br><span style=\"color:#FF0000\">您没有登录，现在是匿名评论，一旦发表将无法修改。</span>");
?>
	<input type="hidden" name="htmltag" value=0>
	</td>
</tr>
<tr>
	<td class="t8"><textarea name="blogbody" class="f1" cols="60" rows="10" id="blogbody" wrap="physical"></textarea></td>
</tr>
<tr>
	<td class="t5">
	<input type="submit" name="postbutton" id="postbutton" value="发表评论" class="f1">
	<input type="button" value="返回上页" class="f1" onclick="history.go(-1)">
	<input type="button" value="使用HTML编辑器" class="f1" onclick="window.location.href='pccom.php?act=pst&nid=<?php echo $nid; ?>';">
</tr>
</form>
</table>
</center>
<?php		
	}
	
	function display_navigation_bar($link,$pc,$nid,$pid,$tag,$spr,$order,$comment,$tid=0,$pur,$trackback , $subject , $recommend , $nodetype , $username , $state)
	{
		global $currentuser,$loginok,$pcconfig;
			
    	if ($pc['USER'] == '_filter') {
		    if ($state == 0)
		    echo "<a href=\"pcadmin_flt.php?fid=".$nid."&filter=n\">通过</a>\n".
		         "<a href=\"pcadmin_flt.php?fid=".$nid."&filter=y\">不通过</a>\n";
		    echo "<a href=\"/bbspstmail.php?userid=".$username;
		}
		else {
    		echo " <a href=\"pccon.php?id=".$pc["UID"]."&nid=".$nid."&pid=".$pid."&tag=".$tag."&tid=".$tid."&p=p\">上一篇</a>\n";
    	    echo " <a href=\"pccon.php?id=".$pc["UID"]."&nid=".$nid."&pid=".$pid."&tag=".$tag."&tid=".$tid."&p=n\">下一篇</a>\n";
    	
    		if($comment != 0)
    		{
    			if($spr)
    				echo "<a href=\"pccon.php?id=".$pc["UID"]."&nid=".$nid."\">仅显示评论主题</a>\n";
    			else
    				echo "<a href=\"pccon.php?id=".$pc["UID"]."&nid=".$nid."&s=all\">展开所有评论</a>\n";
    			echo "<a href=\"pccom.php?act=pst&nid=".$nid."\">发表评论</a>\n";
    		}
    		if($trackback && $tag == 0)
    			echo "<a href=\"/pc/pcmanage.php?userid=".$currentuser["userid"]."&act=post&tag=0&tbArtAddr=".urlencode("http://".$pcconfig["SITE"]."/pc/pccon.php?id=".$pc["UID"]."&nid=".$nid."&s=all")."&tbTBP=".urlencode("http://".$pcconfig["SITE"]."/pc/tb.php?id=".$nid)."\" target=\"_blank\"><font color=red>拿该日志来写BLOG</font></a>\n";
    		if($pur == 3)
    			echo "<a href=\"pcmanage.php?userid=".$pc["USER"]."&act=edit&nid=".$nid."\">修改</a>\n";
    		if($recommend == 0 && $tag == 0)
    			echo "<a href=\"pcrec.php?nid=".$nid."\">推荐</a>\n";
    		if($tag==0)
    			echo "<a href=\"pcfwd.php?nid=".$nid."\">转载</a>\n";
    		if($trackback && $tag == 0)
    			echo 	"<a href=\"javascript:openScript('pctb.php?nid=".$nid."&uid=".$pc["UID"]."&subject=".base64_encode($subject)."',460 , 480)\">引用</a>\n";
    		if(pc_is_manager($currentuser) && $nodetype == 0)
    			echo "<a href=\"#\" onclick=\"bbsconfirm('pcadmin_blo.php?nid=".$nid."','确实要锁定此文吗(不增加访问量，不排名。此操作无法恢复！)?')\">锁定</a>\n";
    		echo 	"<a href=\"";
    		if($pc["EMAIL"])
    			echo "mailto:".$pc["EMAIL"];
    		else
    			echo  "/bbspstmail.php?userid=".$pc["USER"]."&title=问候";
		
		}
		echo "\">写信问候</a>\n".
			//"<a href=\"pccon.php?id=".$id."&nid=".$nid."\">转寄</a>\n".
			//"<a href=\"pccon.php?id=".$id."&nid=".$nid."\">转载</a>\n".
			"<a href=\"pcdoc.php?userid=".$pc["USER"]."&pid=".$pid."&tag=".$tag."&order=".$order."&tid=".$tid."\">返回目录</a>\n".
			"<a href=\"javascript:history.go(-1);\">快速返回</a>\n";
	    bbs_add_super_fav ('['.$pc['NAME'].'] '.$subject, '/pc/pccon.php?id='.$pc['UID'].'&nid='.$nid.'&s=all');
	}
	
	function display_pc_trackbacks($link,$nid)
	{
		$query = "SELECT * FROM trackback WHERE nid = ".$nid;
		$result = mysql_query($query,$link);
		$tb_num = mysql_num_rows($result);
?>
<table cellspacing="0" cellpadding="3" border="0" width="90%" class="t1">
<tr>
	<td class="t9" colspan="2">共有 <?php echo $tb_num; ?> 条引用</td>
</tr>
<?php
		for($i = 0;$i < $tb_num ;$i++)
		{
			if($i%2==0)
				$tdclass= array("t8","t10","t11");
			else
				$tdclass= array("t5","t12","t13");
			$rows = mysql_fetch_array($result);
			echo "<tr>\n<td class=\"".$tdclass[1]."\">&nbsp;".
				"<a href=\"".htmlspecialchars($rows[url])."\">".
				html_format($rows[title]).
				"</a>&nbsp;&nbsp;".
				"[".time_format($rows[time])."]".
				"</td><td width=\"100\" align=\"right\" class=\"".$tdclass[0]."\"><font class=\"f4\">".($i+1)."</font>&nbsp;&nbsp;</td>\n</tr>\n".
			        "<tr>\n<td colspan='2' class=\"".$tdclass[2]."\"><font class=\"content\">".
				html_format($rows[excerpt],TRUE)."</font></td>\n</tr>\n".
				"<tr>\n<td colspan='2' align='right' class=\"".$tdclass[0]."\">[FROM: <a href=\"".htmlspecialchars($rows[url])."\">".$rows[blogname]."</a> ]".
				"</td>\n</tr>\n";
	
		}
?>
</table>
<?php		
		mysql_free_result($result);
		return $tb_num;
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
			$query = "SELECT `username` , `emote` , `subject` , `created`,`cid`,`htmltag` FROM comments WHERE `nid` = '".$nid."' AND `uid` = '".$uid."' ORDER BY `cid` ASC ;";
		
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
			$rows = mysql_fetch_array($result);
			$contentcss = ($rows["htmltag"])?"contentwithhtml":"content";
			if($i%2==0)
				$tdclass= array("t8","t10","t11");
			else
				$tdclass= array("t5","t12","t13");
			echo "<tr>\n<td class=\"".$tdclass[1]."\">&nbsp;".
				"<img src=\"icon/".$rows["emote"].".gif\" border=\"0\" align=\"absmiddle\">\n".
				"<a href=\"pcshowcom.php?cid=".$rows["cid"]."\">".
				html_format($rows["subject"]).
				"</a>".
				"[<a href=\"/bbsqry.php?userid=".$rows["username"]."\">".$rows["username"]."</a> 于 ".time_format($rows["created"])." 提到]\n";
			if($perm || ((strtolower($rows["username"]) == strtolower($currentuser["userid"])) && (strtolower($rows["username"]) != "guest")) || pc_is_manager($currentuser) )
				echo "[<a href=\"#\" onclick=\"bbsconfirm('pceditcom.php?act=del&cid=".$rows["cid"]."','确认删除?')\">删</a>]\n";
			if((strtolower($rows["username"]) == strtolower($currentuser["userid"])) && (strtolower($rows["username"]) != "guest"))
				echo "[<a href=\"pceditcom.php?act=edit&cid=".$rows["cid"]."\">改</a>]\n";
			echo "</td><td width=\"100\" align=\"right\" class=\"".$tdclass[0]."\"><font class=\"f4\">".($i+1)."</font>&nbsp;&nbsp;</td>\n</tr>\n";
			if($spr)
			{
				echo "<tr>\n<td colspan='2' class=\"".$tdclass[2]."\"><font class='".$contentcss."'>".
					html_format($rows["body"],TRUE,$rows["htmltag"])."</font></td>\n</tr>\n".
					"<tr>\n<td colspan='2' align='right' class=\"".$tdclass[0]."\">[FROM: ".pc_hide_ip($rows["hostname"])."]".
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
	@$pid = (int)($_GET["pid"]);
	@$tag = (int)($_GET["tag"]);
	@$tid = (int)($_GET["tid"]);
	
	if(@$_GET["s"]=="all")
		$spr = TRUE;
	else
		$spr = FALSE;
	
	$link = pc_db_connect();
	if( @$_GET["p"] == "p" || @$_GET["p"] == "n" )
	{
		if( $_GET["p"] == "p" )
		{
			$query = "SELECT `nid` FROM nodes WHERE `nid` < ".$nid." AND `uid` = '".$id."' AND `pid` = '".$pid."' AND `access` = '".$tag."' AND `tid` = '".$tid."' AND `type` != '1' ORDER BY `nid` DESC LIMIT 0 , 1 ;";
			$err_alert = "本文已是该分类第一篇文章。";
		}
		else
		{
			$query = "SELECT `nid` FROM nodes WHERE `nid` > ".$nid." AND `uid` = '".$id."' AND `pid` = '".$pid."' AND `access` = '".$tag."' AND `tid` = '".$tid."' AND `type` != '1' ORDER BY `nid` ASC LIMIT 0 , 1 ;";
			$err_alert = "本文已是该分类最后一篇文章。";
		}
		$result = mysql_query($query,$link);
		$rows = mysql_fetch_array($result);
		mysql_free_result($result);
		if($rows)
			header("Location: pccon.php?id=".$id."&nid=".$rows[nid]."&pid=".$pid."&tag=".$tag."&tid=".$tid);	
	}
	
	$pc = pc_load_infor($link,"",$id);
	if(!$pc)
	{
		pc_db_close($link);
		html_init("gb2312","Blog");		
		html_error_quit("对不起，您要查看的Blog不存在");
		exit();
	}
	
	$userPermission = pc_get_user_permission($currentuser,$pc);
	$pur = $userPermission["pur"];
	$tags = $userPermission["tags"];
		
	if ($pc['USER'] == '_filter')	
	    $query = "SELECT * FROM filter WHERE `fid` = '".$nid."'  LIMIT 0 , 1 ;";
	else
	    $query = "SELECT * FROM nodes WHERE `nid` = '".$nid."' AND `uid` = '".$id."' AND type = 0 LIMIT 0 , 1 ;";

	$result = mysql_query($query,$link);
	$rows = mysql_fetch_array($result);
	mysql_free_result($result);
	
	if(!$rows)
	{
		pc_html_init("gb2312",$pc["NAME"],"","",$pc["BKIMG"]);
		html_error_quit("对不起，您要查看的文章不存在");
		exit();
	}
	
	if ($pc['USER'] != '_filter') {
    	if(!$tags[$rows["access"]])
    	{
    		pc_html_init("gb2312",$pc["NAME"],"","",$pc["BKIMG"]);
    		html_error_quit("对不起，您无权查看该文章!");
    		exit();
    	}
    	$nid = $rows["nid"];
	    $tid = $rows["tid"];
	    $author = $pc["USER"];

        if (pc_is_groupwork($pc) && $rows["publisher"])
            $author = $rows["publisher"];
        else
            $author = $pc["USER"];
	}
	else {
	    $nid = $rows[fid];
	    $tid = 0;
	    if (pc_is_groupwork($pc) && $rows["publisher"])
            $author = $rows["publisher"];
        else
            $author = $rows["username"];
	}
	
	if(!($pur == 3 && !pc_is_groupwork($pc)) && $rows["nodetype"] == 0)
	{
		pc_counter($link);
		pc_ncounter($link,$nid);
	}
	
	pc_html_init("gb2312",$pc["NAME"],"","",$pc["BKIMG"]);
	
	if( @$err_alert )
		echo "<script language=\"javascript\">alert(\"".$err_alert."\");</script>";
?>
<a name="top"></a>
<table cellspacing="0" cellpadding="0" border="0" width="100%">
<tr>
	<td>
	<table cellspacing="0" cellpadding="3" border="0" width="100%" class="tt1">
		<tr>
			<td>&nbsp;&nbsp;&nbsp;&nbsp;<?php echo "<a href=\"http://".$pcconfig["SITE"]."/frames.html\" target=\"_top\" class=f1>".$pcconfig["BBSNAME"]."</a> - <a href='index.html' class=f1>Blog</a> - <a href=\"index.php?id=".$pc["USER"]."\" class=f1>".$pc["NAME"]."</a>"; ?></td>
			<td align="right"><?php echo pc_personal_domainname($pc["USER"]); ?>&nbsp;&nbsp;&nbsp;&nbsp;</td>
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
	<td align="center">
	<table cellspacing="0" cellpadding="5" border="0" width="90%" class="t1">
	<tr>
		<td colspan="2" class="t9">
		<img src="icon/<?php echo $rows["emote"]; ?>.gif" border="0" align="absmiddle">
		<?php echo html_format($rows["subject"]); ?></td>
	</tr>
	<tr>
		<td width="20%" align="left" valign="top" class="t8">
		作者：<?php echo "<a href=\"/bbsqry.php?userid=".$author."\">".$author."</a>"; ?><br/>
		发表时间：<br/>
		<?php echo time_format($rows["created"]); ?><br/>
		更新时间：<br/>
		<?php echo time_format($rows["changed"]); ?><br/>
		浏览：<?php echo $rows["visitcount"]; ?>次<br>
		主题：<a href="pcsec.php?sec=<?php echo $rows["theme"]; ?>"><?php echo html_format($pcconfig["SECTION"][$rows["theme"]]); ?></a><br/>
		<?php
			if($rows["comment"]==0)
				echo "锁定主题<br>";
			else
				echo "评论：".$rows["commentcount"]."篇<br>";
			
			if($rows["trackback"])
				echo "引用：".$rows["trackbackcount"]."次<br/>";
		?>
		地址：<?php echo pc_hide_ip($rows["hostname"]); ?>
<?php
    if ($pc['USER'] != '_filter') {
?>
		<br/><br/><br/>
		<table cellspacing=0 cellpadding=5 width=95% border=0 class=t1>
			<tr><td class=t3>
			:::栏目:::
			</td></tr>
			<tr><td class=t5 style="line-height:20px">
			<ul>
<?php
	$blogtopics = pc_blog_menu($link,$pc,$rows["access"]);
	foreach( $blogtopics as $blogtopic )
	{
		if( $blogtopic["TID"] != $tid )
			echo "<li><a href=\"pcdoc.php?userid=".$pc["USER"]."&tag=".$rows["access"]."&tid=".$blogtopic["TID"]."\">".html_format($blogtopic["NAME"])."</a></li>";
		else
			echo "<li><a href=\"pcdoc.php?userid=".$pc["USER"]."&tag=".$rows["access"]."&tid=".$blogtopic["TID"]."\"><strong><font color=red>".html_format($blogtopic["NAME"])."</font></strong></a></li>";
	}
?>
			</ul>
			</td></tr>
		</table>
<?php
    }
?>
		</td>
		<td width="80%" height="300" align="left" valign="top" class="t5">
		<font class="<?php echo ($rows["htmltag"])?"contentwithhtml":"content"; ?>">
		<?php echo html_format($rows["body"],TRUE,$rows["htmltag"]); ?>&nbsp;
		</font>
		</td>
	</tr>
	<tr>
		<td colspan="2" align="right" class="t8">
		<?php display_navigation_bar($link,$pc,$nid,$rows["pid"],$rows["access"],$spr,addslashes(@$_GET["order"]),$rows["comment"],$tid,$pur,$rows["trackback"],$rows["subject"] , $rows["recommend"] , $rows["nodetype"] , 0, 0);// $rows["username"] , $rows["state"]); ?>
		</td>
	</tr>
	</table>
	</td>
</tr>
<?php
		if($rows["comment"] && $rows["commentcount"])
		{
?>
<tr>
	<td align="center"><br/><?php $re_num = display_pc_comments($link,$rows["uid"],$rows["nid"],$spr); ?></td>
</tr>
<?php
	}
		if($rows["trackback"] && $rows["trackbackcount"] && $rows["access"] == 0)
		{
?>
<tr>
	<td align="center"><br/><?php $tb_num = display_pc_trackbacks($link,$rows["nid"]); ?></td>
</tr>
<?php
		}
?>
<tr>
	<td align="center" class="f1" height="40" valign="middle">
	<?php
		if(@$re_num != 0 || @$tb_num != 0)
			display_navigation_bar($link,$pc,$nid,$rows["pid"],$rows["access"],$spr,addslashes(@$_GET["order"]),$rows["comment"],$tid,$pur,$rows["trackback"],$rows["subject"] , $rows["recommend"] , $rows["nodetype"] , 0, 0); //$rows["username"] , $rows["state"]); 
	?>
	&nbsp;</td>
</tr>
<tr>
	<td>
	<?php 
		if ($pc['USER'] != '_filter')
		if($rows["comment"] && $rows["type"] == 0)
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
	[<a href='javascript:location.reload()' class=f1>刷新</a>]
	[<?php 
		echo "<a href=\"";
		if($pc["EMAIL"])
			echo "mailto:".$pc["EMAIL"];
		else
			echo "/bbspstmail.php?userid=".$pc["USER"]."&title=问候";
		echo "\" class=f1>给".$pc["USER"]."写信</a>"; 
	?>][<a href="index.php?id=<?php echo $pc["USER"]; ?>" class=f1><?php echo $pc["NAME"]; ?>首页</a>]
	[<a href="index.html" class=f1>Blog首页</a>]
	[<a href="/frames.html" class=f1 target="_top"><?php echo $pcconfig["BBSNAME"]; ?>首页</a>]
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
