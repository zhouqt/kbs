<?php
/*
** personal corp. configure start
$pcconfig["LIST"] :Blog首页上每页显示的用户数;
$pcconfig["HOME"] :BBS主目录,默认为BBS_HOME;
$pcconfig["BBSNAME"] :站点名称,默认为BBS_FULL_NAME;
$pcconfig["ETEMS"] :RSS输出的条目数;
$pcconfig["NEWS"] :统计全站最新文章/评论时显示的条目数;
$pcconfig["THEMLIST"] :按主题分类时每个主题显示的Blog数;
$pcconfig["SITE"] :站点的域名,在blog显示,RSS输出中均要用到;
$pcconfig["BOARD"] :Blog对应的版面名称,该版版主将默认为Blog管理员;
$pcconfig["SEARCHFILTER"] :进行文章搜索时过滤掉的文字;
$pcconfig["SEARCHNUMBER"] :返回文章搜索结果时每页显示的条目数;
$pcconfig["SECTION"] :Blog分类方式;
$pcconfig["MINREGTIME"] :申请时要求的最短注册时间;
$pcconfig["ADMIN"] :管理员ID，设置后所有管理员都可以维护此Blog
$pcconfig["TMPSAVETIME"] :开启发文暂存功能时，保存的时间间隔， 单位为秒
pc_personal_domainname($userid)函数 :用户Blog的域名;
*/
$pcconfig["LIST"] = 100;
$pcconfig["HOME"] = BBS_HOME;
$pcconfig["BBSNAME"] = BBS_FULL_NAME;
$pcconfig["ETEMS"] = 20;
$pcconfig["NEWS"] = 100;
$pcconfig["THEMLIST"] = 50;
$pcconfig["SITE"] = "www.smth.edu.cn";
$pcconfig["BOARD"] = "SMTH_blog";
$pcconfig["APPBOARD"] = "BlogApply";
$pcconfig["SEARCHFILTER"] = " 的";
$pcconfig["SEARCHNUMBER"] = 10;
$pcconfig["ADMIN"] = "SYSOP";
$pcconfig["MINREGTIME"] = 6;
$pcconfig["TMPSAVETIME"] = 300;
$pcconfig["ALLCHARS"] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
$pcconfig["SECTION"] = array(
			"personal" => "个人空间" ,
			"literature" => "原创文学" ,
			"computer" => "电脑技术" ,
			"feeling" => "情感地带" ,
			"collage" => "青春校园" ,
			"learning" => "学术科学" ,
			"amusement" => "休闲娱乐" ,
			"travel" => "观光旅游" ,
			"literae" => "文化人文" ,
			"community" => "社会信息" ,
			"game" => "游戏乐园" ,
			"sports" => "体育竞技" ,
			"publish" => "媒体新闻" ,
			"business" => "商业经济",
			"life" => "生活资讯",
			"picture" => "图片美术",
			"collection" => "经典收藏",
			"others" => "其他类别"
			);

//首页显示的一些参数
define("_PCMAIN_TIME_LONG_" , 259200 ); //日志统计时长
define("_PCMAIN_NODES_NUM_" , 20 );     //显示的日志数目
define("_PCMAIN_USERS_NUM_" , 20 );     //显示的用户数目
define("_PCMAIN_REC_NODES_" , 40 );     //推荐日志数目
define("_PCMAIN_NEW_NODES_" , 40 );     //新日志数目
define("_PCMAIN_ANNS_NUM_"  , 6  );     //公告数目
define("_PCMAIN_RECOMMEND_" , 1   );  //博客推荐
define("_PCMAIN_RECOMMEND_BLOGGER_" , "SYSOP"); //固定推荐
define("_PCMAIN_RECOMMEND_QUEUE_" , "smthblogger.php");        //使用推荐队列

function pc_personal_domainname($userid)
{
	return "http://".$userid.".mysmth.net";	
}
/* personal corp. configure end */
?>