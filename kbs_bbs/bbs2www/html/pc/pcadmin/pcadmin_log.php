<?php
session_start();
require("pcadmin_inc.php");
pc_admin_check_permission();
$link = pc_db_connect();

$blogadmin = $_SESSION["blogadmin"];
if( $_GET["act"] == "login" && !session_is_registered($blogadmin) )
{
	$blogadmin = array(
			"user" => $currentuser,
			"logintime" => time(),
			"hostname" => $_SERVER["REMOTE_ADDR"]
			);
	session_register($blogadmin);
	$action = $currentuser[userid]." 登录Blog管理员";
	$comment = $currentuser[userid]." 于 ".date("Y-m-d H:i:s",$blogadmin["logintime"])." 自 ".$_SERVER["REMOTE_ADDR"]." 登录本站Blog管理员。";
	pc_logs($link , $action , $comment);
}
if( $_GET["act"] == "logout" && session_is_registered($blogadmin))
{
	$action = $currentuser[userid]." 退出Blog管理员登录";
	$comment = $currentuser[userid]." 于 ".date("Y-m-d H:i:s")." 自 ".$_SERVER["REMOTE_ADDR"]." 退出本站Blog管理员登录。历时 ".intval((time() - $blogadmin["logintime"]) / 60)." 分钟。";
	pc_logs($link , $action , $comment);
	session_unregister($blogadmin);
}

pc_html_init("gb2312",$pcconfig["BBSNAME"]."Blog管理员登录");
?>
<br/><br/><br/><br/>
<?php
if( session_is_registered($blogadmin) )
{
?>
<p align="center">
用户名：<font color=red><?php echo $blogadmin["user"]["userid"]; ?></font><br/>
登录时间：<font color=red><?php echo date("Y-m-d H:i:s",$blogadmin["logintime"]); ?></font><br/>
登录IP：<font color=red><?php echo $blogadmin["hostname"]; ?></font><br/>
<br/><br/><a href="pcadmin_log.php?act=logout">退出管理员登录</a>
</p>
<?php	
}
else
{
?>
<p align="center"><a href="pcadmin_log.php?act=login">管理员登录</a></p>
<?php	
}
pc_db_close($link);
pc_admin_navigation_bar();
html_normal_quit();
?>