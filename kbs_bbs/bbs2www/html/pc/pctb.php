<?php
	/*
	** @id:windinsn dec 18,2003
	*/
	$needlogin=0;
	require("pcfuncs.php");
	function pc_get_trackback($link,$nid)
	{
		$query = "SELECT * FROM trackback WHERE `nid` = '".$nid."' ;";
		$result = mysql_query($query,$link);
		$tbarr = array();
		for($i = 0;$i < mysql_num_rows($result) ; $i++ )
			$tbarr[$i] = mysql_fetch_array($result);
		mysql_free_result($result);
		return $tbarr;
	}
	
	
	$nid = (int)($_GET["nid"]);
	$subject = html_format(base64_decode($_GET["subject"]));
	$link = pc_db_connect();
	$tbarr = pc_get_trackback($link,$nid);
	pc_html_init("gb2312",$subject);
?>
<table cellspacing=0 cellpadding=3 border=0 width=100% class=t1>
	<tr>
		<td class=t2><font class=content>继续我们的讨论……</font></td>
	</tr>
	<tr>
		<td class=t8><font class=content>
		<strong>TrackBack Ping URL:</strong><br />
		http://<?php echo $pcconfig["SITE"]; ?>/pc/tb.php?id=<?php echo $nid; ?><br /><br />
		<strong>Subject:</strong><br />
		<?php echo $subject; ?>
		</font></td>
	</tr>
<?php
	foreach($tbarr as $tb)
	{
		echo "<tr>\n<td class=t5><font class=content>\n".
			"<strong>主题:</strong>\n<a href='".html_format($tb[url])."'>".html_format($tb[title])."</a><br />\n".
			"<strong>摘要:</strong>\n".html_format($tb[excerpt],TRUE)." <br />\n".
			"<strong>Blog:</strong>\n".html_format($tb[blogname])."<br />\n".
			"<strong>时间:</strong>\n".time_format($tb[time])."\n".
			"</font></td></tr>\n";	
	}
	
?>
</table>
<?php	
	pc_db_close($link);
	html_normal_quit();
?>