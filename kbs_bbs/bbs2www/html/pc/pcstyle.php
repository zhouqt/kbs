<?php
require("pcfuncs.php");

if ($loginok != 1)
	html_nologin();
elseif(!strcmp($currentuser["userid"],"guest"))
{
	html_init("gb2312");
	html_error_quit("guest 没有Blog!");
	exit();
}
else
{
	$link = pc_db_connect();
	$pc = pc_load_infor($link,$_GET["userid"]);
		
	if(!$pc)
	{
		pc_db_close($link);
		html_error_quit("对不起，您要查看的Blog不存在");
		exit();
	}
	
	if(!pc_is_admin($currentuser,$pc))
	{
		pc_db_close($link);
		html_error_quit("对不起，您要查看的Blog不存在");
		exit();
	}
	
	if($_POST["key"]=="indexxsl" || $_POST["key"]=="nodexsl" ||  $_POST["key"]=="css" )
	{
		$query = "SELECT uid FROM userstyle WHERE uid = ".$pc["UID"].";";
		$result = mysql_query($query,$link);
		$rows = mysql_fetch_array($result);
		mysql_free_result($result);
		if($rows)
			$query = "UPDATE userstyle SET `".$_POST["key"]."` = '".addslashes(undo_html_format($_POST["userstyle"]))."' , `hostname` = '".addslashes($_SERVER["REMOTE_ADDR"])."' , `stylesheet` = '".intval($_POST["stylesheet"])."' WHERE uid = ".$pc["UID"]." ;";
		else
			$query = "INSERT INTO `userstyle` ( `uid` , `username` , `hostname` , `changed` , `".$_POST["key"]."` , `stylesheet` ) ".
				"VALUES ('".$pc["UID"]."', '".addslashes($pc["USER"])."', '".addslashes($_SERVER["REMOTE_ADDR"])."', NOW( ) , '".addslashes(undo_html_format($_POST["userstyle"]))."' , '".intval($_POST["stylesheet"])."');";
		mysql_query($query,$link);
	}
	
	$type = $_POST["type"]?intval($_POST["type"]):intval($_GET["type"]);
	switch($type)
	{
		case 1:
			$query = "SELECT indexxsl,changed,hostname,stylesheet FROM userstyle WHERE uid = ".$pc["UID"]." ;";
			$title = "Blog首页样式表";
			$key = "indexxsl";
			break;
		case 2:
			$query = "SELECT nodexsl,changed,hostname,stylesheet FROM userstyle WHERE uid = ".$pc["UID"]." ;";
			$title = "Blog文章样式表";
			$key = "nodexsl";
			break;
		default:
			$query = "SELECT css,changed,hostname,stylesheet FROM userstyle WHERE uid = ".$pc["UID"]." ;";
			$title = "Blog CSS样式";
			$key = "css";
	}	
	
	pc_html_init("gb2312",$title);
	
	$result = mysql_query($query,$link);
	$rows = mysql_fetch_array($result);
	mysql_free_result($result);
?>
<br /><br />
<p align="center">
[<a href="pcstyle.php?userid=<?php echo $pc["USER"]; ?>&type=1">首页样式表</a>]&nbsp;&nbsp;
[<a href="pcstyle.php?userid=<?php echo $pc["USER"]; ?>&type=2">文章样式表</a>]&nbsp;&nbsp;
[<a href="pcstyle.php?userid=<?php echo $pc["USER"]; ?>">CSS样式</a>]
</p>
<p align="center">
编辑<b><font color=red><?php echo $title; ?></font></b>&nbsp;&nbsp;
<?php
if($rows){
?>
上次保存时间:<?php echo time_format($rows[changed]); ?>&nbsp;&nbsp;
上次保存地点:<?php echo html_format($rows[hostname]); ?>
<?php } ?>
</p>
<form action="pcstyle.php?userid=<?php echo $pc["USER"]; ?>" method="post" name="postform"><center>
浏览XML文档所用的样式表：
<input type="radio" name="stylesheet" value="0" <?php if($rows[stylesheet]==0) echo "checked"; ?>>XSL样式表
<input type="radio" name="stylesheet" value="1" <?php if($rows[stylesheet]==1) echo "checked"; ?>>CSS样式表
<br />
<textarea name="userstyle" class="f1" onkeydown='if(event.keyCode==87 && event.ctrlKey) {document.postform.submit(); return false;}'  onkeypress='if(event.keyCode==10) return document.postform.submit()' rows="20" cols="80" wrap="physical">
<?php echo html_format($rows[$key]); ?>
</textarea></center>
<input type="hidden" name="key" value="<?php echo $key; ?>">
<input type="hidden" name="type" value="<?php echo $type; ?>">
<p align="center"><input type="submit" class="f1" value="修改文件"></center>
</form>
<p align="center">
[<a href="index.html">Blog首页</a>]
[<a href="index.php?id=<?php echo $pc["USER"]; ?>">首页</a>]
[<a href="pcdoc.php?userid=<?php echo $pc["USER"]; ?>&tag=7">参数设定</a>]
</p>
<?php
	pc_db_close($link);
	html_normal_quit();
}
?>