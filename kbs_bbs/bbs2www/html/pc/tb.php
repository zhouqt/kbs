<?php
/*
** @id:windinsn dec 18,2003
*/
	$needlogin=0;
	require("pcfuncs.php");
	
	function pc_tb_return_succeed()
	{
		header("Content-Type: text/xml");
		header("Content-Disposition: inline;filename=rss.xml");
?>
<?xml version="1.0" encoding="iso-8859-1"?>
<response>
<error>0</error>
</response>
<?php		
	}
	
	function pc_tb_return_fail($err)
	{
		header("Content-Type: text/xml");
		header("Content-Disposition: inline;filename=rss.xml");
?>
<?xml version="1.0" encoding="iso-8859-1"?>
<response>
<error>1</error>
<message><?php echo $err; ?></message>
</response>
<?php		
	}

	function pc_tb_check_node($link,$nid)
	{
		$query = "SELECT `uid` FROM nodes WHERE `nid` = '".$nid."' AND `access` = 0 AND `trackback` = 1 LIMIT 0,1;";	
		$result = mysql_query($query,$link);
		$rows = mysql_fetch_array($result);
		mysql_free_result($result);
		if($rows)
			return $rows[uid];
		else
			return FALSE;
	}
	
	function pc_tb_add_trackback($link,$tbarr)
	{
		$query = "UPDATE nodes SET `trackbackcount` = `trackbackcount` + 1 , `visitcount` = `visitcount` + 1 WHERE `nid` = '".$tbarr[nid]."';";	
		mysql_query($query,$link);
		$query = "INSERT INTO `trackback` ( `tbid` , `uid` , `nid` , `title` , `excerpt` , `url` , `blogname` , `time` ) ".
			"VALUES ('', '".$tbarr[uid]."', '".$tbarr[nid]."', '".addslashes($tbarr[title])."', '".addslashes($tbarr[excerpt])."', '".addslashes($tbarr[url])."', '".addslashes($tbarr[blogname])."', NOW( ) );";
		mysql_query($query,$link);
	}
	
		
	$title = ($_POST["title"])?($_POST["title"]):($_GET["title"]);
	$excerpt = ($_POST["excerpt"])?($_POST["excerpt"]):($_GET["excerpt"]);
	$url = ($_POST["url"])?($_POST["url"]):($_GET["url"]);
	$blog_name = ($_POST["blog_name"])?($_POST["blog_name"]):($_GET["blog_name"]);
	//²¿·ÖblogÕ¾µã»¹Ê¹ÓÃÀÏµÄtrackback¹æ·¶£¬Òò´ËÅÐ¶Ï_GET´«ÊäÀ´µÄ²ÎÊý windinsn
	$nid = $_GET["id"];
	$link = pc_db_connect();
	if(!$url)
	{
		pc_tb_return_fail("Need URL");
		exit();
	}
	$uid = pc_tb_check_node($link,$nid);
	if(!$uid)
	{
		pc_tb_return_fail("Article NOT Exist");
		exit();
	}
	
	$tbarr = array(
			"uid" => $uid,
			"nid" => $nid,
			"title" => $title?$title:"ÎÞÖ÷Ìâ",
			"excerpt" => $excerpt,
			"url" => $url,
			"blogname" => $blog_name?$blog_name:$url
			);
	die("dd=".$_SERVER["REMOTEADDR"]);
	pc_tb_add_trackback($link,$tbarr);
	pc_db_close($link);
	pc_tb_return_succeed();
?>