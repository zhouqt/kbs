<?php
	/**
	 * This file post the article to the board.
	 * $Id$
	 */
	 
	require("funcs.php");
	require("boards.php");
		
	if ($loginok != 1)
		html_nologin();
	else
	{
		html_init("gb2312");
		if( !isset( $_GET["board"] )) {
			html_error_quit("未指定发文版面!");
		}
		$boardName = $_GET["board"];
		$brdArr=array();
		$boardID= bbs_getboard($boardName,$brdArr);
		if( $boardID == 0) html_error_quit("指定的版面不存在!");
		$usernum = $currentuser["index"];
		if (bbs_checkreadperm($usernum, $boardID) == 0) html_error_quit("您无权阅读该版!");
		if (bbs_is_readonly_board($boardArr)) html_error_quit("本版为只读讨论区!");
		if (bbs_checkpostperm($usernum, $boardID) == 0) html_error_quit("您无权在该版面发文!");
		
		if (!isset($_POST["title"])) html_error_quit("没有指定文章标题!");
		if (!isset($_POST["text"])) html_error_quit("没有指定文章内容!");
		if (!isset($_POST["reid"])) $reID = $_GET["reid"];   
		else
			$reID = 0;
		
		settype($reID, "integer");
				
		$articles = array();
		if ($reID > 0) {
			$num = bbs_get_records_from_id($boardName, $reID, $dir_modes["NORMAL"], $articles);
			if ($num == 0) html_error_quit("错误的Re文编号!");
			if ($articles[1]["FLAGS"][2] == 'y') html_error_quit("该文不可回复!");
		}
		
		//post articles
		$ret = bbs_postarticle($boardName, preg_replace("/\\\(['|\"|\\\])/","$1",$_POST["title"]), 
			preg_replace("/\\\(['|\"|\\\])/","$1",$_POST["text"]), intval($_POST["signature"]), $reID, 
			intval($_POST["outgo"]), intval($_POST["anony"]));
		switch ($ret) {
			case -1:
				html_error_quit("错误的讨论区名称!");
				break;
			case -2: 
				html_error_quit("本版为二级目录版!");
				break;
			case -3: 
				html_error_quit("标题为空!");
				break;
			case -4: 
				html_error_quit("此讨论区是唯读的, 或是您尚无权限在此发表文章!");
				break;		
			case -5:	
				html_error_quit("很抱歉, 你被版务人员停止了本版的post权利!");
				break;	
			case -6:
				html_error_quit("两次发文间隔过密,请休息几秒再试!");	
				break;
			case -7: 
				html_error_quit("无法读取索引文件! 请通知站务人员, 谢谢! ");
				break;
			case -8:
				html_error_quit("本文不可回复!");
				break;
			case -9:
				html_error_quit("系统内部错误, 请迅速通知站务人员, 谢谢!");
				break;
		}
?>		
<table cellpadding=3 cellspacing=1 align=center class=TableBorder1>
<tr align=center><th width="100%">发文成功！</td>
</tr><tr><td width="100%" class=TableBody1>
本页面将在3秒后自动返回版面文章列表<meta HTTP-EQUIV=REFRESH CONTENT='3; URL=bbsdoc.php?board=<?php echo $boardName; ?>' >，<b>您可以选择以下操作：</b><br><ul>
<li><a href="mainpage.php">返回首页</a></li>
<li><a href="bbsdoc.php?board=<?php   echo $boardName; ?>">返回<?php   echo $boardArr['DESC']; ?></a></li>
</ul></td></tr></table>

<?php
	html_normal_quit();
	}
?>	
