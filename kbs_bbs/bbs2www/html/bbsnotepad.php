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
                if (cache_header("public",filemtime($top_file),300))
                	return;
		html_init("gb2312");
?>
<body>
<center><?php echo BBS_FULL_NAME; ?> -- 留言板
<hr>
<pre>
<?php 
	echo bbs_printansifile($top_file);
?>
</pre>
</center>
<?php
		html_normal_quit();
	}
?>
