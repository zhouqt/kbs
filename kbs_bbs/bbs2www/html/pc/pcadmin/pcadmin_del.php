<?php
require("pcadmin_inc.php");
pc_admin_check_permission();
$link = pc_db_connect();
$nid = intval($_GET["nid"]);
$query = 'SELECT * FROM filter WHERE nid = '.$nid.  ' LIMIT 1;';
$result = mysql_query($query);
$node = mysql_fetch_array($result);
if ($node)
    html_error_quit("文章已经被删除过");

$query = 'SELECT * FROM nodes WHERE nid = '.$nid.  ' LIMIT 1;';
$result = mysql_query($query);
$node = mysql_fetch_array($result);
if (!$node)
    html_error_quit("文章不存在");

$uid = $node[uid];

$query = "INSERT INTO `filter` (  `pid` , `nid` , `tid` , `type` , `state` , `recuser` , `emote` , `hostname` , `changed` , `created` , `uid` , `username` , `comment` , `commentcount` , `subject` , `body` , `access` , `visitcount` , `htmltag`,`trackback` ,`trackbackcount`,`nodetype`,`theme`,`publisher`) ".
	   	     "VALUES ( '".$node[pid]."' , '".$node[nid]."' , '".$node[tid]."' , '".$node[type]."' , 4 , '".$currentuser["userid"]."' , '".$node[emote]."' , '".$node[hostname]."' , '".$node[changed]."' , '".$node[created]."' , '".$node[uid]."' , '".addslashes($_GET["userid"])."' , '".$node[comment]."' , '".$node[commentcount]."' , '".$node[subject]."' , '".$node[body]."' , '".$node[access]."' , '".$node[visitcount]."' , '".$node[htmltag]."','".$node[trackback]."' ,'".$node[trackbackcount]."','".$node[nodetype]."','".$node[theme]."','".$node[publisher]."');";
if (!mysql_query($query)) {
	html_error_quit("插入过滤表出错");
}

$query = "UPDATE nodes SET `body` = '@ @ 本文已被管理员屏蔽 @ @' where `nid` = " . $nid . ";";
if (!mysql_query($query)) {
	html_error_quit("修改原文出错");
}
pc_update_record($link,$uid);

pc_db_close($link);
pc_return("pcdoc.php?userid=_filter&tag=4");
?>
