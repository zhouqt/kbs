<?php
	/**
	 * This file lists boards to user.
	 * $Id$
	 */
	require("funcs.php");
	if ($loginok !=1 )
		html_nologin();
	else
	{
		html_init("gb2312");
        if (!strcasecmp($currentuser["userid"],"guest")) {
			html_error_quit("guest不能使用隐身");
        }
        if (!($currentuser["userlevel"]&BBS_PERM_CLOAK)) {
			html_error_quit("错误的参数");
        }
?>
<body>
<?
		bbs_update_uinfo("invisible", !$currentuinfo["invisible"]);
        if (!$currentuinfo["invisible"])
            echo("开始隐身!");
        else
            echo("隐身已经停止!");
?>
<br/>
<a href="javascript:history.go(-1)">快速返回</a>
<?php
		html_normal_quit();
	}
?>

