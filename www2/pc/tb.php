<?php
/*
** @id:windinsn dec 18,2003
*/
	//$needlogin=0;
	/*
	** ../funcs.php中将未登录用户自动初始化为guest，这里不需要传递$needlogin=0，否则不能进行管理 windinsn dec 24,2003
	*/
	require("pcfuncs.php");
	
	if ($pcconfig["ENCODINGTBP"]) {
    	iconv_set_encoding("internal_encoding", "UTF-8");
        iconv_set_encoding("output_encoding", "UTF-8");
        iconv_set_encoding("input_encoding", "UTF-8");
    }

    function pc_tb_return_succeed()
	{
		header("Content-Type: text/xml");
		header("Content-Disposition: inline;filename=rss.xml");
		echo '<?xml version="1.0" encoding="UTF-8"?>';
?>
<response>
<error>0</error>
</response>
<?php		
	}
	
	function pc_tb_return_fail($err)
	{
		header("Content-Type: text/xml");
		header("Content-Disposition: inline;filename=rss.xml");
		echo '<?xml version="1.0" encoding="UTF-8"?>';
?>
<response>
<error>1</error>
<message><?php echo $err; ?></message>
</response>
<?php		
	}

	function pc_tb_check_node($link,$nid)
	{
		$query = "SELECT `uid` FROM nodes WHERE `nid` = '".intval($nid)."' AND `access` = 0 AND `trackback` = 1 LIMIT 0,1;";	
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
		global $_SERVER;
		$query = "UPDATE nodes SET `trackbackcount` = `trackbackcount` + 1 , `visitcount` = `visitcount` + 1 , `changed` = `changed` WHERE `nid` = '".intval($tbarr[nid])."';";	
		mysql_query($query,$link);
		$query = "INSERT INTO `trackback` ( `tbid` , `uid` , `nid` , `title` , `excerpt` , `url` , `blogname` , `time` ,`address`) ".
			"VALUES ('', '".intval($tbarr[uid])."', '".intval($tbarr[nid])."', '".addslashes($tbarr[title])."', '".addslashes($tbarr[excerpt])."', '".addslashes($tbarr[url])."', '".addslashes($tbarr[blogname])."', NOW( ) , '".addslashes($_SERVER["REMOTE_ADDR"])."' );";
		mysql_query($query,$link);
	}
	
		
	$title = $_POST["title"];
	$excerpt = $_POST["excerpt"];
	$url = $_POST["url"];
	$blog_name = $_POST["blog_name"];
	
	$nid = intval($_GET["id"]);
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
	
	if ($pcconfig["ENCODINGTBP"]) {
        $title = iconv(mb_detect_encoding ($title),$default_encoding,$title);
	    $excerpt = iconv(mb_detect_encoding ($excerpt),$default_encoding,$excerpt);
	    $blog_name = iconv(mb_detect_encoding ($blog_name),$default_encoding,$blog_name);
	}
	
	$tbarr = array(
			"uid" => $uid,
			"nid" => $nid,
			"title" => $title?$title:"无主题",
			"excerpt" => $excerpt,
			"url" => $url,
			"blogname" => $blog_name?$blog_name:$url
			);
	pc_tb_add_trackback($link,$tbarr);
	pc_db_close($link);
	pc_tb_return_succeed();
?>
