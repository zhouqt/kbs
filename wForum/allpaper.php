<?php

require("inc/funcs.php");
require("inc/user.inc.php");
require("inc/board.inc.php");
require("inc/conn.php");

global $boardArr;
global $boardID;
global $boardName;
global $page;

setStat("小字报列表");

preprocess();

if (!isset($_POST['action'])) {
	show_nav($boardName);
} else {
	show_nav(false);
}

showUserMailBoxOrBR();
board_head_var($boardArr['DESC'],$boardName,$boardArr['SECNUM']);
if ($_POST['action']=="delpaper") {
	batch();
	html_success_quit('返回小字报列表', 'allpaper.php?action=batch&board='.$boardName);
} else {
	boardeven($boardID,$boardName);
} 

show_footer();

function preprocess(){
	global $boardID;
	global $boardName;
	global $currentuser;
	global $boardArr;
	global $usernum;
	if (!isset($_REQUEST['board'])) {
		foundErr("未指定版面。");
	}
	$boardName=$_REQUEST['board'];
	$brdArr=array();
	$boardID= bbs_getboard($boardName, $brdArr);
	$boardArr=$brdArr;
	$boardName=$brdArr['NAME'];
	if ($boardID==0) {
		foundErr("指定的版面不存在");
	}
	$usernum = $currentuser["index"];
	if (bbs_checkreadperm($usernum, $boardID) == 0) {
		foundErr("您无权阅读本版");
	}
	return true;
}

function boardeven($boardID,$boardName){
	global $usernum;
	global $conn;
	global $currentuser;
	$isbm = bbs_is_bm($boardID,$usernum);
	$username = $currentuser["userid"];
?>
	<form action=allpaper.php method=post name=paper id="paper">
	<input type="hidden" name="action" value="delpaper" />
	<input type=hidden name=board value="<?php echo $boardName; ?>" />
	<table class=TableBorder1 cellspacing="1" cellpadding="3" align="center">
	<tr align=center> 
	<th width="15%" height=25>用户</th>
	<th width="35%">标题</th>
	<th width="20%">发布时间</th>
	<th width="15%">人气</th>
	<th width="15%">操作</th>
	</tr> 
<?php
	$sql="select * from smallpaper_tb where boardID=".$boardID." order by Addtime desc";

	if ($conn !== false) {
		$sth=$conn->query($sql);
		$totalrec=$sth->numRows();
	} else {
		$totalrec = 0;
	}
	if ($totalrec==0) {
		echo  "<tr> <td class=TableBody1 align=center colspan=5 height=25>本版还没有小字报</td></tr>";
	} else {
		while($rs=$sth->fetchRow(DB_FETCHMODE_ASSOC)) {
			echo '<tr>';
			echo '<td class=TableBody1 align=center height=24>';
			echo '<a href=dispuser.php?id='.$rs['Owner'].' target=_blank>'.$rs['Owner'].'</a>';
			echo '</td>';
			echo '<td class=TableBody1>';
			echo "<a href=javascript:openScript('viewpaper.php?id=".$rs["ID"]."&boardname=".$boardName."',500,400)>".htmlspecialchars($rs['Title'],ENT_QUOTES).'</a>';
			echo '</td>';
			echo '<td align=center class=TableBody1>'.$rs['Addtime'].'</td>';
			echo '<td align=center class=TableBody1>'.$rs["Hits"].'</td>';
			echo '<td align=center class=TableBody1>';
			if ($isbm || ($username == $rs["Owner"])) echo "<input type=checkbox name=num value=".$rs["ID"].">";
			echo "</td></tr>";
		}
?>
<input type="hidden" id="nums" name="nums">
<script >
function doAction(desc) {
	var nums,s,first,i;
	if(confirm(desc))	{
		objForm=getRawObject("paper");
		objNums=getRawObject("nums");
		objNums.value="";
		first=true;
		nums=getObjectCollection("num");
		for (i=0;i<nums.length;i++){
			s=nums[i];
			if (s.checked) {
				if (first) {
					first=false;
				} else {
					objNums.value+=',';
				}
				objNums.value+=s.value;
			}
		}
		return objForm.submit()
	}
	return false;
}
</script>
<tr><td class=TableBody2 colspan=5 align=right>请选择要删除的小字报
<input type=checkbox name=chkall id=chkall value=on onclick="CheckAll(this.form)"><label style="cursor:hand;" for="chkall">全选</label>
<input type=submit name=Submit value=删除  onclick="doAction('您确定删除这些小字报吗?')">
</td></tr>
<?php
		$rs=null;
	} 
	print "</table></form>";
	return true;
} 

function batch()
{
	global $loginok;
	global $boardID;
	global $usernum;
	global $currentuser;
	global $conn;
	if ($loginok!=1) {
		foundErr("您无权删除小字报。");
	}
	if ($_POST["nums"]=="") {
		foundErr("请指定相关小字报。");
	}
	if ($conn === false) {
		foundErr("数据库故障。");
	}
	$query = "delete from smallpaper_tb where boardID=".$boardID." and ID in (".$_POST["nums"].")";
	if (!bbs_is_bm($boardID,$usernum)) {
		$query .= " and Owner = '".$currentuser["userid"]."'";
	}
	$conn->query($query);
  	setSucMsg("您指定的小字报已经删除。"); // 实际上是 有权删除的 小字报 - atppp
  	return true;
} 
?>
