<?php
require("pcfuncs.php");
require("pcstat.php");

function pc_opml_init()
{
?>
<?xml version="1.0" encoding="gb2312" ?> 
<opml>
<body>
<?php	
}

function pc_opml_quit()
{
?>
</body>
</opml>
<?php	
}

function pc_opml_outline($title,$htmlUrl,$xmlUrl)
{
?>
<outline title="<?php echo htmlspecialchars($title); ?>" htmlUrl="<?php echo htmlspecialchars($htmlUrl); ?>" xmlUrl="<?php echo htmlspecialchars($xmlUrl); ?>" /> 
<?php	
}

//10min更新一次
if(pc_update_cache_header())
	return;

$type = intval( $_GET["t"]);
$link = pc_db_connect();
if($type == 2) //新用户
	$blogs = getNewUsers($link,50);
elseif($type == 1) //最高访问
	$blogs = getMostVstUsers($link,50);
else //最近更新
	$blogs = getLastUpdates($link,50);
pc_db_close($link);
header("Content-Type: text/xml");
header("Content-Disposition: inline;filename=opml.xml");
pc_opml_init();
foreach($blogs as $blog)
{
	$title = stripslashes($blog[corpusname]);
	$htmlUrl = pc_personal_domainname($blog[username]);
	$xmlUrl = "http://".$pcconfig["SITE"] . "/pc/rss.php?userid=" . $blog[username];
	pc_opml_outline($title,$htmlUrl,$xmlUrl);
}
pc_opml_quit();
?>