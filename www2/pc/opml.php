<?php
require("pcfuncs.php");
require("pcstat.php");

function pc_opml_init($opmlTitle)
{
echo '<?xml version="1.0" encoding="gb2312" ?>';
?>
<opml>
<head>
<title><?php echo htmlspecialchars($opmlTitle); ?></title>
</head>
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

function pc_opml_outline($title,$htmlUrl,$xmlUrl,$description,$text="")
{
	if(!$text) $text = $title;
?>
<outline text="<?php echo htmlspecialchars($text); ?>" title="<?php echo htmlspecialchars($title); ?>" type="rss" version="RSS" htmlUrl="<?php echo htmlspecialchars($htmlUrl); ?>" xmlUrl="<?php echo htmlspecialchars($xmlUrl); ?>" description="<?php echo htmlspecialchars($description); ?>" /> 
<?php	
}

//10min更新一次
if(pc_update_cache_header())
	return;

$type = 0;
@$type = intval( $_GET["t"]);
$link = pc_db_connect();
if($type == 3) //积分最高
{
	$blogs = getScoreTopUsers($link, 100);
	$opmlTitle = $pcconfig["BBSNAME"] . "BLOG热门用户组";
}
elseif($type == 2) //新用户
{
	$blogs = getNewUsers($link,100);
	$opmlTitle = $pcconfig["BBSNAME"] . "BLOG新用户组";
}
elseif($type == 1) //最高访问
{
	$blogs = getMostVstUsers($link,100);
	$opmlTitle = $pcconfig["BBSNAME"] . "BLOG热门用户组";
}
else //最近更新
{
	$blogs = getLastUpdates($link,100);
	$opmlTitle = $pcconfig["BBSNAME"] . "BLOG更新用户组";
}
pc_db_close($link);
header("Content-Type: text/xml");
header("Content-Disposition: inline;filename=opml.xml");
pc_opml_init($opmlTitle);
foreach($blogs as $blog)
{
	$title = stripslashes($blog["corpusname"]);
	$htmlUrl = pc_personal_domainname($blog["username"]);
	$xmlUrl = "http://".$pcconfig["SITE"] . "/pc/rss.php?userid=" . $blog["username"];
	$description = stripslashes($blog["description"]);
	pc_opml_outline($title,$htmlUrl,$xmlUrl,$description);
}
pc_opml_quit();
?>
