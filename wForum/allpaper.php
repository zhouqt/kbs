<?php

require("inc/funcs.php");
require("inc/user.inc.php");
require("inc/board.inc.php");
require("inc/conn.php");

global $boardArr;
global $boardID;
global $boardName;
global $page;

preprocess();

setStat("小字报列表");

show_nav($boardName);

if (isErrFounded()) {
	html_error_quit() ;
} else {
	?>
	<br>
	<TABLE cellSpacing=0 cellPadding=0 width=97% border=0 align=center>
	<?php

	if ($loginok==1) {
		showUserMailbox();
?>
</table>
<?php
	}
	board_head_var($boardArr['DESC'],$boardName,$boardArr['SECNUM']);
	if ($_POST['action']=="delpaper") {
		batch();
	} else {
		boardeven();
	} 
}

//showBoardSampleIcons();
show_footer();

CloseDatabase();

function boardeven($boardID,$boardName){
	global $usernum;
	$currentPage=intval($_REQUEST['page']);
	if ($currentpage==0 ) {
		$currentpage=1;
	}
	if (bbs_is_bm($boardID,$usernum)) {
?>
	<div align=center>请点击人气切换到管理状态</div>
	<?php   } ?>
	<form action=allpaper.php method=post name=paper>
	<input type="hidden" name="action" value="delpaper" />
	<input type=hidden name=board value="<?php   echo $boardName; ?>" />
	<table class=tableborder1 cellspacing="1" cellpadding="3" align="center">
	<tr align=center> 
	<th width="15%" height=25>用户</th>
	<th width="50%">标题</th>
	<th width="20%">发布时间</th>
	<th width="15%" id=tabletitlelink><?php   if (bbs_is_bm($boardID,$usernum))
	{
	?><a href="allpaper.php?action=batch&board=<?php     echo $boardName; ?>"><?php   } ?>人气</a></th>
	</tr> 
	<?php 

	$sql="select * from smallpaper_tb where boardID=".$boardID." order by Addtime desc";

	$sth=$conn->query($sql);
	$totalrec=$sth->numRows();
	if ($totalrec==0) {
		echo  "<tr> <td class=tablebody1 colspan=4 height=25>本版还没有小字报</td></tr>";
	}else{
		$page_count=0;
		while($rs=$sth->fetchRow(DB_FETCHMODE_ASSOC)) {
			echo '<tr>';
			echo '<td class=tablebody1 align=center height=24>';
			echo '<a href=dispuser.php?name='.$rs['Owner'].' target=_blank>'.$rs['Owner'].'</a>';
			echo '</td>';
			echo '<td class=tablebody1>';
			echo "<a href=javascript:openScript('viewpaper.php?id=".$rs["ID"]."&boardname=".$boardName."',500,400)>".htmlentities($rs['Title']).'</a>';
			echo '</td>';
			echo '<td class=tablebody1>'.$rs['Addtime'].'</td>';
			echo '<td align=center class=tablebody1>';
			if ($_REQUEST['action']=="batch")	{
				echo "<input type=checkbox name=sid value=".$rs["ID"].">";
			}else{
				echo $rs["Hits"];
			} 

			echo "</td></tr>";
			$page_count++;
	} 
	if ($_REQUEST['action']=="batch")
	{
	  echo "<tr><td class=tablebody2 colspan=4 align=right>请选择要删除的小字报，<input type=checkbox name=chkall value=on onclick=\"CheckAll(this.form)\">全选 <input type=submit name=Submit value=执行  onclick=\"{if(confirm('您确定执行的操作吗?')){this.document.paper.submit();return true;}return false;}\"></td></tr>";
	} 

	echo "</table>";
	$Pcount=;
	print "<table border=0 cellpadding=0 cellspacing=3 width=\"97%\" align=center>".
	"<tr><td valign=middle nowrap>".
	"页次：<b>".$currentpage."</b>/<b>".$Pcount."</b>页".
	"每页<b>".$Forum_Setting[11]."</b> 共有<b>".$totalrec."</b>条</td>".
	"<td valign=middle nowrap><div align=right><p>分页： ";

	if ($currentpage>4)
	{

	  print "<a href=\"?page=1&boardid=".$Boardid."\">[1]</a> ...";
	} 

	if ($Pcount>$currentpage+3)
	{

	  $endpage=$currentpage+3;
	}
	  else
	{

	  $endpage=$Pcount;
	} 

	for ($i=$currentpage-3; $i<=$endpage; $i=$i+1)
	{

	  if (!$i<1)
	  {

		if ($i==intval($currentpage))
		{

		  print " <font color=".$Forum_body[8].">[".$i."]</font>";
		}
		  else
		{

		  print " <a href=\"?page=".$i."&boardid=".$Boardid."\">[".$i."]</a>";
		} 

	  } 


	} 

	if ($currentpage+3<$Pcount)
	{

	  print "... <a href=\"?page=".$Pcount."&boardid=".$Boardid."\">[".$Pcount."]</a>";
	} 

	print "</font></td></tr>";

	$rs=null;

	} 


	print "</table></form>";

	return $function_ret;
} 

function batch()
{
  extract($GLOBALS);


  $adminpaper=false;
  if (!$Founduser)
  {

    $Founderr=true;
    $Errmsg=$Errmsg+"<br>"+"<li>请登陆后进行操作。";
  } 

  if (($master || $superboardmaster || $boardmaster) && intval($GroupSetting[27])==1)
  {

    $adminpaper=true;
  }
    else
  {

    $adminpaper=false;
  } 

  if ($UserGroupID>3 && intval($GroupSetting[27])==1)
  {

    $adminpaper=true;
  } 

  if ($FoundUserPer && intval($GroupSetting[27])==1)
  {

    $adminpaper=true;
  }
    else
  if ($FoundUserPer && intval($GroupSetting[27])==0)
  {

    $adminpaper=false;
  } 

  if (!$adminpaper)
  {

    $Founderr=true;
    $Errmsg=$Errmsg+"<br>"+"<li>您没有权限执行本操作。";
  } 

  if ($_POST["sid"]=="")
  {

    $Founderr=true;
    $Errmsg=$Errmsg+"<br>"+"<li>请指定相关小字报。";
  }
    else
  {

    $sid=str_replace("'","",$_POST["sid"]);
  } 

  if ($Founderr)
  {
    return $function_ret;
  } 

$conn->query("delete from smallpaper_tb where boardID=".$boardID." and s_id in (".$sid.")");
  $sucmsg="<li>您选择的小字报已经删除。";
dvbbs_suc();
  return $function_ret;
} 
?>
