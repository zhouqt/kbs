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
pc_personal_domainname($userid)函数 :用户Blog的域名;
*/
$pcconfig["LIST"] = 20;
$pcconfig["HOME"] = BBS_HOME;
$pcconfig["BBSNAME"] = BBS_FULL_NAME;
$pcconfig["ETEMS"] = 20;
$pcconfig["NEWS"] = 20;
$pcconfig["THEMLIST"] = 50;
$pcconfig["SITE"] = "www.smth.edu.cn";
$pcconfig["BOARD"] = "SMTH_blog";
$pcconfig["SEARCHFILTER"] = " 的";
$pcconfig["SEARCHNUMBER"] = 10;
$pcconfig["ADMIN"] = "SYSOP";
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
			"publish" => "新闻出版" ,
			"others" => "其他类别"
			);

function pc_personal_domainname($userid)
{
	return "http://".$userid.".mysmth.net";	
}
/* personal corp. configure end */
?>