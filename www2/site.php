<?php
define("ATTACHMAXSIZE",BBS_MAXATTACHMENTSIZE);        //附件总字节数的上限，单位 bytes
define("ATTACHMAXCOUNT",BBS_MAXATTACHMENTCOUNT);      //附件数目的上限
define("MAINPAGE_FILE","mainpage.php");              //首页导读的 URL
define("QUOTED_LINES", BBS_QUOTED_LINES);             //web 回文保留的引文行数
define("SITE_NEWSMTH", 1);
define("RUNNINGTIME", 1);                             //底部显示页面运行时间

$section_nums = array();
$section_names = array();
for($i=0;$i<BBS_SECNUM;$i++) {
	$section_nums[] = constant("BBS_SECCODE".$i);
	$section_names[] = array(constant("BBS_SECNAME".$i."_0"),constant("BBS_SECNAME".$i."_1"));
}

// 界面方案的名称
$style_names = array(
	"默认方案（蓝色经典）",
	"白的（猪猪猪测试用）"
);

?>
