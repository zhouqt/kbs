<?php
require("pcadmin_inc.php");
pc_admin_check_permission();
$link = pc_db_connect();

$blogadmin = intval($_COOKIE["BLOGADMIN"]);

if( $_GET["act"] == "login" && !$blogadmin )
{
	$blogadmin = time();
	$action = $currentuser[userid]." 登录Blog管理员";
	$comment = $currentuser[userid]." 于 ".date("Y-m-d H:i:s",$blogadmin)." 自 ".$_SERVER["REMOTE_ADDR"]." 登录本站Blog管理员。";
	pc_logs($link , $action , $comment);
	setcookie("BLOGADMIN" , $blogadmin);
}
if( $_GET["act"] == "logout" && $blogadmin)
{
	$action = $currentuser[userid]." 退出Blog管理员登录";
	$comment = $currentuser[userid]." 于 ".date("Y-m-d H:i:s")." 自 ".$_SERVER["REMOTE_ADDR"]." 退出本站Blog管理员登录。历时 ".intval((time() - $blogadmin) / 60)." 分钟。";
	pc_logs($link , $action , $comment);
	unset($blogadmin);
	setcookie("BLOGADMIN");
}

pc_html_init("gb2312",$pcconfig["BBSNAME"]."Blog管理员登录");
pc_admin_navigation_bar();
?>
<br/><br/><br/><br/>
<?php
if( $blogadmin )
{
?>
<p align="center">
用户名：<font color=red><?php echo $currentuser[userid]; ?></font><br/>
登录时间：<font color=red><?php echo date("Y-m-d H:i:s",$blogadmin); ?></font><br/>
登录IP：<font color=red><?php echo $currentuser[lasthost]; ?></font><br/>
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