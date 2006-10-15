<?php
	/*
	** @id:windinsin nov 29,2003
	*/
	require("rsstool.php");
	require("pcstat.php");
	
	//20min更新一次
	if(pc_update_cache_header())
		return;
	
	$link = pc_db_connect();
	$rss = getNewBlogs($link,1,$pcconfig["ETEMS"]);
	pc_db_close($link);
	header("Content-Type: text/xml");
	header("Content-Disposition: inline;filename=userrss.xml");
	@pc_rss_user($rss);
?>