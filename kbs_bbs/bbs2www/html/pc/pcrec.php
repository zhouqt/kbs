<?php
/*
**  recommend articles
**  id:windinsn feb 28 , 2004
*/
require("pcfuncs.php");

if ($loginok != 1)
	html_nologin();

pc_html_init("gb2312","推荐日志");

if(!strcmp($currentuser["userid"],"guest"))
{
	html_error_quit("guest 不能推荐文章!");
	exit();
}

$nid = $_GET["nid"];
$nid = intval( $nid );
$link = pc_db_connect();
    $query = "SELECT recommend , uid , theme  FROM nodes WHERE access = 0 AND type = 0 AND nid = ".$nid." LIMIT 0 , 1;";
    $result = mysql_query($query , $link);
    $node = mysql_fetch_array($result);
    
    if(!$node)
    {
    	html_error_quit("对不起，您要推荐的文章不存在");
    	exit();
    }
    if($node[recommend] != 0)
    {
    	html_error_quit("对不起，该文已被推荐");
    	exit();
    }

if (!isset($_GET['topic']))
{
?>   
<br /><br />
<p align="center">
<form aciton="<?php echo $_SERVER['PHP_SELF']; ?>" method="get" >
<input type="hidden" name="nid" value="<?php echo $nid; ?>" />
推荐类型
<select name="topic" class="f1">
<?php
    while(list($key,$value)=each($pcconfig["SECTION"])) {
        if ($key == $node[theme])
            echo '<option value="'.$key.'" selected>'.htmlspecialchars($value).'</option>';
        else
            echo '<option value="'.$key.'">'.htmlspecialchars($value).'</option>';
    }
?>
</select>
<input type="submit" value="推荐" class="f1" />
<input type="button" value="返回" class="f1" onclick="history.go(-1);" />
</form>
</p> 
<?php    
}
else
{
    
/*
if(!pc_is_manager($currentuser))
{
	$pc = pc_load_infor($link,"",$node[uid]);	
	if(!$pc || !pc_is_admin($currentuser,$pc))
	{
		html_error_quit("对不起，您无权推荐本文");
		exit();	
	}
}
*/
    $topic = $_GET['topic'];
    if (!$pcconfig["SECTION"][$topic]) $topic = 'others';
    $query = "UPDATE nodes SET rectopic = '".addslashes($topic)."' , changed  = changed , recommend = 1, recuser = '".addslashes($currentuser["userid"])."' WHERE nid = ".$nid." ;";
    mysql_query($query,$link);
?>
<br /><br />
<p align="center">
推荐成功，单击返回文章。
<br /><br />
<a href="pccon.php?id=<?php echo $node[uid]; ?>&nid=<?php echo $nid; ?>">[返回]</a>
</p>
<?php
}
pc_db_close($link);
html_normal_quit();
?>