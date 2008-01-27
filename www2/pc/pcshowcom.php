<?php
	/*
	** this file display single comment
	** @id:windinsn nov 27,2003
	*/
	require("pcfuncs.php");
	
	$cid = (int)($_GET["cid"]);
	
	$link = pc_db_connect();
	$query = "SELECT * FROM comments WHERE `cid` = '".$cid."' LIMIT 0 , 1 ;";
	$result = mysql_query($query,$link);
	$comment = mysql_fetch_array($result);
	if(!$comment)
	{
		@mysql_free_result($result);
		pc_html_init("gb2312",$pcconfig["BBSNAME"]."Blog");
		html_error_quit("对不起，您要查看的评论不存在");
		exit();
	}
	$query = "SELECT `access`,`uid`,`subject`,`emote`,`tid`,`pid`,`nodetype` FROM nodes WHERE `nid` = '".$comment["nid"]."' LIMIT 0 , 1 ;";
	$result = mysql_query($query,$link);
	$node = mysql_fetch_array($result);
	if(!$node)
	{
		@mysql_free_result($result);
		pc_html_init("gb2312",$pcconfig["BBSNAME"]."Blog");
		html_error_quit("对不起，您要查看的评论不存在");
		exit();
	}
	
	$pc = pc_load_infor($link,"",$node["uid"]);
	if(!$pc)   
        {   
               	pc_html_init("gb2312",$pcconfig["BBSNAME"]."Blog");
		html_error_quit("对不起，您要查看的Blog不存在");   
               	exit();   
        }
	
	function pc_add_new_comment($nid,$alert)
	{
		global $pc, $currentuser;
?>
<center>
<table cellspacing="0" cellpadding="5" width="500" border="0" class="t1">
<tr>
	<td class="t5"><strong>发表评论 </strong>
	<?php if($alert && !$pc["ANONYCOMMENT"]){ ?>
	<font class=f4>
	注意：仅有本站登录用户才能发表评论。<br />
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
<form name="postform" action="pccom.php?act=add&nid=<?php echo $nid; ?>" method="post" onsubmit="if(this.subject.value==''){alert('请输入评论主题!');return false;}">
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
               
        $userPermission = pc_get_user_permission($currentuser,$pc);
	$sec = $userPermission["sec"];
	$pur = $userPermission["pur"];
	$tags = $userPermission["tags"];
	if(!$tags[$node["access"]])
	{
		pc_html_init("gb2312",$pcconfig["BBSNAME"]."Blog");
		html_error_quit("对不起，您不能查看本条记录!");
		exit();
	}
	
	if(!($pur == 3 && !pc_is_groupwork($pc)) &&  $node["nodetype"]==0)
	{
		pc_counter($link);
		pc_ncounter($link,$comment["nid"]);
	}
	
	pc_html_init("gb2312",$pcconfig["BBSNAME"]."Blog");
?>
<br>
<center>
<table cellspacing="0" cellpadding="5" border="0" width="90%" class="t1">
<tr>
	<td class="t2">
	<img src="icon/<?php echo $node["emote"]; ?>.gif" border="0" alt="心情符号" align="absmiddle">
	《
	<a href="pccon.php?<?php echo "id=".$node["uid"]."&nid=".$comment["nid"]."&pid=".$node["pid"]."&tid=".$node["tid"]."&tag=".$node["access"]; ?>" class="t2">
	<?php echo html_format($node["subject"]); ?>
	</a>
	》
	的评论
	</td>
</tr>
<tr>
	<td class="t8">
	<?php
		echo "<a href=\"/bbsqry.php?userid=".$comment["username"]."\">".$comment["username"]."</a>\n".
			"于 ".time_format($comment["created"])." 提到:\n";
	?>
	</td>
</tr>
<tr>
	<td class="t13">
	<img src="icon/<?php echo $comment["emote"]; ?>.gif" border="0" alt="心情符号" align="absmiddle">
	<strong>
	<?php echo html_format($comment["subject"]); ?>
	</strong>
	</td>
</tr>
<tr>
	<td class="t13" height="200" align="left" valign="top">
	<font class="<?php echo ($comment["htmltag"])?"contentwithhtml":"content"; ?>">
	<?php echo html_format($comment["body"],TRUE,$comment["htmltag"]); ?>
	</font>
	</td>
</tr>
<tr>
	<td class="t5" align="right">
	[FROM:
	<?php echo pc_hide_ip($comment["hostname"]); ?>
	]
	&nbsp;&nbsp;&nbsp;&nbsp;
	</td>
</tr>
<tr>
	<td class="t3">
<?php
	$query = "SELECT `cid` FROM comments WHERE `nid` = '".$comment["nid"]."' AND `cid` < '".$cid."' ORDER BY `cid` DESC LIMIT 0 , 1 ;";
	$result = mysql_query($query,$link);
	if($rows = mysql_fetch_array($result))
		echo "<a href=\"pcshowcom.php?cid=".$rows["cid"]."\">上一篇</a> \n";
	else
		echo "上一篇 \n";
	$query = "SELECT `cid` FROM comments WHERE `nid` = '".$comment["nid"]."' AND `cid` > '".$cid."' ORDER BY `cid` ASC LIMIT 0 , 1 ;";
	$result = mysql_query($query,$link);
	if($rows = mysql_fetch_array($result))
		echo "<a href=\"pcshowcom.php?cid=".$rows["cid"]."\">下一篇</a> \n";
	else
		echo "下一篇 \n";
	mysql_free_result($result);
?>	
	<a href="pccon.php?<?php echo "id=".$node["uid"]."&nid=".$comment["nid"]."&pid=".$node["pid"]."&tid=".$node["tid"]."&tag=".$node["access"]; ?>">返回原文</a>
	<a href="pccom.php?act=pst&nid=<?php echo $comment["nid"]; ?>">发表评论</a>
	<a href="/bbspstmail.php?userid=<?php echo $comment["username"]; ?>&title=问候">寄信给<?php echo $comment["username"]; ?></a>
	<?php bbs_add_super_fav ($node["subject"], '/pc/pcshowcom.php?cid='.$cid); ?>
	</td>
</tr>
</table>
<hr size=1>
<?php 
	$alert = ($loginok != 1 || !strcmp($currentuser["userid"],"guest"))?TRUE:FALSE;
	pc_add_new_comment($comment["nid"],$alert); 
?>
</center>
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
