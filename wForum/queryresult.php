<?php
require("inc/funcs.php");
require("inc/user.inc.php");
require("inc/board.inc.php");


global $boardName;
global $boardArr;
global $boardID;

setStat("搜索结果");

preprocess();

show_nav($boardName);

showUserMailBoxOrBR();

if ($boardName!='') 
	board_head_var($boardArr['DESC'],$boardName,$boardArr['SECNUM']);
else {
	head_var("论坛搜索",'',0);
}

if (isErrFounded()) {
		html_error_quit();
} else {
	doSearch($boardID,$boardName);
}

if (isErrFounded()) {
	html_error_quit();
}

show_footer();

function preprocess(){
	global $boardID;
	global $boardName;
	global $currentuser;
	global $boardArr;
	global $title,$title2,$title3,$author;
	if (!isset($_REQUEST['boardName'])) {
		foundErr("未指定版面。");
		return false;
	}
	$boardName=$_REQUEST['boardName'];
	$brdArr=array();
	$boardID= bbs_getboard($boardName,$brdArr);
	$boardArr=$brdArr;
	$boardName=$brdArr['NAME'];
	if ($boardID==0) {
		foundErr("指定的版面不存在");
		return false;
	}
	$usernum = $currentuser["index"];
	if (bbs_checkreadperm($usernum, $boardID) == 0) {
		foundErr("您无权阅读本版");
		return false;
	}
	$title=trim($_REQUEST['title']);
	$title2=trim($_REQUEST['title2']);
	$title3=trim($_REQUEST['title3']);
	$author=trim($_REQUEST['author']);
	
	return true;
}

function doSearch($boardID,$boardName){
	global $title,$title2,$title3,$author;
	$articles=bbs_searchtitle($boardName,$title,$title2,$title3,$author,intval($_REQUEST['dt']),isset($_REQUEST['mg']),isset($_REQUEST['ag']));
	$num=count($articles);
	if ($num==0 || $articles<=0) {
		foundErr("<font color=#ff0000>没有找到您要的结果</font>");
		return false;
	}
?>
<script src="inc/loadThread.js"></script>
<iframe width=0 height=0 src="" id="hiddenframe" name="hiddenframe"></iframe>

<table cellpadding=0 cellspacing=0 border=0 width="97%" align=center>
<tr><td>搜索主题共查询到<font color=#FF0000><?php echo$num; ?></font>个结果
</td></tr></table>
<TABLE cellPadding=3 cellSpacing=1 class=TableBorder1 align=center>
<TR valign=middle>
<Th height=25 width=32>状态</Th>
<Th width=*>主 题</Th>
<Th width=80>作 者</Th>
<Th width=64>回复</Th>
<Th width=200>最后更新 | 回复人</Th></TR>
</TR>
<script language="JavaScript">
<!--
<?php
	print_file_display_javascript($boardName);
	for ($i=1;$i<=$num;$i++) {
			$origin=$articles[$i]['origin'];
			$lastreply=$articles[$i]['lastreply'];
			$threadNum=$articles[$i]['articlenum']-1;
?>
	origin = new Post(<?php echo $origin['ID']; ?>, '<?php echo $origin['OWNER']; ?>', '<?php echo strftime("%Y-%m-%d %H:%M:%S", $origin['POSTTIME']); ?>', '<?php echo $origin['FLAGS'][0]; ?>');
	lastreply = new Post(<?php echo $lastreply['ID']; ?>, '<?php echo $lastreply['OWNER']; ?>', '<?php echo strftime("%Y-%m-%d %H:%M:%S", $lastreply['POSTTIME']); ?>', '<?php echo $lastreply['FLAGS'][0]; ?>');
	writepost(<?php echo $i+$start; ?>, '<?php echo addslashes(htmlspecialchars($origin['TITLE'],ENT_QUOTES)); ?> ', <?php echo $threadNum; ?>, origin, lastreply, <?php echo ($origin['GROUPID'] == $lastreply['GROUPID'])?"true":"false"; ?>);
<?php
	}
?>
//-->
</script>
</table>
<?php
}
?>
