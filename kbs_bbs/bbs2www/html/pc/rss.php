<?php
	/*
	** @id:windinsn nov 28,2003
	*/
	$needlogin=0;
	require("pcfuncs.php");
	
	function pc_rss_init()
	{
?>
<?xml version="1.0" encoding="gb2312"?>
<rdf:RDF 
	xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
	xmlns:dc="http://purl.org/dc/elements/1.1/" 
	xmlns="http://purl.org/rss/1.0/"
>
<?php	
	}
	
	function pc_rss_end()
	{
?>
</rdf:RDF>
<?php
	}
	
	function pc_rss_channel($rss)
	{
?>
	<channel rdf:about="<?php echo $rss[channel][siteaddr]; ?>">
		<title><?php echo $rss[channel][title]; ?></title>
		<link><?php echo $rss[channel][pcaddr]; ?></link>
		<description><?php echo $rss[channel][desc]; ?></description>
		<dc:language>gb2312</dc:language> 
		<dc:creator><?php echo $rss[channel][email]; ?></dc:creator> 
<?php
		if($rss[channel][logoimg])
		{
?>
<image rdf:resource="<?php echo $rss[channel][logoimg]; ?>" />
<?php
		}
?>
		<items>
			<rdf:Seq>
				<rdf:li resource="<?php echo $rss[channel][siteaddr]; ?>" /> 
				<rdf:li resource="<?php echo $rss[channel][pcaddr]; ?>" /> 
			</rdf:Seq>
		</items>
	</channel>
<?php
	}
	
	function pc_rss_etem($etem)
	{
?>
	<item rdf:about="<?php echo $etem[etemaddr]; ?>">
		<title><?php echo $etem[etemtitle]; ?></title> 
		<link><?php echo $etem[etemaddr]; ?></link>
		<dc:creator><?php echo $etem[etemauth]; ?></dc:creator> 
		<dc:date><?php echo $etem[etemtime]; ?></dc:date> 
		<description>
			<![CDATA[
				<?php echo $etem[etemdesc]; ?>
			]]> 
		</description>
	</item>
<?php
	}
	
	function pc_rss_output($rss)
	{
		pc_rss_init();
		pc_rss_channel($rss);
		foreach($rss[etems] as $etem)
			pc_rss_etem($etem);
		pc_rss_end();
	}
	
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
			"NAME" => html_format($rows[corpusname]),
			"USER" => $rows[username],
			"DESC" => html_format($rows[description],TRUE),
			"THEM" => html_format($rows[theme]),
			"UID" => $rows[uid]
			);
	$rss = array();
	$rss[channel] = array(
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
					"etemtitle" => html_format($rows[subject]),
					"etemdesc" => html_format($rows[body],TRUE),
					"etemauth" => $pc["USER"],
					"etemtime" => rss_time_format($rows[created])
					);
		$i ++ ;
	}
	mysql_free_result($result);
	pc_db_close($link);
	
	header("Content-Type: text/xml");
	header("Content-Disposition: inline;filename=rss.xml");
	@pc_rss_output($rss);
?>