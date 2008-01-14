<?php
require("pcfuncs.php");

function pc_load_nodes($link,$pc,$pur=0,$pno=1)
{
	$pno = intval($pno);
	if($pno <= 1 )
		$pno = 1;
	
	$start = ($pno - 1) * $pc["INDEX"]["nodeNum"];
	$query = "SELECT * FROM `nodes` WHERE `uid` = '".$pc["UID"]."' AND type = 0 ";
	if($pur == 0)
		$query .= " AND `access` = 0 ";
	elseif($pur == 1)
		$query .= " AND ( `access` = 0 OR `access` = 1 ) ";
	elseif($pur == 3)
		$query .= " AND ( `access` = 0 OR `access` = 1 OR `access` = 2 ) ";
	$query .= " ORDER BY `nid` DESC LIMIT ".$start." , 10 ;";
	$result = mysql_query($query,$link);
	$nodes = array();
	$i = 0;
	while($rows=mysql_fetch_array($result))
	{
		$nodes[$i]=$rows;
		$i ++;
	}
	mysql_free_result($result);
	return $nodes;
}

function pc_load_trackbacks($link,$pc)
{
	$query = "SELECT * FROM trackback WHERE uid = '".$pc["UID"]."' ORDER BY tbid DESC LIMIT 0 , 10;";	
	$result = mysql_query($query,$link);
	$trackbacks = array();
	while($rows = mysql_fetch_array($result))
	{
		$trackbacks[] = array(
					"URL" => htmlspecialchars(stripslashes($rows[url])),
					"TITLE" => htmlspecialchars(stripslashes($rows[title])),
					"TIME" => time_format($rows[time])
					);
	}
	mysql_free_result($result);
	return $trackbacks;
}

function pc_load_comments($link,$pc,$pur=0)
{
	$query = "SELECT cid , comments.subject , comments.created , comments.username , comments.nid FROM comments, nodes WHERE comments.nid = nodes.nid ";
	if($pur == 0)
		$query .= " AND access = 0 ";
	elseif($pur == 1)
		$query .= " AND ( access = 0 OR access = 1 ) ";
	elseif($pur == 3)
		$query .= " AND ( `access` = 0 OR `access` = 1 OR `access` = 2 OR `access` = 3 ) ";
	$query .= " AND comments.uid = ".$pc["UID"]." AND comment = 1 ORDER BY cid DESC LIMIT 0 , 10 ;";
	$result = mysql_query($query,$link);
	$comments = array();
	for($i = 0;$i < mysql_num_rows($result) ; $i++)
	{
		$rows = mysql_fetch_array($result);
		$comments[] = array(
					"USER" => $rows[username],
					"CID" => $rows[cid],
					"NID" => $rows[nid],
					"SUBJECT" => htmlspecialchars(stripslashes($rows[subject])),
					"TIME" => time_format($rows[created])
					);
	}
	mysql_free_result($result);
	return $comments;
}

function pc_load_calendar($link,$pc,$pur)
{
	global $totalnodes;
	$query = "SELECT `nid` , `created` FROM nodes WHERE `uid` = '".$pc["UID"]."' AND `type` = 0 ";
	if($pur == 0)
		$query .= " AND `access` = 0 ";
	elseif($pur == 1)
		$query .= " AND ( `access` = 0 OR `access` = 1 ) ";
	elseif($pur == 3)
		$query .= " AND ( `access` = 0 OR `access` = 1 OR `access` = 2 OR `access` = 3 ) ";
	$query .= " ORDER BY `nid` DESC;";
	$result = mysql_query($query,$link);
	$bc = array();
	$totalnodes = mysql_num_rows($result);
	while($rows = mysql_fetch_array($result))
	{
		if(!$bc[substr($rows[created],0,8)])
		{
			$bc[substr($rows[created],0,8)] = $rows[nid] ;
		}
		else
			continue;
	}
	mysql_free_result($result);
	return $bc;
}

function pc_load_archfile($pc)
{
	$startYear = (int)(time_mysql($pc["CREATED"]) / 10000000000);
	$startMonth = (int)((time_mysql($pc["CREATED"])-$startYear*10000000000) / 100000000);	
	$thisYear = date("Y");
	$thisMonth = date("m");
	$archfile = array();
	for($yy=$thisYear ; $yy >= $startYear ; $yy --)
	{
		$firstMonth = ($yy == $startYear)?$startMonth:1;
		for($mm = $thisMonth ; $mm >= $firstMonth  ; $mm --)
		{
			$archfile[] = array("YEAR" => $yy , "MONTH" => $mm );
		}
		$thisMonth = 12;	
	}
	return $archfile;
}

$userid = addslashes($_GET["id"]);
$uid = (int)($_GET["id"]);
$pno = (int)($_GET["pno"]);

$link = pc_db_connect();
$pc = pc_load_infor($link,$userid,$uid);
if(!$pc)
{
	pc_db_close($link);
	html_init("gb2312",$pcconfig["BBSNAME"]."Blog");		
	html_error_quit("对不起，您要查看的Blog不存在");
	exit();
}
$stylesheet = pc_load_stylesheet($link,$pc);

$userPermission = pc_get_user_permission($currentuser,$pc);
$sec = $userPermission["sec"];
$pur = $userPermission["pur"];
$tags = $userPermission["tags"];

if($pur != 3)
	pc_counter($link);

$nodes = pc_load_nodes($link,$pc,$pur,$pno);   //文章
$blogs = pc_blog_menu($link,$pc,0);            //分类
$comments = pc_load_comments($link,$pc,$pur);  //评论
$bc = pc_load_calendar($link,$pc,$pur);        //日历
$trackbacks = pc_load_trackbacks($link,$pc);   //引用
$archfiles = pc_load_archfile($pc);            //归档
pc_db_close($link);

header("Content-Type: text/xml");
header("Content-Disposition: inline;filename=SMTHBlog_".$pc["USER"].".xml");
echo '<?xml version="1.0" encoding="GB2312"?>';
	if( $stylesheet === 0 )
	{
	echo '<?xml-stylesheet type="text/xsl" href="/pc/indexxsl.php?id='.$pc["USER"].'"?>';
	}
	elseif( $stylesheet === 1 )
	{
	echo '<?xml-stylesheet type="text/css" href="/pc/indexxsl.php?id='.$pc["USER"].'"?>';
	}
?>
<!-- Edited by windinsn@smth.org -->
<rdf:RDF xmlns:smthBlog="http://www.smth.org/blog/ns/1.0/" 
	 xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" 
	 xmlns:dc="http://purl.org/dc/elements/1.1/" 
	 xmlns:sy="http://purl.org/rss/1.0/modules/syndication/" 
	 xmlns:co="http://purl.org/rss/1.0/modules/company/" 
	 xmlns:ti="http://purl.org/rss/1.0/modules/textinput/" 
	 xmlns="http://purl.org/rss/1.0/"
>
<smthBlog:viewMode>normal</smthBlog:viewMode>
<channel rdf:about="http://<?php echo $pcconfig["SITE"]; ?>">
<title><?php echo $pc["NAME"]; ?></title>
<link><?php echo pc_personal_domainname($pc["USER"]); ?></link>
<description><?php echo $pc["DESC"]; ?></description>
<image rdf:resource="<?php echo html_format($pc["LOGO"]); ?>"/>
<dc:language>gb2312</dc:language>
<dc:creator><?php echo $pc["USER"]; ?></dc:creator>
<items>
	<rdf:Seq>
		<rdf:li resource="index.html"/>
		<rdf:li resource="<?php echo pc_personal_domainname($pc["USER"]); ?>"/>
	</rdf:Seq>
</items>
<smthBlog:calendars>
<?php
	$bcdates = array_keys($bc);
	foreach( $bcdates as $bcdate )
	{
		$bcdatestr = $bcdate."";
?>
	<smthBlog:calendar>
		<smthBlog:year><?php echo $bcdatestr[0].$bcdatestr[1].$bcdatestr[2].$bcdatestr[3]; ?></smthBlog:year>
		<smthBlog:month><?php echo $bcdatestr[4].$bcdatestr[5]; ?></smthBlog:month>
		<smthBlog:day><?php echo $bcdatestr[6].$bcdatestr[7]; ?></smthBlog:day>
		<smthBlog:id><?php echo $bc[$bcdate]; ?></smthBlog:id>
		<smthBlog:link><?php echo "pccon.php?id=".$pc["UID"]."&amp;nid=".$bc[$bcdate]."&amp;s=all"; ?></smthBlog:link>
	</smthBlog:calendar>
<?php
	}
?>
</smthBlog:calendars>

<smthBlog:categories>
<?php
	foreach( $blogs as $blog )
	{
		if($blog["TAG"] == 9) continue;
?>
	<smthBlog:category>
		<smthBlog:title><?php echo htmlspecialchars(stripslashes($blog["NAME"])); ?></smthBlog:title>
		<smthBlog:link><?php echo "pcdoc.php?userid=".$pc["USER"]."&amp;tag=".$blog["TAG"]."&amp;tid=".$blog["TID"]; ?></smthBlog:link>
	</smthBlog:category>
<?php
	}
?>
</smthBlog:categories>

<smthBlog:newNodes>
<?php
	foreach( $nodes as $node )
	{
?>
	<smthBlog:newNode>
		<smthBlog:subject><?php echo htmlspecialchars(stripslashes($node[subject])); ?></smthBlog:subject>
		<smthBlog:time><?php echo time_format($node[created]); ?></smthBlog:time>
		<smthBlog:id><?php echo $node[nid]; ?></smthBlog:id>
		<smthBlog:link><?php echo "pccon.php?id=".$pc["UID"]."&amp;nid=".$node[nid]."&amp;s=all"; ?></smthBlog:link>
	</smthBlog:newNode>
<?php
	}
?>
</smthBlog:newNodes>

<smthBlog:newComments>
<?php
	foreach( $comments as $comment )
	{
?>
	<smthBlog:newComment>
		<smthBlog:subject><?php echo $comment["SUBJECT"]; ?></smthBlog:subject>
		<smthBlog:user><?php echo htmlspecialchars(stripslashes($comment["USER"])); ?></smthBlog:user>
		<smthBlog:id><?php echo $comment["CID"]; ?></smthBlog:id>
		<smthBlog:nid><?php echo $comment["NID"]; ?></smthBlog:nid>
		<smthBlog:time><?php echo $comment["TIME"]; ?></smthBlog:time>
		<smthBlog:link><?php echo "pcshowcom.php?cid=".$comment["CID"]; ?></smthBlog:link>
	</smthBlog:newComment>
<?php
	}
?>
</smthBlog:newComments>

<smthBlog:newTrackbacks>
<?php
	foreach( $trackbacks as $trackback )
	{
?>
	<smthBlog:newTrackback>
		<smthBlog:title><?php echo $trackback["TITLE"]; ?></smthBlog:title>
		<smthBlog:link><?php echo $trackback["URL"]; ?></smthBlog:link>
		<smthBlog:time><?php echo $trackback["TIME"]; ?></smthBlog:time>
	</smthBlog:newTrackback>
<?php
	}
?>
</smthBlog:newTrackbacks>

<smthBlog:archfiles>
<?php
	foreach( $archfiles as $archfile )
	{
?>
	<smthBlog:archfile>
		<smthBlog:year><?php echo $archfile["YEAR"]; ?></smthBlog:year>
		<smthBlog:month><?php echo $archfile["MONTH"]; ?></smthBlog:month>
	</smthBlog:archfile>
<?php
	}
?>
</smthBlog:archfiles>

<smthBlog:friends>
<?php
	for($i = 0 ; $i < count($pc["LINKS"]) ; $i ++)
	{
?>
	<smthBlog:friend>
		<smthBlog:image><?php echo intval($pc["LINKS"][$i]["IMAGE"]); ?></smthBlog:image>
		<smthBlog:title><?php echo htmlspecialchars($pc["LINKS"][$i]["LINK"]); ?></smthBlog:title>
		<smthBlog:link><?php echo htmlspecialchars($pc["LINKS"][$i]["URL"]); ?></smthBlog:link>
	</smthBlog:friend>
<?php
	}
?>
</smthBlog:friends>

<smthBlog:formSearch>
	<![CDATA[
		<form action="pcnsearch.php" method="get" onsubmit="if(this.keyword.value==''){alert('请输入关键字');return false;}" />
			<input type="text" name="keyword" id="keyword" class="blogFormInput" />
			<input type="submit" class="blogFormButton" value="Search" />
		</form>
	]]>
</smthBlog:formSearch>


<smthBlog:formLogin>
	<![CDATA[
		<form action="/bbslogin.php?mainurl=/pc/index.php?id=<?php echo $pc["USER"]; ?>" method="post" target="_top" />
			<input type="text" class="blogFormInput" name="id" size="12" onMouseOver="this.focus()" onFocus="this.select()" />
			<INPUT type="password"  class="blogFormInput" size="12" name="passwd" maxlength="39" />
			<input type="submit" class="blogFormButton"  value="Login" />
		</form>
	]]>
</smthBlog:formLogin>

<smthBlog:totalNodes><?php echo $totalnodes; ?></smthBlog:totalNodes>
<smthBlog:totalHits><?php echo $pc["VISIT"]; ?></smthBlog:totalHits>
<smthBlog:rssLink><?php echo "rss.php?userid=".$pc["USER"]; ?></smthBlog:rssLink>
<smthBlog:klipLink><?php echo "klip.php?id=".$pc["USER"]; ?></smthBlog:klipLink>
</channel>

<?php
	for($i = 0 ; $i < min(count($nodes),$pc["INDEX"]["nodeNum"]) ; $i ++ )
	{
?>
<item rdf:about="pccon.php?id=<?php echo $pc["UID"]; ?>&amp;nid=<?php echo $nodes[$i][nid]; ?>&amp;s=all">
	<title><?php echo htmlspecialchars(stripslashes($nodes[$i][subject])); ?></title>
	<link>pccon.php?id=<?php echo $pc["UID"]; ?>&amp;nid=<?php echo $nodes[$i][nid]; ?>&amp;s=all</link>
	<dc:creator><?php echo $pc["USER"]; ?></dc:creator>
	<dc:date><?php echo time_format($nodes[$i][created]); ?></dc:date>
	<description><![CDATA[
		<?php echo html_format($nodes[$i][body],TRUE,$nodes[$i][htmltag]); ?>
		]]></description>
	<smthBlog:node>
		<smthBlog:id><?php echo $nodes[$i][nid]; ?></smthBlog:id>
		<smthBlog:emote><?php echo $nodes[$i][emote]; ?></smthBlog:emote>
		<smthBlog:address><?php echo pc_hide_ip($nodes[$i][hostname]); ?></smthBlog:address>
		<smthBlog:view><?php echo $nodes[$i][visitcount]; ?></smthBlog:view>
		<smthBlog:reply><?php echo $nodes[$i][commentcount]; ?></smthBlog:reply>
		<smthBlog:quote><?php echo $nodes[$i][trackbackcount]; ?></smthBlog:quote>
	</smthBlog:node>
</item>
<?php
	}
?>
</rdf:RDF>
