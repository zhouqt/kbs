<?php
/*
**  为水木清华blog申请定制的申请表
**  满足条件的申请表将被提交至$pcconfig["APPBOARD"]
**  并在数据表newapply中保留申请资料
**  @windinsn Mar 12 , 2004
*/
require("pcfuncs.php");

/*
** management字段：0通过；1待处理；2不通过；3以后不得申请
*/
function pc_app_have_applied($link , $currentuser)
{
	$query = "SELECT management FROM newapply WHERE username = '".$currentuser["userid"]."' AND management != 2 LIMIT 0 , 1;";
	$result = mysql_query($query,$link);
	$rows = mysql_fetch_array($result);
	mysql_free_result($result);
	if( $rows) 
		return $rows[management];
	else
		return FALSE;
}

if ($loginok != 1)
	html_nologin();
elseif(!strcmp($currentuser["userid"],"guest"))
{
	html_init("gb2312");
	html_error_quit("请登录后再进行Blog申请!");
	exit();
}
else
{
	if( time() - $currentuser["firstlogin"] < intval( $pcconfig["MINREGTIME"] * 2592000 ) )
	{
		html_init("gb2312");
		html_error_quit("对不起，您的注册时间尚不足".$pcconfig["MINREGTIME"]."个月");
		exit();	
	}
	if( !$_POST["appname"] || !$_POST["appself"] || !$_POST["appdirect"] )
	{
		html_init("gb2312");
		html_error_quit("对不起，请详细填写Blog申请表");
		exit();	
	}
	
	$link = pc_db_connect();
	if( pc_load_infor($link,$currentuser["userid"]) )
	{
		pc_db_close($link);
		html_init("gb2312");
		html_error_quit("对不起，您已经拥有Blog了");
		exit();	
	}
	
	$appstate = pc_app_have_applied($link , $currentuser);
	if($appstate === 0 )
	{
		pc_db_close($link);
		html_init("gb2312");
		html_error_quit("对不起，您的Blog申请已通过");
		exit();	
	}
	elseif($appstate == 1 )
	{
		pc_db_close($link);
		html_init("gb2312");
		html_error_quit("对不起，您的Blog申请还在处理中，请耐心等候");
		exit();	
	}
	elseif($appstate == 3 )
	{
		pc_db_close($link);
		html_init("gb2312");
		html_error_quit("对不起，您不能进行Blog申请，请联系管理员");
		exit();	
	}
	
	
	$apptitle = "[申请] ".$currentuser["userid"]." 申请建立水木BLOG";
	$appbody  = "(1) BLOG名称：".$_POST["appname"]."\n\n\n".
		    "(2) 申请人 ID 及简要自我介绍\n".
		    "    ID：".$currentuser["userid"]."\n".
		 //   "    注册时间：".date("Y年m月d日",$currentuser["firstlogin"])."\n".
		    "        \n".$_POST["appself"]."\n\n\n".
		    "(3) 经营方向：(您对您个人Blog的初步规划)\n        ".$_POST["appdirect"]."\n\n";
	
	$appself =  "ID：".$currentuser["userid"]."\n".
		   // "注册时间：".date("Y年m月d日",$currentuser["firstlogin"])."\n".
		    "自我介绍：\n".$_POST["appself"];
	$query = "INSERT INTO `newapply` ( `naid` , `username` , `appname` , `appself` , `appdirect` , `hostname` , `apptime` , `manager` , `management` ) ".
	 	 "VALUES ('', '".$currentuser["userid"]."', '".addslashes($_POST["appname"])."', '".addslashes($appself)."', '".addslashes($_POST["appdirect"])."', '".addslashes($_SERVER["REMOTE_ADDR"])."', NOW( ) , NULL , '1');";
	if(!mysql_query($query,$link))
	{
		pc_db_close($link);
		html_init("gb2312");
		html_error_quit("十分抱歉，由于系统原因，您的申请尚未登记入库。请重新填写一次");
		exit();	
	}
	
	pc_db_close($link);
	
	$ret = bbs_postarticle($pcconfig["APPBOARD"], preg_replace("/\\\(['|\"|\\\])/","$1",$apptitle), preg_replace("/\\\(['|\"|\\\])/","$1",$appbody), 0 , 0 , 0 , 0);
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
            case -21:
                html_error_quit("您的积分不符合当前讨论区的设定, 暂时无法在当前讨论区发表文章...");
                break;
		}
	pc_html_init("gb2312","Blog申请");
?>
<table cellpadding=3 cellspacing=1 align=center class=TableBorder1>
<tr align=center><th width="100%">申请提交成功！</td>
</tr><tr><td width="100%" class=TableBody1>
您的BLOG申请已经提交成功，管理员会在两天内处理您的申请。<br/><br/>
本页面将在3秒后自动切换自Blog论坛<meta HTTP-EQUIV=REFRESH CONTENT='3; URL=/bbsdoc.php?board=<?php echo $pcconfig["BOARD"]; ?>' >，<b>您可以选择以下操作：</b><br><ul>
<li><a href="/mainpage.php">返回首页</a></li>
<li><a href="/pc/index.html">返回Blog首页</a></li>
<li><a href="/bbsdoc.php?board=<?php echo $pcconfig["BOARD"]; ?>">返回Blog论坛</a></li>
<li><a href="/bbsdoc.php?board=<?php echo $pcconfig["APPBOARD"]; ?>">返回Blog申请版面</a></li>
</ul></td></tr></table>
<p align="center">
<?php
    /**
     *    水木的web代码bbslib和cgi部分是修改于NJUWWWBBS-0.9，此部分
     * 代码遵循原有的nju www bbs的版权声明（GPL）。php部分的代码（
     * phplib以及php页面）不再遵循GPL，正在考虑使用其他开放源码的版
     * 权声明（BSD或者MPL之类）。
     *
     *   希望使用水木代码的Web站点加上powered by kbs的图标.该图标
     * 位于html/images/poweredby.gif目录,链接指向http://dev.kcn.cn
     * 使用水木代码的站点可以通过dev.kcn.cn获得代码的最新信息.
     *
     */
    powered_by_smth();
?>
</p>
<?php	
	html_normal_quit();
}
	
?>
