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
$pcconfig["USERFILES"] :支持用户个人空间,若支持需要定义 _USER_FILE_ROOT_
$pcconfig["USERFILESLIMIT"]:用户个人空间的默认大小,单位是b
$pcconfig["USERFILESNUMLIMIT"]:用户个人空间的默认容量
$pcconfig["USERFILEPERM"]:用户个人空间是否支持权限控制
$pcconfig["USERFILEREF"] :用户个人空间是否检查HTTP_REFERER以防止盗链,开启此功能时请编辑 $accept_hosts 的预定值
$pcconfig["ENCODINGTBP"] :对 trackback ping 的字符串进行编码处理,包括送出编码和接收编码.开启此功能请确定你的PHP支持 mbstring 和 iconv
$pcconfig["TYPES"]: blog类型
$pcconfig["BLOGONBLOG"]: blog on blog,hehe,如果没有，设成""即可。
pc_personal_domainname($userid)函数 :用户Blog的域名;
*/
$pcconfig["LIST"] = 100;
$pcconfig["HOME"] = BBS_HOME;
$pcconfig["BBSNAME"] = BBS_FULL_NAME;
$pcconfig["ETEMS"] = 20;
$pcconfig["NEWS"] = 100;
$pcconfig["THEMLIST"] = 50;
$pcconfig["SITE"] = "www.newsmth.net";
$pcconfig["BOARD"] = "blogassistant";
$pcconfig["APPBOARD"] = "Blog_Apply";
$pcconfig["SEARCHFILTER"] = " 的";
$pcconfig["SEARCHNUMBER"] = 10;
$pcconfig["ADMIN"] = "SYSOP";
$pcconfig["MINREGTIME"] = 1;
$pcconfig["TMPSAVETIME"] = 300;
$pcconfig["ALLCHARS"] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
$pcconfig["USERFILES"] = true;
$pcconfig["USERFILESLIMIT"] = 5*1024*1024;
$pcconfig["USERFILESNUMLIMIT"] = 1000;
$pcconfig["USERFILEPERM"]= false;
$pcconfig["USERFILEREF"] = true;
$pcconfig["ENCODINGTBP"] = true;
$pcconfig["BLOGONBLOG"] = "smth.blog";
$pcconfig["TYPES"] = array (
                    "normal" => "",
                    "group"  => "group",
                    "system" => "smth",
                    "board" => "board",
                    "columnist" => "columnist"
                                                    );
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

//define("_BLOG_ANONY_COMMENT_", 1);  // 是否允许匿名评论

//首页显示的一些参数
define("_PCMAIN_TIME_LONG_" , 259200 ); //日志统计时长
define("_PCMAIN_NODES_NUM_" , 20 );     //显示的日志数目
define("_PCMAIN_USERS_NUM_" , 20 );     //显示的用户数目
define("_PCMAIN_REC_NODES_" , 40 );     //推荐日志数目
define("_PCMAIN_NEW_NODES_" , 40 );     //新日志数目
define("_PCMAIN_SECTION_NODES_" , 5 );     //分类日志数
define("_PCMAIN_ANNS_NUM_"  , 6  );     //公告数目
define("_PCMAIN_RECOMMEND_" , 1   );  //博客推荐
//define("_PCMAIN_RECOMMEND_BLOGGER_" , "SYSOP"); //固定推荐
/*
** 注: smth.org的blog是使用推荐队列，其他站点若无推荐队列，可以设定固定推荐
*/
define("_PCMAIN_RECOMMEND_QUEUE_" , "smthblogger.php");        //使用推荐队列

function pc_personal_domainname($userid)
{
	return "http://".$userid.".mysmth.net";	
}

define('_USER_FILE_ROOT_' , BBS_HOME.'/blogs'); //个人空间根目录位置 需要手工建立

/*
* $accept_hosts: 当用户个人空间支持反盗链时，检查是否从信任主机上连接过来
*/
$accept_hosts = array(
                '127.0.0.1',
				'61.182.213.237',
				'10.0.4.237',
				'61.182.213.238',
				'10.0.4.238',
				'61.182.213.215',
				'61.182.213.217'
                );
                
/* Trackback Ping String Encoding Configure Start */
$support_encodings = 'ISO-8859-1,UTF-8';
$default_encoding  = 'ISO-8859-1';
$sending_encoding  = 'UTF-8';
/* Trackback Ping String Encoding Configure End */

/* personal corp. configure end */
?>
