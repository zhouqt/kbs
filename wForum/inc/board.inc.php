<?php

function showBoardStaticsTop($boardArr){
?>
<TABLE cellpadding=3 cellspacing=1 class=tableborder1 align=center><TR><Th height=25 width=100% align=left id=tabletitlelink style="font-weight:normal">本版当前共有<b><?php echo $boardArr['CURRENTUSERS'];?></b>人在线 </Th></TR></td></tr></TABLE>
<BR>
<table cellpadding=0 cellspacing=0 border=0 width=97% align=center valign=middle><tr><td align=center width=2> </td><td align=left style="height:27" valign="center"><a href=announce.php?boardid=2><span class="buttonclass1" border=0 alt=发新帖></span></a>&nbsp;&nbsp;<a href=vote.php?boardid=2><span class="buttonclass2" border=0 alt=发起新投票></span>&nbsp;&nbsp;<a href=smallpaper.php?boardid=2><span class="buttonclass3" border=0 alt=发布小字报></span></a></td><td align=right><img src=pic/team2.gif align=absmiddle>
<?php 
	$bms=split(' ',$boardArr['BM']);
	foreach($bms as $bm) {
?>
<a href="dispuser.php?name=<?php echo $bm; ?>" target=_blank title=点击查看该版主资料><?php echo $bm; ?></a>
<?php
	}
?>
</td></tr></table>
<?php
}

function showBoardContents($boardID,$boardName,$page){
	global $dir_modes;
	$dir_mode = $dir_modes["ORIGIN"];
?>
<?php
	$total = bbs_countarticles($boardID, $dir_mode);
	if ($total<=0) {
?>
<tr><td>
	本版还没有文章
<td></tr>
</table>
<?php
	} else {
?>
<form action=admin_batch.asp method=post name=batch><TR align=middle><Th height=25 width=32 id=tabletitlelink><a href=list.asp?name=<?php echo $boardName; ?>&page=&action=batch>状态</a></th><Th width=* id=tabletitlelink>主 题  (点<img src=pic/plus.gif align=absmiddle>即可展开贴子列表)</Th><Th width=80 id=tabletitlelink>作 者</Th><Th width=64 id=tabletitlelink>回复</Th><Th width=195 id=tabletitlelink>最后更新 | 回复人</Th></TR>
<?php
		
		$totalPages=ceil($total/ARTICLESPERPAGE);
		if (($page>$totalPages)) {
			$page=$totalPages;
		} else if ($page<1) {
			$page=1;
		}
		$start=$total-$page* ARTICLESPERPAGE+1;
		$num=ARTICLESPERPAGE;
		if ($start<=0) {
			$num+=$start-1;
			$start=1;
		}


		$articles = bbs_getarticles($boardName, $start, $num, $dir_modes["ORIGIN"]);
		$articleNum=count($articles);
		for($i=$articleNum-1;$i>=0;$i--){
			unset($threads);
			$threads=bbs_get_threads_from_id($boardID, intval($articles[$i]['ID']), $dir_modes["NORMAL"], 50000);
			$threadNum=count($threads);
?>
<TR align=middle><TD class=tablebody2 width=32 height=27><img src="pic/blue/folder.gif" alt=开放主题></TD><TD align=left class=tablebody1 width=* ><img src="pic/nofollow.gif" id="followImg1"><a href="disparticle.php?boardName=<?php echo $boardName ;?>&ID=<?php echo $i+$start ;?>" title="<?php echo $articles[$i]['TITLE'] ;?><br>作者：<?php echo $articles[$i]['OWNER'] ;?><br>发表于<?php echo strftime("%Y-%m-%d %H:%M:%S", $articles[$i]['POSTTIME']); ?>"><?php echo $articles[$i]['TITLE'] ;?></a> 
<?php
	$threadPages=ceil(($threadNum+1)/THREADSPERPAGE);
	if ($threadPages>1) {
		echo "<b>[<img src=\"pic/multipage.gif\"> ";
		for ($t=1; ($t<7) && ($t<=$threadPages) ;$t++) {
			echo "<a href=\"disparticle.php?boardName=".$boardName."&ID=".($i+$start). "&page=".$t."\">".$t."</a> ";
		}
		if ($threadPages>7) {
			if ($threadPages>8) {
				echo "...";
			}
			echo "<a href=\"disparticle.php?boardName=".$boardName."&ID=".($i+$start). "&page=".$threadPages."\">".$threadPages."</a> ";
		}
		echo " ]</b>";
	}
?>
</TD><TD class=tablebody2 width=80><a href="dispuser.asp?id=<?php echo $articles[$i]['OWNER'] ;?>" target=_blank><?php echo $articles[$i]['OWNER'] ;?></a></TD><TD class=tablebody1 width=64><?php echo $threads==NULL?0:$threadNum; ?></TD><TD align=left class=tablebody2 width=195>&nbsp;<a href="dispbbs.asp?boardid=1&id=1&star=1#1">
<?php
			if ($threads==NULL) {
				echo strftime("%Y-%m-%d %H:%M", $articles[$i]['POSTTIME']);
			} else {
				echo strftime("%Y-%m-%d %H:%M", $threads[$threadNum-1]['POSTTIME']);
			}
?></a>&nbsp;<font color=#FF0000>|</font>&nbsp;<a href=dispuser.asp?id=4 target=_blank>
<?php 
			if ($threads==NULL) {
				echo $articles[$i]['OWNER'];
			} else {
				echo $threads[$threadNum-1]['OWNER'];
			}
?></a></TD></TR>
<tr style="display:none" id="follow<?php echo $i; ?>"><td colspan=5 id="<?php echo $articles[$i]['ID'];?>" style="padding:0px"><div style="width:240px;margin-left:18px;border:1px solid black;background-color:lightyellow;color:black;padding:2px" onclick="loadThreadFollow(<?php echo $i;?>,1)">正在读取关于本主题的跟贴，请稍侯……</div></td></tr>
<?php
		}
?>
</form></table><table border=0 cellpadding=0 cellspacing=3 width=97% align=center >
<form method=get action="board.php">
<input type="hidden" name="name" value="<?php echo $boardName ; ?>">
<tr><td valign=middle>页次：<b><?php echo $page; ?></b>/<b><?php echo $totalPages; ?></b>页 每页<b><?php echo ARTICLESPERPAGE; ?></b> 主题数<b><?php echo $total ?></b></td><td valign=middle ><div align=right >分页：
<?php
    $lastTenPages=(floor(($page-1)/ 10))*10;
	if ($page==1) {
		echo "<font face=webdings color=\"#FF0000\">9</font>   ";
	}   else {
		echo "<a href=\"board.php?name=".$boardName."&page=1\" title=首页><font face=webdings>9</font></a>   ";
	} 

	if ($lastTenPages>0)  {
		echo "<a href='?name=". $boardName ."&page=" . $lastTenPages . "' title=上十页><font face=webdings>7</font></a>   ";  
	} 

	echo "<b>";
	for ($i=$lastTenPages+1; $i<=$lastTenPages+10; $i++) {
		if ($i==$page)	{
			echo "<font color=#ff0000>".$i."</font> ";
		} else {
			echo "<a href='board.php?name=".$boardName."&page=".$i."'>".$i."</a> ";
		} 
		if ($i==$totalPages) {
		  break;
		} 
	} 
	echo "</b>";
	if ($i<$totalPages) {
		echo "<a href='board.php?name=".$boardName."&page=".$i."' title=下十页><font face=webdings>8</font></a>   ";  
	} 
	if ($page==$totalPages) {
		echo "<font face=webdings color=#ff0000>:</font>   ";
	}  else  {
		echo "<a href='board.php?name=".$boardName."&page=".$totalPages."' title=尾页><font face=webdings>:</font></a>   ";
	} 
?>
转到:<input type=text name="page" size=3 maxlength=10  value=1><input type=submit value=Go ></div></td></tr>
</form></table>
<?php
	}
}

function showBroadcast($boardID,$boardName){
?>
<tr><td class=tablebody1 colspan=5 height=20>
	<table width=100% ><tr><td valign=middle height=20 width=50> <a href=AllPaper.asp?boardid=1 title=点击查看本论坛所有小字报><b>广播：</b></a> </td><td width=*> <marquee scrolldelay=150 scrollamount=4 onmouseout="if (document.all!=null){this.start()}" onmouseover="if (document.all!=null){this.stop()}"></marquee><td align=right width=240><a href=elist.asp?boardid=<?php echo boardID; ?> title=查看本版精华><font color=#FF0000><B>精华</B></font></a> | <a href=boardstat.asp?reaction=online&boardid=<?php echo boardID; ?> title=查看本版在线详细情况>在线</a> | <a href=bbseven.asp?boardid=1 title=查看本版事件>事件</a> | <a href=BoardPermission.asp?boardid=<?php echo boardID; ?> title=查看本版用户组权限>权限</a> | <a href=admin_boardset.asp?boardid=<?php echo boardID; ?>>管理</a></td></tr></table>
</td></tr>
<?php
}

function board_head_var($boardDesc,$boardName,$secNum)
{
  GLOBAL $SiteName;
  GLOBAL $SiteURL;
  GLOBAL $stats;
  global $section_names;
  if ($URL=='') {
	  $URL=$_SERVER['PHP_SELF'];
  }
?>
<table cellspacing=1 cellpadding=3 align=center class=tableBorder2>
<tr><td height=25 valign=middle>
<img src="pic/forum_nav.gif" align=absmiddle> <a href="<?php echo $SiteURL; ?>"><?php   echo $SiteName; ?></a> → 
<a href="section.php?sec=<?php echo $secNum; ?>"><?php echo $section_names[intval($secNum)][0] ; ?></a> → <a href="board.php?name=<?php echo $boardName; ?>"><?php echo $boardDesc; ?></a> → <?php echo $stats; ?> 
<a name=top></a>
</td></tr>
</table>
<br>
<?php 
} 
function boardJump(){
	global $section_names;
	global $sectionCount;
	global $section_nums;
?>
<div align=right><select onchange="if(this.options[this.selectedIndex].value!=''){location=this.options[this.selectedIndex].value;}">
<option selected>跳转论坛至...</option>
<?php
	for ($i=0;$i<$sectionCount;$i++){
		echo "<option value=\"section.php?sec=".$i."\">╋".$section_names[$i][0]."</option>";
		$boards = bbs_getboards($section_nums[$i], 0, 0);
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
<FORM METHOD=POST ACTION="boardsearch.php?name=<?php echo $boardName; ?>">
<td width=50% valign=middle nowrap height=40>快速搜索：<input type=text name=keyword>&nbsp;<input type=submit name=submit value=搜索></td>
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
<table cellspacing=1 cellpadding=3 width=100% class=tableborder1 align=center><tr><th width=80% align=left>　-=> <?php echo $SiteName; ?>图例</th><th noWrap width=20% align=right>所有时间均为 - 北京时间 &nbsp;</th></tr><tr><td colspan=2 class=tablebody1><table cellspacing=4 cellpadding=0 width=92% border=0 align=center><tr><td><img src=pic/blue/folder.gif> 开放的主题</td><td><img src=pic/blue/hotfolder.gif> 回复超过10贴</td><td><img src=pic/blue/lockfolder.gif> 锁定的主题</td><td><img src=pic/istop.gif> 固顶的主题 </td><td><img src=pic/ztop.gif> 总固顶的主题 </td><td> <img src=pic/isbest.gif> 精华帖子 </td><td> <img src=pic/closedb.gif> 投票帖子 </td></tr></table></td></tr></table>
<?php
}
?>