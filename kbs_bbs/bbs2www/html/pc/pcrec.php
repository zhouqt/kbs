<?php
/*
**  recommend articles
**  id:windinsn feb 28 , 2004
*/
require("pcfuncs.php");

if ($loginok != 1)
	html_nologin();

html_init("gb2312");

if(!strcmp($currentuser["userid"],"guest"))
{
	html_error_quit("guest �����Ƽ�����!");
	exit();
}

$nid = $_GET["nid"];
$nid = intval( $nid );

$link = pc_db_connect();
$query = "SELECT recommend , uid FROM nodes WHERE access = 0 AND type = 0 AND nid = ".$nid." LIMIT 0 , 1;";
$result = mysql_query($query , $link);
$node = mysql_fetch_array($result);

if(!$node)
{
	html_error_quit("�Բ�����Ҫ�Ƽ������²�����");
	exit();
}
if($node[recommend] != 0)
{
	html_error_quit("�Բ��𣬸����ѱ��Ƽ�");
	exit();
}
if(!pc_is_manager($currentuser))
{
	$pc = pc_load_infor($link,"",$node[uid]);	
	if(!$pc || !pc_is_admin($currentuser,$pc))
	{
		html_error_quit("�Բ�������Ȩ�Ƽ�����");
		exit();	
	}
}

$query = "UPDATE nodes SET changed  = changed , recommend = 1 WHERE nid = ".$nid." ;";
mysql_query($query,$link);
?>
<br /><br />
<p align="center">
�Ƽ��ɹ��������������¡�
<br /><br />
<a href="pccon.php?id=<?php echo $node[uid]; ?>&nid=<?php echo $nid; ?>">[����]</a>
</p>
<?php
pc_db_close($link);
html_normal_quit();
?>