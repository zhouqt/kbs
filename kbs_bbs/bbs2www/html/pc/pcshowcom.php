<?php
	/*
	** this file display single comment
	** @id:windinsn nov 27,2003
	*/
	$needlogin=0;
	require("pcfuncs.php");
	
	$cid = (int)($_GET["cid"]);
	
	pc_html_init("gb2312","个人文集");
	$link = pc_db_connect();
	$query = "SELECT * FROM comments WHERE `cid` = '".$cid."' LIMIT 0 , 1 ; ";
	$result = mysql_query($query,$link);
	$comment = mysql_fetch_array($result);
	if(!$comment)
	{
		@mysql_free_result($result);
		html_error_quit("对不起，您要查看的评论不存在");
		exit();
	}
	$query = "SELECT `access`,`uid`,`subject`,`emote`,`tid`,`pid` FROM nodes WHERE `nid` = '".$comment[nid]."' LIMIT 0 , 1 ; ";
	$result = mysql_query($query,$link);
	$node = mysql_fetch_array($result);
	if(!$node)
	{
		@mysql_free_result($result);
		html_error_quit("对不起，您要查看的评论不存在");
		exit();
	}
	if($node[access] > 0)
	{
		//判断是否为好友或所有者	
		if ($loginok != 1 || !strcmp($currentuser["userid"],"guest"))
		{
			html_error_quit("对不起，guest 不能查看本条记录!");
			exit();
		}
		
		$query = "SELECT `username` FROM users WHERE `uid` = '".$node[uid]."' LIMIT 0 , 1 ; ";
		$result = mysql_query($query,$link);
		$rows = mysql_fetch_array($result);
		$pc = array(
				"USER" => $rows[username],
				"UID" => $node[uid]
				);
		mysql_free_result($result);
		if( ( $node[access] == 1 && !pc_is_friend($currentuser["userid"],$pc["USER"])) || ( $node[access] > 1 && strtolower($pc["USER"]) != strtolower($currentuser["userid"]) ) )
		{
			html_error_quit("对不起，您不能查看本条记录!");
			exit();
		}
	}
?>
<br>
<center>
<table cellspacing="0" cellpadding="5" border="0" width="90%" class="t1">
<tr>
	<td class="t2">
	<img src="icon/<?php echo $node[emote]; ?>.gif" border="0" alt="心情符号" align="absmiddle">
	《
	<a href="pccon.php?<?php echo "id=".$node[uid]."&nid=".$comment[nid]."&pid=".$node[pid]."&tid=".$node[tid]."&tag=".$node[access]; ?>" class="t2">
	<?php echo html_format($node[subject]); ?>
	</a>
	》
	的评论
	</td>
</tr>
<tr>
	<td class="t8">
	<?php
		echo "<a href=\"/bbsqry.php?userid=".$comment[username]."\">".$comment[username]."</a>\n".
			"于 ".time_format($comment[created])." 提到:\n";
	?>
	</td>
</tr>
<tr>
	<td class="t13">
	<img src="icon/<?php echo $comment[emote]; ?>.gif" border="0" alt="心情符号" align="absmiddle">
	<strong>
	<?php echo html_format($comment[subject]); ?>
	</strong>
	</td>
</tr>
<tr>
	<td class="t13" height="200" align="left" valign="top">
	<?php echo html_format($comment[body],TRUE); ?>
	</td>
</tr>
<tr>
	<td class="t5" align="right">
	[FROM:
	<?php echo $comment[hostname]; ?>
	]
	&nbsp;&nbsp;&nbsp;&nbsp;
	</td>
</tr>
<tr>
	<td class="t3">
<?php
	$query = "SELECT `cid` FROM comments WHERE `nid` = '".$comment[nid]."' AND `cid` < '".$cid."' ORDER BY `cid` DESC LIMIT 0 , 1 ; ";
	$result = mysql_query($query,$link);
	if($rows = mysql_fetch_array($result))
		echo "<a href=\"pcshowcom.php?cid=".$rows[cid]."\">上一篇</a> \n";
	else
		echo "上一篇 \n";
	$query = "SELECT `cid` FROM comments WHERE `nid` = '".$comment[nid]."' AND `cid` > '".$cid."' ORDER BY `cid` ASC LIMIT 0 , 1 ; ";
	$result = mysql_query($query,$link);
	if($rows = mysql_fetch_array($result))
		echo "<a href=\"pcshowcom.php?cid=".$rows[cid]."\">下一篇</a> \n";
	else
		echo "下一篇 \n";
	mysql_free_result($result);
?>	
	<a href="pccon.php?<?php echo "id=".$node[uid]."&nid=".$comment[nid]."&pid=".$node[pid]."&tid=".$node[tid]."&tag=".$node[access]; ?>">返回原文</a>
	<a href="pccom.php?act=pst&nid=<?php echo $comment[nid]; ?>">发表评论</a>
	<a href="/bbspstmail.php?userid=<?php echo $comment[username]; ?>&title=问候">寄信给<?php echo $comment[username]; ?></a>
	</td>
</tr>
</table>
</center>
<?php	
?>