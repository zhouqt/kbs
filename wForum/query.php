<?php
require("inc/funcs.php");
require("inc/user.inc.php");
require("inc/board.inc.php");


global $boardName;
global $boardArr;

setStat("版面搜索");

show_nav();

preprocess();

if ($loginok==1) {
?>
<table border="0" width="97%">
<?php
	showUserMailbox();
?>
</table>
<?php
} else {
	echo "<br><br>";
}

if ($boardName!='') 
	board_head_var($boardArr['DESC'],$boardName,$boardArr['SECNUM']);
else {
	head_var("论坛搜索",'',0);
}

if (isErrFounded()) {
		html_error_quit();
} else {
	showSearchMenu();
}

show_footer();

function preprocess(){
	global $boardID;
	global $boardName;
	global $currentuser;
	global $boardArr;
	if (!isset($_GET['boardName'])) {
		return true;
	}
	$boardName=$_GET['boardName'];
	$brdArr=array();
	$boardID= bbs_getboard($boardName,$brdArr);
	$boardArr=$brdArr;
	$boardName=$brdArr['NAME'];
	if ($boardID==0) {
		$boardName='';
		return true;
	}
	$usernum = $currentuser["index"];
	if (bbs_checkreadperm($usernum, $boardID) == 0) {
		$boardName='';
		return true;
	}
	return true;
}

function showSearchMenu(){
	global $section_names;
	global $sectionCount;
	global $section_nums;
?>
	<form action=queryresult.php method=post>
    <table cellpadding=5 cellspacing=1 align=center class=tableborder1>
    	<tr>
	<th valign=middle colspan=2 >论坛搜索</th></tr>
           <tr>
	<td class=tablebody2 valign=middle colspan=2 align=center><b>请选择要搜索的版面</b></td></tr>
	<tr>
	<td class=tablebody1 colspan=2 valign=middle align=center>
           <b>搜索论坛： &nbsp; 
<select name=boardName size=1>
<?php
	for ($i=0;$i<$sectionCount;$i++){
		$boards = bbs_getboards($section_nums[$i], 0, 0);
		if ($boards != FALSE) {
			$brd_desc = $boards["DESC"]; // 中文描述
			$brd_name = $boards["NAME"];
			$rows = sizeof($brd_desc);
			for ($t = 0; $t < $rows; $t++)	{
				echo "<option value=\"".$brd_name[$t]."\">".$brd_desc[$t]."</option>";
			}
		}
	}
?>
</select>
	</b></td>
	</tr><tr>
	<td class=tablebody2 valign=middle colspan=2 align=center><b>搜索选项</b></td></tr>
<tr><td class=tablebody1 valign=middle colspan=2 align=center>标题含有: <input type="text" maxlength="50" size="20" name="title"> AND <input type="text" maxlength="50" size="20" name="title2"></td></tr>
<tr><td class=tablebody1 valign=middle colspan=2 align=center>标题不含: <input type="text" maxlength="50" size="20" name="title3"></td></tr>
<tr><td class=tablebody1 valign=middle colspan=2 align=center>作者帐号: <input type="text" maxlength="12" size="12" name="userid"></td></tr>
<tr><td class=tablebody1 valign=middle colspan=2 align=center>时间范围: <input type="text" maxlength="4"  size="4"  name="dt" value="7"> 天以内</td></tr>
<tr><td class=tablebody1 valign=middle colspan=2 align=center>精华文章:<input type="checkbox" name="mg">  带附件文章:<input type="checkbox" name="ag">  不含跟贴:<input type="checkbox" name="og"></td></tr>
	<tr>
	<td class=tablebody2 valign=middle colspan=2 align=center>
	<input type=submit value=开始搜索>
	</td></form></tr></table>
<?php
}
?>
