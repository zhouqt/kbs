<?php

$setboard=1;

require("inc/funcs.php");
require("inc/user.inc.php");

setStat("分区版面列表");

preprocess();

show_nav();

showUserMailBoxOrBR();
head_var($section_names[$secNum][0],'section.php?sec='.$secNum, 0);
?>
<table cellSpacing=0 cellPadding=0 width=97% border=0 align=center>
 <?php
	showAnnounce(); 
?>
<tr>
<td align=center width=100% valign=middle colspan=2>
<hr>
</td></tr>
</table>
<?php
showSecs($secNum,0,true);
showUserInfo();
showSample();

show_footer();

/*--------------- function defines ------------------*/

function preprocess(){
	GLOBAL $sectionCount;
	global $secNum;

	$path='';
	$secNum=intval($_GET['sec']);
	if ( ($secNum<0)  || ($secNum>=$sectionCount)) {
		foundErr("您指定的分区不存在！");
	} else {
		if ($_GET['ShowBoards']=='N') {
			setcookie('ShowSecBoards'.$secNum, '' ,time()+604800);
			$_COOKIE['ShowSecBoards'.$secNum]='';
		} else {
			setcookie('ShowSecBoards'.$secNum, 'Y' ,time()+604800);
			$_COOKIE['ShowSecBoards'.$secNum]='Y';
		}
	}
}

?>
