<?php
	/**
	 * This file delete current user's message log.
	 * $Id$
	 */
	require("funcs.php");
	if ($loginok != 1)
		html_nologin();
	else
	{
		html_init("gb2312");
        if ($currentuser["userid"]=="guest")
			html_error_quit("匆匆过客不能处理讯息，请先登录");
		$filename = bbs_sethomefile($currentuser["userid"],"msgfile");
		if (file_exists($filename))
			unlink($filename);
		$filename = bbs_sethomefile($currentuser["userid"], "msgcount");
		if (file_exists($filename))
			unlink($filename);
?>
<body>
已经删除所有讯息备份<a href="javascript:history.go(-2)">返回</a>
<?php
		html_normal_quit();
	}
?>
