<?php
require("inc/funcs.php");
require("inc/board.inc.php");
require("inc/user.inc.php");
require("inc/conn.php");
require("inc/ubbcode.php");

global $boardArr;
global $boardID;
global $boardName;

preprocess();

setStat("发布小字报");

html_init();

if (isErrFounded()) {
	html_error_quit() ;
} else {
	showpaper($boardID,$boardName);
}

show_footer();

CloseDatabase();

function preprocess(){
	global $boardID;
	global $boardName;
	global $currentuser;
	global $boardArr;
	global $loginok;
	global $id;


	if (!isset($_GET['boardname'])) {
		foundErr("未指定版面。");
		return false;
	}
	$boardName=$_GET['boardname'];
	$brdArr=array();
	$boardID= bbs_getboard($boardName,$brdArr);
	$boardArr=$brdArr;
	$boardName=$brdArr['NAME'];
	if ($boardID==0) {
		foundErr("指定的版面不存在。");
		return false;
	}
	$usernum = $currentuser["index"];
	if (bbs_checkreadperm($usernum, $boardID) == 0) {
		foundErr("您无权阅读本版！");
		return false;
	}
	if (!isset($_GET['id'])){
		foundErr("错误的参数！");
		return false;
	}
	$id=intval($_GET['id']);
	return true;
}

function showpaper($boardID,$boardName){
	global $conn;
	global $id;
	$rs=$conn->getRow("select * from smallpaper_tb where ID=".$id,DB_FETCHMODE_ASSOC);
	if (count($rs)==0) {
		foundErr("没有找到相关信息。");
		return false;
	}	else  {
		$conn->query("update smallpaper_tb set Hits=Hits+1 where ID=".$id);
?>
<table cellpadding=3 cellspacing=1 align=center class=TableBorder1>
<TBODY> 
<TR> 
<Th height=24><?php echo htmlspecialchars($rs["Title"],ENT_QUOTES); ?></Th>
</TR>
<TR> 
<TD class=TableBody1>
<p align=center><a href=dispuser.php?id=<?php echo $rs["Owner"]; ?> target=_blank><?php echo $rs["Owner"]; ?></a> 发布于 <?php  echo $rs["Addtime"]; ?></p>
    <blockquote>   
      <br>   
<?php     echo dvbcode($rs["Content"],1); ?>  
      <br>
<div align=right>浏览次数：<?php  echo $rs["Hits"]; ?></div>
    </blockquote>
</TD>
</TR>
<TR align=middle> 
<TD height=24 class=TableBody2><a href=# onclick="window.close();">『 关闭窗口 』</a></TD>
</TR>
</TBODY>
</TABLE>
<?php 
  } 
  
  $rs=null;

} 

?>

