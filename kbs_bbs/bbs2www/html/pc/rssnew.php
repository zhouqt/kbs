<?php
	/*
	** @id:windinsin nov 29,2003
	*/
	require("rsstool.php");
	require("pcstat.php");
	
	$link = pc_db_connect();
	$rss = getNewBlogs($link);
	pc_db_close($link);
	header("Content-Type: text/xml");
	header("Content-Disposition: inline;filename=userrss.xml");
	@pc_rss_user($rss);
?>