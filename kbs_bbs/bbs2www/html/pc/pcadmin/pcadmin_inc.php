<?php
require("pcfuncs.php");
function pc_admin_check_permission()
{
	global $loginok , $currentuser , $pcconfig ;
	if ($loginok != 1)
		html_nologin();
	elseif(!strcmp($currentuser["userid"],"guest"))
	{
		html_init("gb2312");
		html_error_quit("对不起，请先登录");
		exit();
	}
	elseif(!pc_is_manager($currentuser))
	{
		html_init("gb2312");
		html_error_quit("对不起，您无权访问该页");
		exit();
	}
	else
		return;
}

function pc_admin_navigation_bar()
{
?>
<p align="center">
[
Blog管理员工具:
<a href="index.html">返回首页</a>
<a href="pcadmin_rec.php">推荐文章管理</a>
<a href="pcadmin_bla.php">黑名单管理</a>
<a href="pcadmin_app.php">处理申请</a>
<a href="pcadmin_grp.php">特殊BLOG管理</a>
<a href="pcdoc.php?userid=_filter">过滤器</a>
]
</p>
<?php
}

?>
