<?php
	/*
	** @id:windinsn nov 28,2003
	*/
	require("rsstool.php");
	
	$userid = addslashes($_GET[userid]);
	$link = pc_db_connect();
	$query = "SELECT `uid`,`username`,`corpusname`,`description`,`theme`,`modifytime` FROM users WHERE `username` = '".$userid."' LIMIT 0 , 1 ;";
	$result = mysql_query($query,$link);
	$rows = mysql_fetch_array($result);
	if(!$rows)
	{
		html_init("gb2312");
		html_error_quit("Blog²»´æÔÚ!");
		exit();
	}
	
	if( pc_cache( $rows[modifytime] ))
		return;
	
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
			"pcaddr" => pc_personal_domainname($pc["USER"]),
			"desc" => $pc["DESC"],
			"email" => $pc["USER"].".bbs@".$pcconfig["SITE"]
			);
	
	$nItems = $pcconfig["ETEMS"];
	if (strcmp($currentuser["userid"], $userid) == 0) $nItems = 100000;
	
	$query = "SELECT * FROM nodes WHERE `uid` = '".$pc["UID"]."' AND `access` = 0 ORDER BY `nid` DESC LIMIT 0 , ".$nItems."  ;";
	$result = mysql_query($query,$link);
	$i = 0;
	while($rows = mysql_fetch_array($result))
	{
		$rss[etems][$i] = array(
					"addr" => "http://".$pcconfig["SITE"]."/pc/pccon.php?id=".$pc["UID"]."&amp;nid=".$rows[nid]."&amp;tid=".$rows[tid],
					"etemcomaddr" => "http://".$pcconfig["SITE"]."/pc/pccom.php?act=pst&amp;nid=".$rows[nid],
					"etemtitle" => htmlspecialchars(stripslashes($rows[subject])),
					"etemdesc" => html_format($rows[body],TRUE,$rows[htmltag]),
					"etemauth" => $pc["USER"],
					"etemtime" => rss_time_format($rows[created]),
					"gmttime" => gmt_date_format(strtotime(time_format($rows[created])." +0800")),
					"etemnid" => $rows[nid]
					);
		$i ++ ;
	}
	mysql_free_result($result);
	pc_db_close($link);
	
	if (isset($_GET["v"]) && ($_GET["v"] == "2")) {
		header("Content-Type: text/xml; charset=gb2312");
		header("Content-Disposition: inline;filename=rss2.xml");
		@pc_rss2_output($rss);
	} else {
		header("Content-Type: text/xml");
		header("Content-Disposition: inline;filename=rss.xml");
		@pc_rss_output($rss);
	}
?>
