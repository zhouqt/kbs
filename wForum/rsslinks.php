<?php
require("inc/funcs.php");
require("inc/user.inc.php");
require("inc/board.inc.php");

global $boardArr;
global $boardID;
global $boardName;

if (!RSS_SUPPORT) exit;

preprocess();

setStat("版面 RSS");

show_nav($boardName);

showUserMailBoxOrBR();
board_head_var($boardArr['DESC'],$boardName,$boardArr['SECNUM']);
main($boardName);
show_footer();

function preprocess(){
	global $boardID;
	global $boardName;
	global $currentuser;
	global $boardArr;
	if (!isset($_GET['board'])) {
		foundErr("未指定版面。");
	}
	$boardName=$_GET['board'];
	$brdArr=array();
	$boardID= bbs_getboard($boardName, $brdArr);
	if ($boardID==0) {
		foundErr("指定的版面不存在");
	}
	$boardArr=$brdArr;
	$boardName=$brdArr['NAME'];
	if ($boardArr['FLAG'] & BBS_BOARD_GROUP ) {
		foundErr("指定的版面不存在");
	}
	$usernum = $currentuser["index"];
	if (bbs_checkreadperm($usernum, $boardID) == 0) {
		foundErr("您无权阅读本版");
	}
	return true;
}

function main($boardName) {
	global $SiteURL;
?>
<table cellPadding="1" cellSpacing="1" align="center" class="TableBorder1" style="table-layout:fixed;word-break:break-all;">
<tr><th height="25" width="100%"><?php echo $boardName; ?> 版 RSS Feeds</th></tr>
<tr><td width="100%" class="TableBody1" style="padding: 10px;">
<?php
	$urls = array(
			array("版面最后 20 篇主题",
				"不包含内容，链接指向主题阅读，适用 Firefox live bookmark 等", "rss.php?board=$boardName",
				"不包括内容，链接指向简易阅读，适用 Thunderbird 在线阅读等", "rss.php?board=$boardName&amp;lw=1",
				"包括内容，链接指向主题阅读，适用 Sage，FeedDemon 等", "rss.php?board=$boardName&amp;ic=1"),
			array("文摘区最后 20 篇",
				"不包含内容，链接指向文摘阅读，适用 Firefox live bookmark 等", "rss.php?board=$boardName&amp;ftype=1",
				"不包括内容，链接指向简易阅读，适用 Thunderbird 在线阅读等", "rss.php?board=$boardName&amp;lw=1&amp;ftype=1",
				"包括内容，链接指向文摘阅读，适用 Sage，FeedDemon 等", "rss.php?board=$boardName&amp;ic=1&amp;ftype=1"),
			array("全站十大",
				"不包括内容，链接指向简易阅读，适用 Thunderbird 在线阅读等", "rsstopten.php")
				);
	$str = "<ul>";
	foreach ($urls as $url) {
		$str .= "<li>".$url[0]."<ul>";
		for ($i = 1; $i < count($url); $i+=2) {
			$str .= "<li><p>".$url[$i].":<br/> <a href=\"$SiteURL".$url[$i+1]."\" target=\"_blank\">$SiteURL".$url[$i+1]."</a></p></li>";
		}
		$str .= "</ul></li>";
	}
	$str .= "</ul>";
	echo $str;
?>
</td></tr></table>
<?php
}
?>