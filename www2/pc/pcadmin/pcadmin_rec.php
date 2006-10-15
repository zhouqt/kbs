<?php
/*
**  manage blog recommends articls
**  id: windinsn feb 28 , 2004	
*/
require("pcadmin_inc.php");
pc_admin_check_permission();
$link = pc_db_connect();

if(isset($_GET[sig]) && $_GET[nid])
{
	$sig = intval($_GET[sig]);
	$nid = intval($_GET[nid]);
	
	if( $sig != 2 && $sig != 3 )//撤销推荐
	{
		$query = "DELETE FROM recommend WHERE nid = ".$nid." ;";
		mysql_query($query,$link);
	}
	elseif( $sig == 3 )//状态变为已推荐
	{
		$query = "UPDATE recommend SET state = 0 WHERE nid = ".$nid." ;";
		mysql_query($query,$link);
	}
	else
	{
		$query = "SELECT nid , hostname , created , uid ,subject , body , emote , htmltag , recuser , rectopic FROM nodes WHERE nid =".$nid." AND access = 0 AND type = 0 AND ( recommend = 1 OR recommend = 3 ) LIMIT 0,1;";
		$result = mysql_query( $query , $link);
		$rows = mysql_fetch_array($result);
		if($rows)
		{
			$query = "INSERT INTO `recommend` ( `rid` , `nid` , `hostname` , `created` , `uid` , `subject` , `body` , `emote` , `htmltag` , `recuser` ,`state`,`topic` ) ".
			"VALUES ('', '".$rows[nid]."', '".addslashes($rows[hostname])."', '".$rows[created]."', '".$rows[uid]."', '".addslashes($rows[subject])."', '".addslashes($rows[body])."', '".$rows[emote]."', '".$rows[htmltag]."' , '".addslashes($rows[recuser])."' , 1,'".addslashes($rows[rectopic])."');";
			mysql_query($query,$link);
		}
		mysql_free_result($result);
	}
	
	$query = "UPDATE nodes SET changed  = changed , recommend = ".$sig." WHERE access = 0 AND type = 0 AND nid = ".$nid." ;";
	mysql_query($query,$link);
}

$pno = $_GET["pno"];
$pno = intval( $pno );
if( $pno < 1 ) $pno = 1;
$start = ($pno - 1)*$pcconfig["LIST"];
$type = $_GET["type"];
$type = intval($type);

$recstate = array("error" , "<font color=#FF0000>待处理</font>" , "<font color=#0000FF>推荐中</fnot>" , "<font color=#333333>已推荐</font>" );

switch($type)
{
	case 1:
		$query = "SELECT nid , uid , emote , subject , created , visitcount , commentcount , trackbackcount , recommend , recuser , rectopic ".
		"FROM nodes WHERE access = 0 AND recommend = 1  ".
		"ORDER BY recommend ASC , nid DESC ".
		"LIMIT ".$start ." , ".$pcconfig["LIST"]." ;";
		break;
	case 2:
		$query = "SELECT nid , uid , emote , subject , created , visitcount , commentcount , trackbackcount , recommend , recuser , rectopic ".
		"FROM nodes WHERE access = 0 AND recommend = 2  ".
		"ORDER BY recommend ASC , nid DESC ".
		"LIMIT ".$start ." , ".$pcconfig["LIST"]." ;";
		break;
	case 3:
		$query = "SELECT nid , uid , emote , subject , created , visitcount , commentcount , trackbackcount , recommend , recuser , rectopic ".
		"FROM nodes WHERE access = 0 AND recommend = 3  ".
		"ORDER BY recommend ASC , nid DESC ".
		"LIMIT ".$start ." , ".$pcconfig["LIST"]." ;";
		break;
	default:
	$query = "SELECT nid , uid , emote , subject , created , visitcount , commentcount , trackbackcount , recommend , recuser , rectopic ".
		"FROM nodes WHERE access = 0 AND recommend > 0 AND recommend < 4 ".
		"ORDER BY recommend ASC , nid DESC ".
		"LIMIT ".$start ." , ".$pcconfig["LIST"]." ;";
}
$result = mysql_query( $query , $link );
$num = mysql_num_rows( $result );
pc_html_init("gb2312" , $pcconfig["BBSNAME"]."Blog推荐文章管理");
pc_admin_navigation_bar();
?><br/>
<p align="center">
<strong><?php echo $pcconfig["BBSNAME"]."Blog推荐文章管理"; ?></strong>
</p>
<p align="center">
[<a href="pcadmin_rec.php?type=1">待处理文章</a>]
[<a href="pcadmin_rec.php?type=2">推荐中文章</a>]
[<a href="pcadmin_rec.php?type=3">已推荐文章</a>]
[<a href="pcadmin_rec.php">全部</a>]
</p>
<center>
<table width="95%" cellspacing=0 cellpadding=5 class=t1 border=0>
<tr>
	<td width="30" class="t2">序号</td>
	<td width="40" class="t2">状态</td>
	<td class="t2">主题</td>
	<td width="120" class="t2">创建时间</td>
	<td width="60" class="t2">类型</td>
	<td width="20" class="t2">访</td>
	<td width="20" class="t2">回</td>
	<td width="20" class="t2">引</td>
	<td width="80" class="t2">推荐人</td>
	<td width="60" colspan="3" class="t2">处理</td>
</tr>
<?php
	for( $i = 0 ; $i < $num ; $i ++)
	{
		$recnode=mysql_fetch_array($result);
		echo "<tr>\n".
			"<td class=\"t3\">".( $i + 1 + $start )."</td>\n".
			"<td class=\"t4\">".$recstate[$recnode[recommend]]."</td>\n".
			"<td class=\"t8\"><a href=\"pccon.php?id=".$recnode[uid]."&nid=".$recnode[nid]."&s=all\">".html_format($recnode[subject])."</a></td>\n".
			"<td class=\"t4\">".time_format($recnode[created])."</td>\n".
			"<td class=\"t3\"><a href=\"pcreco.php?topic=".$recnode[rectopic]."\">".$pcconfig["SECTION"][$recnode[rectopic]]."</a></td>\n".
			"<td class=\"t3\">".$recnode[visitcount]."</td>\n".
			"<td class=\"t4\">".$recnode[commentcount]."</td>\n".
			"<td class=\"t3\">".$recnode[trackbackcount]."</td>\n".
			"<td class=\"t4\"><a href=\"/bbsqry.php?userid=".$recnode[recuser]."\">".$recnode[recuser]."</a></td>\n";
		if( $recnode[recommend] == 2 )
		echo	"<td class=\"t3\" width=\"20\"><span title=\"将本文撤销为已推荐文章\"><a href=\"pcadmin_rec.php?type=".$type."&pno=".$pno."&nid=".$recnode[nid]."&sig=3\">撤</a></span></td>\n".
			"<td class=\"t3\" width=\"20\"><span title=\"将本文撤销为未推荐文章\"><a href=\"pcadmin_rec.php?type=".$type."&pno=".$pno."&nid=".$recnode[nid]."&sig=0\">还</a></span></td>\n".
			"<td class=\"t3\" width=\"20\"><span title=\"将本文撤销为不能推荐文章\"><a href=\"pcadmin_rec.php?type=".$type."&pno=".$pno."&nid=".$recnode[nid]."&sig=4\">绝</a></span></td>\n";
		elseif( $recnode[recommend] == 3 )
		echo	"<td class=\"t3\" width=\"20\"><span title=\"将本文加入当前推荐文章\"><a href=\"pcadmin_rec.php?type=".$type."&pno=".$pno."&nid=".$recnode[nid]."&sig=2\">推</a></span></td>\n".
			"<td class=\"t3\" width=\"20\"><span title=\"将本文撤销为未推荐文章\"><a href=\"pcadmin_rec.php?type=".$type."&pno=".$pno."&nid=".$recnode[nid]."&sig=0\">还</a></span></td>\n".
			"<td class=\"t3\" width=\"20\"><span title=\"将本文撤销为不能推荐文章\"><a href=\"pcadmin_rec.php?type=".$type."&pno=".$pno."&nid=".$recnode[nid]."&sig=4\">绝</a></span></td>\n";
		else
		echo	"<td class=\"t3\" width=\"20\"><span title=\"将本文加入当前推荐文章中\"><a href=\"pcadmin_rec.php?type=".$type."&pno=".$pno."&nid=".$recnode[nid]."&sig=2\">是</a></span></td>\n".
			"<td class=\"t3\" width=\"20\"><span title=\"不推荐本文，但以后可以再次推荐\"><a href=\"pcadmin_rec.php?type=".$type."&pno=".$pno."&nid=".$recnode[nid]."&sig=0\">否</a></span></td>\n".
			"<td class=\"t3\" width=\"20\"><span title=\"不推荐本文，以后也不能推荐\"><a href=\"pcadmin_rec.php?type=".$type."&pno=".$pno."&nid=".$recnode[nid]."&sig=4\">绝</a></span></td>\n";
		echo 	"</tr>\n";
	}
?>
</table>
<?php
mysql_free_result( $result );
?>
<p align="right">
<a href="pcadmin_rec.php?pno=<?php echo $pno; ?>&type=<?php echo $type; ?>">[刷新]</a>

<?php
	if( $pno > 1)
		echo "<a href=\"pcadmin_rec.php?type=".$type."\">[第一页]</a>\n".
			"<a href=\"pcadmin_rec.php?type=".$type."&pno=".($pno-1)."\">[上一页]</a>\n";
	if( $pcconfig["LIST"] == $num ) echo "<a href=\"pcadmin_rec.php?type=".$type."&pno=".($pno+1)."\">[下一页]</a>\n";
?>
</p>
<?php
pc_db_close($link);
pc_admin_navigation_bar();
html_normal_quit();
?>