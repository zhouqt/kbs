<?php
	/**
	 * This file mail message log to current user.
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
		if (bbs_mailwebmsgs() == false)
		{
			html_error_quit("讯息备份寄回信箱失败");
		}
?>
<body>
讯息备份已经寄回您的信箱<a href="javascript:history.go(-2)">返回</a>
<?php
		html_normal_quit();
	}
?>
