<?php
	/*
	** @id:windinsn nov 28,2003
	*/
	require("rsstool.php");
	
	$userid = addslashes($_GET[userid]);
	$link = pc_db_connect();
	$query = "SELECT `uid`,`username`,`corpusname`,`description`,`theme` FROM users WHERE `username` = '".$userid."' LIMIT 0 , 1 ;";
	$result = mysql_query($query,$link);
	$rows = mysql_fetch_array($result);
	if(!$rows)
	{
		html_init("gb2312");
		html_error_quit("个人文集不存在!");
		exit();
	}
	$pc = array(
			"NAME" => htmlspecialchars(stripslashes($rows[corpusname])),
			"USER" => $rows[username],
			"DESC" => htmlspecialchars(stripslashes($rows[description])),
			"THEM" => htmlspecialchars(stripslashes($rows[theme])),
			"UID" => $rows[uid]
			);
	$rss = array();
	$rss[channel] = array(
			"userid" => $pc["UID"] ,
			"siteaddr" => "http://".$pcconfig["SITE"],
			"title" => $pc["NAME"],
			"pcaddr" => "http://".$pc["USER"].".mysmth.net",
			"desc" => $pc["DESC"],
			"email" => $pc["USER"].".bbs@".$pcconfig["SITE"]
			);
	
	$query = "SELECT * FROM nodes WHERE `uid` = '".$pc["UID"]."' AND `access` = 0 ORDER BY `nid` DESC LIMIT 0 , ".$pcconfig["ETEMS"]."  ; ";
	$result = mysql_query($query,$link);
	$i = 0;
	while($rows = mysql_fetch_array($result))
	{
		$rss[etems][$i] = array(
					"etemaddr" => "http://".$pcconfig["SITE"]."/pc/pccon.php?id=".$pc["UID"]."&amp;nid=".$rows[nid]."&amp;tid=".$rows[tid],
					"etemcomaddr" => "http://".$pcconfig["SITE"]."/pc/pccom.php?act=pst&amp;nid=".$rows[nid],
					"etemtitle" => htmlspecialchars(stripslashes($rows[subject])),
					"etemdesc" => html_format($rows[body],TRUE),
					"etemauth" => $pc["USER"],
					"etemtime" => rss_time_format($rows[created]),
					"etemnid" => $rows[nid]
					);
		$i ++ ;
	}
	mysql_free_result($result);
	pc_db_close($link);
	
	header("Content-Type: text/xml");
	header("Content-Disposition: inline;filename=rss.xml");
	@pc_rss_output($rss);
?>