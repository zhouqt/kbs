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

function showBoardStaticsTop($boardArr){
?>
<TABLE cellpadding=3 cellspacing=1 class=TableBorder1 align=center>
<TR><Th height=25 width=100% align=left id=TableTitleLink style="font-weight:normal">
本版当前共有<b><?php echo $boardArr['CURRENTUSERS'];?></b>人在线。今日帖子<?php echo bbs_get_today_article_num($boardArr['NAME'] ); ?>。
[<a href="favboard.php?bname=<?php echo $boardArr["NAME"]; ?>" title="收藏本版面到收藏夹顶层目录">收藏本版</a>]</Th></TR></td></tr></TABLE>
<BR>
<table cellpadding=0 cellspacing=0 border=0 width=97% align=center valign=middle><tr><td align=center width=2> </td>
<td align=left style="height:27" valign="center"><table cellpadding=0 cellspacing=0 border=0 ><tr>
<td width="110"><a href=postarticle.php?board=<?php echo $boardArr['NAME']; ?>><div class="buttonClass1" border=0 alt=发新帖></div></a></td>
<td width="110"><a href=# onclick="alert('本功能尚在开发中！')"><div class="buttonClass2" border=0 alt=发起新投票></div></a></td>
<td width="110"><a href=smallpaper.php?board=<?php echo $boardArr['NAME']; ?>><div class="buttonClass3" border=0 alt=发布小字报></div></a></td>
</tr></table></td>
<td align=right><img src=pic/team2.gif align=absmiddle>
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

function showBoardContents($boardID,$boardName,$page){
	global $dir_modes;
?>
<?php
	$total = bbs_getThreadNum($boardID);
	if ($total<=0) {
?>
<tr><td>
	本版还没有文章
<td></tr>
</table>
<?php
	} else {
?>
<form action=admin_batch.asp method=post name=batch><TR align=middle>
<Th height=25 width=32 id=TableTitleLink><a href=list.asp?name=<?php echo $boardName; ?>&page=&action=batch>状态</a></th>
<Th width=* id=TableTitleLink>主 题  (点<img src=pic/plus.gif align=absmiddle>即可展开贴子列表)</Th>
<Th width=80 id=TableTitleLink>作 者</Th>
<Th width=64 id=TableTitleLink>回复</Th>
<Th width=200 id=TableTitleLink>最后更新 | 回复人</Th></TR>
<?php
		
		$totalPages=ceil($total/ARTICLESPERPAGE);
		if (($page>$totalPages)) {
			$page=$totalPages;
		} else if ($page<1) {
			$page=1;
		}
	/*
		$start=$total-$page* ARTICLESPERPAGE+1;
		$num=ARTICLESPERPAGE;
		if ($start<=0) {
			$num+=$start-1;
			$start=1;
		}
    */
		$start=($page-1)* ARTICLESPERPAGE;
		$num=ARTICLESPERPAGE;

		$articles = bbs_getthreads($boardName, $start, $num, 1);
		$articleNum=count($articles);
?>
<script language="JavaScript">
<!--
	boardName = '<?php echo $boardName; ?>';
	THREADSPERPAGE = <?php echo THREADSPERPAGE; ?>;
	
	function Post(id, owner, posttime, flags) {
		this.ID = id;
		this.OWNER = owner;
		this.POSTTIME = posttime;
		this.FLAGS = flags;
	}
	
	function writepost(unused_id, html_title, threadNum, origin, lastreply, origin_exists) {
		/* note: when origin post does not exists, origin is actually the same as lastreply except
		 * groupID is different. See also www_generateOriginIndex() */
		//if (!origin_exists) return;
		document.write("<TR align=middle><TD class=TableBody2 width=32 height=27 align=\"center\">");
		if (article_is_zhiding(origin.FLAGS)) {
			document.write("<img src=\"pic/istop.gif\" alt=固顶的主题>");
		} else if( threadNum > 10 ) {
			document.write("<img src=\"pic/blue/hotfolder.gif\" alt=回复超过10贴>");
		} else if(article_is_noreply(origin.FLAGS)) {
			document.write("<img src=\"pic/blue/lockfolder.gif\" alt=锁定的主题>");
		} else if(article_is_digest(origin.FLAGS)) {
			document.write("<img src=\"pic/isbest.gif\" alt=精华帖>");
		} else {
			document.write("<img src=\"pic/blue/folder.gif\" alt=开放主题>");
		}
		document.write("</TD><TD align=left class=TableBody1 width=* >");
		if (threadNum==0) {
			document.write('<img src="pic/nofollow.gif" id="followImg' + unused_id + '">');
		} else {
			document.write('<img loaded="no" src="pic/plus.gif" id="followImg' + origin.ID + '" style="cursor:hand;" onclick="loadThreadFollow(\'' + origin.ID + "','" + boardName + "')\" title=展开贴子列表>");
		}
		if (origin_exists) {
			href_title = html_title + ' <br>作者：' + origin.OWNER + '<br>发表于' + origin.POSTTIME;
		} else {
			href_title = "原贴已删除";
		}
		document.write('<a href="disparticle.php?boardName=' + boardName + '&ID=' + origin.ID + '" title="' + href_title + '">' + html_title + ' </a>');
		threadPages = Math.ceil((threadNum+1)/THREADSPERPAGE);
		if (threadPages>1) {
			document.write("<b>[<img src=\"pic/multipage.gif\"> ");
			for (t=1; (t<7) && (t<=threadPages); t++) {
				document.write("<a href=\"disparticle.php?boardName=" + boardName + "&ID=" + origin.ID + "&start=" + ((t-1)*THREADSPERPAGE) + "\">" + t + "</a> ");
			}
			if (threadPages>7) {
				if (threadPages>8) {
					document.write("...");
				}
				document.write("<a href=\"disparticle.php?boardName=" + boardName + "&ID=" + origin.ID + "&start=" + ((threadPages-1)*THREADSPERPAGE) + "\">" + threadPages + "</a> ");
			}
			document.write(" ]</b>");
		}
		if (article_is_unread(lastreply.FLAGS)) {
			 //最后回复未读那这个 thread 就未读
			document.write("<img src=\"pic/topnew2.gif\" alt=\"未读\">");
		}
		document.write("</TD>");
		document.write('<TD class=TableBody2 width=80 align="center">');
		if (origin_exists) {
			document.write('<a href="dispuser.php?id=' + origin.OWNER + '" target=_blank>' + origin.OWNER + '</a>');
		} else {
			document.write('原贴已删除');
		}
		document.write('</TD>');
		document.write('<TD class=TableBody1 width=64 align="center">' + threadNum + '</TD>');
		document.write('<TD align=left class=TableBody2 width=200><nobr>&nbsp;<a href="disparticle.php?boardName=' + boardName + '&ID=' + origin.ID + '&start=' + threadNum + '">');
		document.write(lastreply.POSTTIME + '</a>&nbsp;<font color=#FF0000>|</font>&nbsp;<a href=dispuser.php?id=' + lastreply.OWNER + ' target=_blank>');
		document.write(lastreply.OWNER + '</nobr></a></TD></TR>');
		if (threadNum>0) {
			document.write('<tr style="display:none" id="follow' + origin.ID + '"><td colspan=5 id="followTd' + origin.ID + '" style="padding:0px"><div style="width:240px;margin-left:18px;border:1px solid black;background-color:lightyellow;color:black;padding:2px" onclick="loadThreadFollow(\'' + origin.ID + '\', \'' + boardName + '\')">正在读取关于本主题的跟贴，请稍侯……</div></td></tr>');
		}
	}
<?php
		for($i=0;$i<$articleNum;$i++){
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
</form></table><table border=0 cellpadding=0 cellspacing=3 width=97% align=center >
<form method=get action="board.php">
<input type="hidden" name="name" value="<?php echo $boardName ; ?>">
<tr><td valign=middle>页次：<b><?php echo $page; ?></b>/<b><?php echo $totalPages; ?></b>页 每页<b><?php echo ARTICLESPERPAGE; ?></b> 主题数<b><?php echo $total ?></b></td><td valign=middle ><div align=right >分页：
<?php
    $lastTenPages=(floor(($page-1)/ 10))*10;
	if ($page==1) {
		echo "<font face=webdings color=\"#FF0000\">9</font>   "; //ToDo: XHTML 不建议使用 webdings 字体。
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
	global $conn;
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
  /* ToDo: 注意：精华 link 暂时是搜索本版所有精华贴，待改。注意搜索好像没有分页。- atppp */
?>
	</marquee><td align=right width=240><a href="queryresult.php?boardName=<?php echo $boardName; ?>&title=&title2=&title3=&userid=&dt=0&mg=on" title=查看本版精华贴><font color=#FF0000><B>精华</B></font></a>
	| <a href=# onclick="alert('本功能尚在开发中！')" title=查看本版在线详细情况>在线</a>
	| <a href=# onclick="alert('本功能尚在开发中！')" title=查看本版事件>事件</a>
	| <a href=# onclick="alert('本功能尚在开发中！')" title=查看本版用户组权限>权限</a>
    | <a href=# onclick="alert('本功能尚在开发中！')">管理</a></td></tr></table>
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
<table cellspacing=1 cellpadding=3 align=center class=TableBorder2>
<tr><td>
<img src="pic/forum_nav.gif"> <a href="index.php"><?php   echo $SiteName; ?></a> → 
<a href="section.php?sec=<?php echo $secNum; ?>"><?php echo $section_names[intval($secNum)][0] ; ?></a> → <a href="board.php?name=<?php echo $boardName; ?>"><?php echo $boardDesc; ?></a> → <?php echo $stats; ?> 
</td></tr>
</table>
<br>
<?php 
} 
function boardJump(){ /* 这里占了很多运行时间和网络传输，用cache或者js优化是可能的，但是要小心权限问题。 - atppp */
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
		$boards = bbs_getboards($section_nums[$i], 0, $yank);
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
<table cellspacing=1 cellpadding=3 width=100% class=TableBorder1 align=center><tr><th width=80% align=left>　-=> <?php echo $SiteName; ?>图例</th><th noWrap width=20% align=right>所有时间均为 - <?php echo SERVERTIMEZONE; ?> &nbsp;</th></tr><tr><td colspan=2 class=TableBody1><table cellspacing=4 cellpadding=0 width=92% border=0 align=center><tr><td><img src=pic/blue/folder.gif> 开放的主题</td><td><img src=pic/blue/hotfolder.gif> 回复超过10贴</td><td><img src=pic/blue/lockfolder.gif> 锁定的主题</td><td><img src=pic/istop.gif> 固顶的主题 </td><td><img src=pic/ztop.gif> 总固顶的主题 </td><td> <img src=pic/isbest.gif> 精华帖子 </td><td> <img src=pic/closedb.gif> 投票帖子 </td></tr></table></td></tr></table>
<?php
}
?>
