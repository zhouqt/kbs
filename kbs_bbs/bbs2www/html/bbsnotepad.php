<?php
	/**
	 * $Id$ 
	 */
	require("funcs.php");
	if ($loginok !=1 )
		html_nologin();
	else
	{
		$top_file = get_bbsfile("/etc/notepad");
		$fp = fopen($top_file, "r");
		if ($fp == FALSE) {
		        html_init("gb2312");
			html_error_quit("现在没有留言");
                }
                $modifytime=filemtime($top_file);
	session_cache_limiter("public");
	$oldmodified=$_SERVER["HTTP_IF_MODIFIED_SINCE"];
	if ($oldmodified!="") {
/*
		list($dayobweek,$day,$month,$year,$hour,$minute,$second)=
			sscanf($oldmidofied,"%s, %d %s %d %d:%d:%d");
		$oldtime=gmmktime($hour,$minute,$second,$month,$day,$year);
*/
                $oldtime=strtotime($oldmodified);
	} else $oldtime=0;
	if ($oldtime>=$modifytime) {
		header("HTTP/1.1 304 Not Modified");
	        header("Cache-Control: max-age=300");
		return;
	}
	header("Last-Modified: " . gmdate("D, d M Y H:i:s", $modifytime) . "GMT");
	header("Expires: " . gmdate("D, d M Y H:i:s", $modifytime+300) . "GMT");
	header("Cache-Control: max-age=300");
	html_init("gb2312");
?>
<body>
<center><?php echo $BBS_FULL_NAME; ?> -- 留言板
<hr>
<pre>
<? 
	bbs_printansifile($top_file);
?>
</pre>
</center>
<?php
		html_normal_quit();
	}
?>
