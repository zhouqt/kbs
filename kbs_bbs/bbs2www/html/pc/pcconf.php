<?php
/*
** personal corp. configure start
** LIST: user number in pc.php
** HOME: bbs home directory
** ETEMS: etems in xml file
** SITE: site address,used in xml file
** BOARD: whose manager can manage anyone's personal corp.
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
			"others" => "其它类别"
			);

/* personal corp. configure end */
?>