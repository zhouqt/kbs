<?php

/**
 * Checking whether a board is set with some specific flags or not.
 * 
 * @param $board the board object to be checked
 * @param $flag the flags to check
 * @return TRUE  the board is set with the flags
 *         FALSE the board is not set with the flags
 * @author flyriver
 */
function bbs_check_board_flag($board,$flag)
{
	if ($board["FLAG"] & $flag)
		return TRUE;
	else
		return FALSE;
}

/**
 * Checking whether a board is an anonymous board or not.
 * 
 * @param $board the board object to be checked
 * @return TRUE  the board is an anonymous board
 *         FALSE the board is not an anonymous board
 * @author flyriver
 */
function bbs_is_anony_board($board)
{
	global $BOARD_FLAGS;
	return bbs_check_board_flag($board, $BOARD_FLAGS["ANONY"]);
}

/**
 * Checking whether a board is an outgo board or not.
 * 
 * @param $board the board object to be checked
 * @return TRUE  the board is an outgo board
 *         FALSE the board is not an outgo board
 * @author flyriver
 */
function bbs_is_outgo_board($board)
{
	global $BOARD_FLAGS;
	return bbs_check_board_flag($board, $BOARD_FLAGS["OUTGO"]);
}

/**
 * Checking whether a board is a junk board or not.
 * 
 * @param $board the board object to be checked
 * @return TRUE  the board is a junk board
 *         FALSE the board is not a junk board
 * @author flyriver
 */
function bbs_is_junk_board($board)
{
	global $BOARD_FLAGS;
	return bbs_check_board_flag($board, $BOARD_FLAGS["JUNK"]);
}

/**
 * Checking whether a board is an attachment board or not.
 * 
 * @param $board the board object to be checked
 * @return TRUE  the board is an attachment board
 *         FALSE the board is not an attachment board
 * @author flyriver
 */
function bbs_is_attach_board($board)
{
	global $BOARD_FLAGS;
	return bbs_check_board_flag($board, $BOARD_FLAGS["ATTACH"]);
}

/**
 * Checking whether a board is a readonly board or not.
 * 
 * @param $board the board object to be checked
 * @return TRUE  the board is a readnoly board
 *         FALSE the board is not a readonly board
 * @author flyriver
 */
function bbs_is_readonly_board($board)
{
	global $BOARD_FLAGS;
	return bbs_check_board_flag($board, $BOARD_FLAGS["READONLY"]);
}


function showBoardStaticsTop($boardArr, $is_ann=false){
	global $conn;
	global $loginok;
?>
<TABLE cellpadding=3 cellspacing=1 class=TableBorder1 align=center>
<TR><Th height=25 width=100% align=left id=TableTitleLink style="font-weight:normal">
本版当前共有<b><?php echo $boardArr['CURRENTUSERS'];?></b>人在线。今日帖子<?php echo bbs_get_today_article_num($boardArr['NAME'] ); ?>。
[<a href="favboard.php?bname=<?php echo $boardArr["NAME"]; ?>" title="收藏本版面到收藏夹顶层目录">收藏本版</a>]
<?php
	if ($loginok && SHOW_POST_UNREAD) {
?>
&nbsp;[<a href="doclear.php?boardName=<?php echo $boardArr["NAME"]; ?>" title="将本版所有文章标记成已读">清除未读</a>]
<?php
	}
?>
</Th></TR></TABLE>
<BR>
<table cellpadding=2 cellspacing=0 border=0 width=97% align=center valign=middle><tr><td align=center width=2> </td>
<td align=left style="height:27" valign="center"><table cellpadding=0 cellspacing=0 border=0 ><tr>
<td width="110"><a href=postarticle.php?board=<?php echo $boardArr['NAME']; ?>><div class="buttonClass1" border=0 alt=发新帖></div></a></td>
<!--<td width="110"><a href=# onclick="alert('本功能尚在开发中！')"><div class="buttonClass2" border=0 alt=发起新投票></div></a></td>-->
<?php
	if ($conn !== false) {
?>
<td width="110"><a href=smallpaper.php?board=<?php echo $boardArr['NAME']; ?>><div class="buttonClass3" border=0 alt=发布小字报></div></a></td>
<?php
	}
?>
</tr></table></td>
<td align=right>
<?php
	if ($is_ann) {
?>
	<a href="board.php?name=<?php echo $boardArr['NAME']; ?>" title=查看本版文章><font color=blue><B>讨论区</B></font></a> 
<?php
	} else {
		$ann_path = bbs_getannpath($boardArr['NAME']);
		if ($ann_path != FALSE) {
	    	if (!strncmp($ann_path,"0Announce/",10))
			$ann_path=substr($ann_path,9);
?>
	<a href="elite.php?path=<?php echo urlencode($ann_path); ?>" title=查看本版精华区><font color=#FF0000><B>精华区</B></font></a> 
<?php
		}
	}
?>	
<!--
    | <a href=# onclick="alert('本功能尚在开发中！')" title=查看本版在线详细情况>在线</a>
	| <a href=# onclick="alert('本功能尚在开发中！')" title=查看本版事件>事件</a>
	| <a href=# onclick="alert('本功能尚在开发中！')" title=查看本版用户组权限>权限</a>
    | <a href=# onclick="alert('本功能尚在开发中！')">管理</a>
-->
&nbsp;&nbsp;<img src=pic/team2.gif align=absmiddle>
<?php 
	$bms=split(' ',$boardArr['BM']);
	foreach($bms as $bm) {
?>
<a href="dispuser.php?id=<?php echo $bm; ?>" target=_blank title=点击查看该版主资料><?php echo $bm; ?></a>
<?php
	}
?>
</td></tr></table>
<?php
}


function showBroadcast($boardID,$boardName){
	global $conn;
	if ($conn === false) return;
?>
<tr><td class=TableBody1 colspan=5 height=20>
	<table width=100% ><tr><td valign=middle height=20 width=50><a href=allpaper.php?board=<?php echo $boardName; ?> title=点击查看本论坛所有小字报><b>广播</b></a>：</td><td width=*> <marquee scrolldelay=150 scrollamount=4 onmouseout="if (document.all!=null){this.start()}" onmouseover="if (document.all!=null){this.stop()}">
<?php
	$sth = $conn->query("SELECT ID,Owner,Title FROM smallpaper_tb where Addtime>=subdate(Now(),interval 1 day) and boardID=" . $boardID . " ORDER BY Addtime desc limit 5");
	while($rs = $sth->fetchRow(DB_FETCHMODE_ASSOC)) {
		print "<font color=#ff0000>".$rs['Owner']."</font>说：<a href=javascript:openScript('viewpaper.php?id=".$rs['ID']."&boardname=".$boardName."',500,400)>".htmlspecialchars($rs['Title'],ENT_QUOTES)."</a>";
  } 
  unset($rs);
  $sth->free();
?>
	</marquee>
</td></tr></table>
</td></tr>
<?php
}

function board_head_var($boardDesc,$boardName,$secNum)
{
  GLOBAL $SiteName;
  GLOBAL $stats;
  global $section_names;
  if ($URL=='') {
	  $URL=$_SERVER['PHP_SELF'];
  }
?>
<table cellspacing=1 cellpadding=3 align=center class=TableBorder2>
<tr><td>
<img src="pic/forum_nav.gif"> <a href="index.php"><?php   echo $SiteName; ?></a> → 
<a href="section.php?sec=<?php echo $secNum; ?>"><?php echo $section_names[get_secname_index(intval($secNum))][0] ; ?></a> → 
<a href="board.php?name=<?php echo $boardName; ?>"><?php echo $boardDesc; ?></a> → 
<?php echo $stats; ?> 
</td></tr>
</table>
<br>
<?php 
} 
function boardJump(){
	global $section_names;
	global $sectionCount;
	global $section_nums;
	global $yank;
?>
<div align=right><select onchange="if(this.options[this.selectedIndex].value!=''){location=this.options[this.selectedIndex].value;}">
<option selected>跳转论坛至...</option>
<?php
	for ($i=0;$i<$sectionCount;$i++){
		echo "<option value=\"section.php?sec=".$i."\">╋".$section_names[$i][0]."</option>";
		$boards = bbs_getboards($section_nums[$i], 0, $yank | 2);
		if ($boards != FALSE) {
			$brd_desc = $boards["DESC"]; // 中文描述
			$brd_name = $boards["NAME"];
			$rows = sizeof($brd_desc);
			for ($t = 0; $t < $rows; $t++)	{
				echo "<option value=\"board.php?name=".$brd_name[$t]."\">&nbsp;&nbsp;├".$brd_desc[$t]."</option>";
			}
		}
	}
?>
</select></div>
<?php
}
function boardSearchAndJump($boardName, $boardID){

?>
<table border=0 cellpadding=0 cellspacing=3 width=97% align=center>
<tr>
<FORM METHOD=GET ACTION="queryresult.php">
<input type="hidden" name="boardName" value="<?php echo $boardName; ?>">
<td width=50% valign=middle nowrap height=40>快速搜索：<input type=text name=title>&nbsp;<input type=submit name=submit value=搜索></td>
</FORM>
<td valign=middle nowrap width=50% > 
<?php
	boardJump();
?>
</td></tr></table><BR>
<?php
}

function showBoardSampleIcons(){
	global $SiteName;
?>
<table cellspacing=1 cellpadding=3 width=100% class=TableBorder1 align=center><tr><th width=80% align=left>　-=&gt; <?php echo $SiteName; ?>图例</th><th noWrap width=20% align=right>所有时间均为 - <?php echo SERVERTIMEZONE; ?> &nbsp;</th></tr><tr><td colspan=2 class=TableBody1><table cellspacing=4 cellpadding=0 width=92% border=0 align=center><tr><td><img src=pic/blue/folder.gif> 开放的主题</td><td><img src=pic/blue/hotfolder.gif> 回复超过10贴</td><td><img src=pic/blue/lockfolder.gif> 锁定的主题</td><td><img src=pic/istop.gif> 固顶的主题 </td><td><img src=pic/ztop.gif> 总固顶的主题 </td><td> <img src=pic/isbest.gif> 精华帖子 </td><td> <img src=pic/closedb.gif> 投票帖子 </td></tr></table></td></tr></table>
<?php
}

function isMultiQueryAllowed() {
	global $currentuser;

	if (ALLOWMULTIQUERY) return true;
	if (ALLOW_SYSOP_MULTIQUERY) {
		if ($currentuser["userlevel"] & BBS_PERM_SYSOP) return true;
	}
	return false;
}
?>
