<?php
require("pcfuncs.php");

$nid = intval($_GET["id"]);
$link = pc_db_connect();
$query = "SELECT * FROM nodes WHERE type = 0 AND nid = ".$nid." LIMIT 0 , 1;";
$result = mysql_query($query,$link);
$node = mysql_fetch_array($result);
mysql_free_result($result);

if(!$node)
{
	pc_db_close($link);
	html_init("gb2312",$pcconfig["BBSNAME"]."Blog");		
	html_error_quit("对不起，您要查看的文章不存在");
	exit();
}

$pc = pc_load_infor($link,"",$node[uid]);

if(!$pc)
{
	pc_db_close($link);
	html_init("gb2312","Blog");		
	html_error_quit("对不起，您要查看的Blog不存在");
	exit();
}

$userPermission = pc_get_user_permission($currentuser,$pc);
$pur = $userPermission["pur"];
$tags = $userPermission["tags"];

if(!$tags[$node[access]])
{
	pc_html_init("gb2312",$pc["NAME"],"","",$pc["BKIMG"]);
	html_error_quit("对不起，您无权查看该文章!");
	exit();
}

if($pur != 3)
	pc_counter($link);

$blogs = pc_blog_menu($link,$pc["UID"],$node[access]);

if($node[comment])
{
	$comments = array();
	$query = "SELECT * FROM comments WHERE nid = ".$node[nid].";";
	$result = mysql_query($query,$link);
	while($rows = mysql_fetch_array($result))
		$comments[] = $rows;
	mysql_free_result($result);
}

if($node[trackback] && $node[access] == 0)
{
	$trackbacks = array();
	$query = "SELECT * FROM trackback WHERE nid = ".$node[nid].";";
	$result = mysql_query($query,$link);
	while($rows = mysql_fetch_array($result))
		$trackbacks[] = $rows;
	mysql_free_result($result);
}

pc_db_close($link);

header("Content-Type: text/xml");
header("Content-Disposition: inline;filename=SMTHBlog_".$pc["USER"]."_".html_format($node[subject]).".xml");
?>
<?xml version="1.0" encoding="GB2312"?>
<?xml-stylesheet type="text/xsl" href="/pc/nodexsl.php?id=<?php echo $pc["USER"]; ?>"?>
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
<image rdf:resource="<?php echo $pc["LOGO"]; ?>"/>
<dc:language>gb2312</dc:language>
<dc:creator><?php echo $pc["USER"].".bbs@".$pcconfig["SITE"]; ?></dc:creator>
<items>
	<rdf:Seq>
		<rdf:li resource="pcmain.php"/>
		<rdf:li resource="<?php echo pc_personal_domainname($pc["USER"]); ?>"/>
		<rdf:li resource="nodexml.php?id=<?php echo $node[nid]; ?>"/>
	</rdf:Seq>
</items>

<smthBlog:categories>
<?php
	foreach( $blogs as $blog )
	{
		if($blog["TAG"] == 9) continue;
?>
	<smthBlog:category>
		<smthBlog:title><?php echo html_format($blog["NAME"]); ?></smthBlog:title>
		<smthBlog:link><?php echo "pcdoc.php?userid=".$pc["USER"]."&amp;tag=".$blog["TAG"]."&amp;tid=".$blog["TID"]; ?></smthBlog:link>
	</smthBlog:category>
<?php
	}
?>
</smthBlog:categories>

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
<smthBlog:totalHits><?php echo $pc["VISIT"]; ?></smthBlog:totalHits>
<smthBlog:rssLink><?php echo "rss.php?userid=".$pc["USER"]; ?></smthBlog:rssLink>
<smthBlog:klipLink><?php echo "klip.php?id=".$pc["USER"]; ?></smthBlog:klipLink>


<?php
if($node[comment])
{
?>
<smthBlog:comments>
<?php
	foreach($comments as $comment)
	{
?>
	<smthBlog:comment>
		<smthBlog:id><?php echo $comment[cid]; ?></smthBlog:id>
		<smthBlog:subject><?php echo html_format($comment[subject]); ?></smthBlog:subject>
		<smthBlog:time><?php echo rss_time_format($comment[created]); ?></smthBlog:time>
		<smthBlog:user><?php echo html_format($comment[username]); ?></smthBlog:user>
		<smthBlog:link><?php echo "pcshowcom.php?cid=".$comment[cid]; ?></smthBlog:link>
		<smthBlog:emote><?php echo $comment[emote]; ?></smthBlog:emote>
		<smthBlog:address><?php echo $comment[hostname]; ?></smthBlog:address>
		<smthBlog:body>
			<![CDATA[
			<?php echo html_format($comment[body],TRUE,$comment[htmltag]); ?>
			]]>
		</smthBlog:body>
	</smthBlog:comment>
<?php
	}
?>
</smthBlog:comments>
<?php
}


if($node[trackback] && $node[access] == 0)
{
?>
<smthBlog:trackbacks>
<?php
	foreach($trackbacks as $trackback)
	{
?>
	<smthBlog:trackback>
		<smthBlog:title><?php echo html_format($trackback[title]); ?></smthBlog:title>
		<smthBlog:name><?php echo html_format($trackback[blogname]); ?></smthBlog:name>
		<smthBlog:link><?php echo html_format($trackback[url]); ?></smthBlog:link>
		<smthBlog:excerpt>
			<![CDATA[
			<?php echo html_format($trackback[excerpt],TRUE); ?>
			]]>
		</smthBlog:excerpt>
	</smthBlog:trackback>
<?php
	}
?>
</smthBlog:trackbacks>
<?php
}
?>
</channel>

<item rdf:about="nodexml.php?id=<?php echo $node[nid]; ?>">
	<title><?php echo html_format($node[subject]); ?></title>
	<link>"pccon.php?nid=<?php echo $node[nid]; ?>&amp;id=<?php echo $pc["UID"]; ?>&amp;s=all"</link>
	<dc:creator><?php echo $pc["USER"]; ?></dc:creator>
	<dc:date><?php echo rss_time_format($node[created]); ?></dc:date>
	<description><![CDATA[
		<?php echo html_format($node[body],TRUE,$node[htmltag]); ?>
		]]></description>
	<smthBlog:node>
		<smthBlog:id><?php echo $node[nid]; ?></smthBlog:id>
		<smthBlog:emote><?php echo $node[emote]; ?></smthBlog:emote>
		<smthBlog:address><?php echo $node[hostname]; ?></smthBlog:address>
		<smthBlog:view><?php echo $node[visitcount]; ?></smthBlog:view>
		<smthBlog:reply><?php echo $node[commentcount]; ?></smthBlog:reply>
		<smthBlog:quote><?php echo $node[trackbackcount]; ?></smthBlog:quote>
	</smthBlog:node>
</item>
</rdf:RDF>