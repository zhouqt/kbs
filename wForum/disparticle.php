<?php


$needlogin=1;

require("inc/funcs.php");
require("inc/user.inc.php");
require("inc/board.inc.php");

global $boardArr;
global $boardID;
global $boardName;
global $article;
global $articleID;
global $start;
global $listType;

preprocess();

setStat($article['TITLE']);

show_nav();

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
	require_once("inc/ubbcode.php");
	showArticleThreads($boardName,$boardID,$articleID,$article,$start,$listType);
}

//showBoardSampleIcons();
show_footer();

function preprocess(){
	global $boardID;
	global $boardName;
	global $currentuser;
	global $boardArr;
	global $article;
	global $articleID;
	global $dir_modes;
	global $listType;
	global $start;
	if (!isset($_GET['boardName'])) {
		foundErr("未指定版面。");
		return false;
	}
	$boardName=$_GET['boardName'];
	$brdArr=array();
	$boardID= bbs_getboard($boardName,$brdArr);
	$boardArr=$brdArr;
	if ($boardID==0) {
		foundErr("指定的版面不存在");
		return false;
	}
	$usernum = $currentuser["index"];
	if (bbs_checkreadperm($usernum, $boardID) == 0) {
		foundErr("您无权阅读本版");
		return false;
	}
	if (!isset($_GET['ID'])) {
		foundErr("您指定的文章不存在！");
		return false;
	} else {
		$articleID=intval($_GET['ID']);
	}
	$listType=0;
	if(isset($_GET['listType'])) {
		if ($_GET['listType']=='1')
			$listType=1;
	}
	if (!isset($_GET['start'])) {
		$start=0;
	} else {
		$start=intval($_GET['start']);
	}

	bbs_set_onboard($boardID,1);
	$articles = bbs_getarticles($boardName, $articleID, 1, $dir_modes["ORIGIN"]);
	@$article=$articles[0];
	if ($article==NULL) {
		foundErr("您指定的文章不存在！");
		return false;
	}
	return true;
}

function article_bar($boardName,$boardID,$articleID,$article,$threadID,$listType){
	global $dir_modes;
?>
<table cellpadding=0 cellspacing=0 border=0 width=97% align=center>
	<tr>
	<td align=left valign=middle style="height:27">&nbsp; 
	<a href="postarticle.php?board=<?php echo $boardName; ?>"><span class="buttonclass1" border=0 alt=发表一个新主题></span></a>&nbsp;&nbsp;<a href="vote.php?name=<?php echo $boardName; ?>"><span class="buttonclass2" border=0 alt=发表一个新投票></span>&nbsp;&nbsp;<a href="postarticle.php?board=<?php echo $boardName; ?>&reID=<?php echo $article['ID']; ?>"><span class="buttonclass4" border=0 alt=回复本主题></span></a>
	</td>
	<td align=right valign=middle><a href="disparticle.php?boardName=<?php echo $boardName; ?>&ID=<?php echo $articleID>1?$articleID-1:1; ?>"><img src="pic/prethread.gif" border=0 alt=浏览上一篇主题 width=52 height=12></a>&nbsp;
	<a href="javascript:this.location.reload()"><img src="pic/refresh.gif" border=0 alt=刷新本主题 width=40 height=12></a> &nbsp;
<?php
	if ($listType==1) {
?>
	<a href="?boardName=<?php echo $boardName; ?>&ID=<?php echo $articleID; ?>&start=<?php echo (ceil(($threadID+1)/THREADSPERPAGE)-1)*THREADSPERPAGE;; ?>&listType=0"><img src="pic/flatview.gif" width=40 height=12 border=0 alt=平板显示贴子></a>
<?php
	} else {
?>
	<a href="?boardName=<?php echo $boardName; ?>&ID=<?php echo $articleID; ?>&start=<?php echo $threadID; ?>&listType=1"><img src="pic/treeview.gif" width=40 height=12 border=0 alt=树形显示贴子></a>
<?php
	}
?>
	　<a href="disparticle.php?boardName=<?php 	echo $boardName; ?>&ID=<?php echo $articleID<bbs_countarticles($boardID, $dir_modes['ORIGIN'])?$articleID+1:$articleID; ?>"><img src="pic/nextthread.gif" border=0 alt=浏览下一篇主题 width=52 height=12></a>
	</td>
	</tr>
</table>
<?php
}

function dispArticleTitle($boardName,$boardID,$articleID,$article, $threadID){
?>
<TABLE cellPadding=0 cellSpacing=1 align=center class=tableborder1>
	<tr align=middle> 
	<td align=left valign=middle width="100%" height=25>
		<table width=100% cellPadding=0 cellSpacing=0 border=0>
		<tr>
		<th align=left valign=middle width="73%" height=25>
		&nbsp;* 文章主题</B>： <?php echo $article['TITLE']; ?></th>
		<th width=37% align=right>
		<a href=# onclick="javascript:WebBrowser.ExecWB(4,1)"><img src="pic/saveas.gif" border=0 width=16 height=16 alt=保存该页为文件 align=absmiddle></a>&nbsp;<object id="WebBrowser" width=0 height=0 classid="CLSID:8856F961-340A-11D0-A96B-00C04FD705A2"></object>
		<a href="report.asp?BoardID=1&id=1"><img src=pic/report.gif alt=报告本帖给版主 border=0></a>&nbsp;
		<a href="printpage.asp?BoardID=1&id=1"><img src="pic/printpage.gif" alt=显示可打印的版本 border=0></a>&nbsp;
		<a href="pag.asp?BoardID=1&id=1"><img src="pic/pag.gif" border=0 alt=把本贴打包邮递></a>&nbsp;
		<a href="favadd.asp?BoardID=1&id=1"><IMG SRC="pic/fav_add.gif" BORDER=0 alt=把本贴加入论坛收藏夹></a>&nbsp;
		<a href="sendpage.asp?BoardID=1&id=1"><img src="pic/emailtofriend.gif" border=0 alt=发送本页面给朋友></a>&nbsp;
		<a href=#><span style="CURSOR: hand" onClick="window.external.AddFavorite('http://www.dvbbs.net//dispbbs.asp?BoardID=1&id=1', ' 动网先锋论坛 - 2341')"><IMG SRC="pic/fav_add1.gif" BORDER=0 width=15 height=15 alt=把本贴加入IE收藏夹></a>&nbsp;
		</th>
		</tr>
		</table>
	</td>
	</tr>
</table>
<?php
}

function showArticleThreads($boardName,$boardID,$articleID,$article,$start,$listType) {
	global $dir_modes;
	$threads=bbs_get_threads_from_id($boardID, intval($article['ID']), $dir_modes["NORMAL"], 50000);
	if ($threads!=NULL) {
		$total=count($threads)+1;
	} else {
		$total=1;
	}
	$totalPages=ceil(($total)/THREADSPERPAGE);
	$num=THREADSPERPAGE;
	if ($start<0) {
		$start=0;
	} elseif ($start>=$total) {
		$start=$total-1;
	}
	if (($start+$num)>$total) {
		$num=$total-$start;
	}
	$page=ceil(($start+1)/THREADSPERPAGE);
	article_bar($boardName,$boardID,$articleID, $article, $start, $listType);
	dispArticleTitle($boardName,$boardID,$articleID,$article,$start);
?>
<TABLE cellPadding=5 cellSpacing=1 align=center class=tableborder1 style=" table-layout:fixed;word-break:break-all">
<?php
	if ($listType==1) {
		$num=1;
	} 
	for($i=0;$i<$num;$i++) {
		if (($i+$start)==0) {
			showArticle($boardName,$boardID,$i+$start,intval($article['ID']), $article);
		} else {
			showArticle($boardName,$boardID,$i+$start,intval($threads[$start+$i-1]['ID']),$threads[$start+$i-1]);
		}
	}
?>
</table>
<table cellpadding=0 cellspacing=3 border=0 width=97% align=center><tr><td valign=middle nowrap>本主题贴数<b><?php echo $total; ?></b>
<?php
	if ($listType!=1) {
?>
，分页： 
<?php
		if ($page>4) {
			echo "<a href=\"?boardName=".$boardName."&ID=".$articleID."&start=0\">[1]</a> ";
			if ($page>5) {
				echo "...";
			}
		} 

		if ($totalPages>$page+3){
			$endpage=$page+3;
		}  else{
			$endpage=$totalPages;
		} 

		for ($i=($page-3>0)?($page-3):1; $i<=$endpage; $i++){
			if ($i==$page)   {
				echo " <font color=#ff0000>[".$i."]</font>";
			} else {
				echo " <a href=\"?boardName=".$boardName."&ID=".$articleID."&start=".($i-1)*THREADSPERPAGE."\">[".$i."]</a>";
			} 
		} 

		if ($endpage<$totalPages) {
			if ($endpage<$totalPages-1){
				echo "...";
			}
			echo " <a href=\"?boardName=".$boardName."&ID=".$articleID."&start=".($totalPages-1)*THREADSPERPAGE."\">[".$totalPages."]</a>";
		} 
	}
?></td><td valign=middle nowrap align=right>
<?php 
	boardJump();
?></td></tr></table>
<br>
<?php
	if ($listType==1) {
		showArticleTree($boardName,$boardID,$articleID,$article,$threads,$total,$start);
	}
}

function showArticle($boardName,$boardID,$num, $threadID,$thread){
?>
<tr><td class=tablebody1 valign=top width=175>
<table width=100% cellpadding=4 cellspacing=0>
<tr><td width=* valign=middle style="filter:glow(color=#9898BA,strength=2)">&nbsp;<a name=1><font color=#990000><B><?php echo $thread['OWNER']; ?></B></font></a>	</td>
<td width=25 valign=middle><img src=pic/ofmale.gif alt=帅哥哟，离线，有人找我吗？></td>
<td width=16 valign=middle></td></tr></table>&nbsp;&nbsp;<img src=userface/image1.gif width=32 height=32><br>&nbsp;&nbsp;<img src=pic/level10.gif><br>&nbsp;&nbsp;等级：版主<BR>&nbsp;&nbsp;文章：2<br>&nbsp;&nbsp;积分：60<br>&nbsp;&nbsp;注册：2003-5-27<BR></td>

<td class=tablebody1 valign=top width=*>

<table width=100% ><tr><td width=*><a href="javascript:openScript('messanger.asp?action=new&touser=Roy',500,400)"><img src="pic/message.gif" border=0 alt="给Roy发送一个短消息"></a>&nbsp;<a href="friendlist.asp?action=addF&myFriend=Roy" target=_blank><img src="pic/friend.gif" border=0 alt="把Roy加入好友"></a>&nbsp;<a href="dispuser.asp?id=4" target=_blank><img src="pic/profile.gif" border=0 alt="查看Roy的个人资料"></a>&nbsp;<a href="queryResult.asp?stype=1&nSearch=3&keyword=Roy&BoardID=1&SearchDate=ALL" target=_blank><img src="pic/find.gif" border=0 alt="搜索Roy在测试的所有贴子"></a>&nbsp;<A href="mailto:roy@zixia.net"><IMG alt="点击这里发送电邮给Roy" border=0 src=pic/email.gif></A>&nbsp;<a href="editarticle.php?board=<?php echo $boardName; ?>&reID=<?php echo $thread['ID']; ?>"><img src="pic/edit.gif" border=0 alt=编辑></a>&nbsp;<a href="postarticle.php?board=<?php echo $boardName; ?>&reID=<?php echo $thread['ID']; ?>"><img src="pic/reply_a.gif" border=0 alt=回复这个贴子></a></td><td width=50><b><?php echo $num==0?'楼主':'第<font color=#ff0000>'.$num.'</font>楼'; ?></b></td></tr><tr><td bgcolor=#D8C0B1 height=1 colspan=2></td></tr>
</table>

<blockquote><table class=tablebody2 border=0 width=90% style=" table-layout:fixed;word-break:break-all"><tr><td width="100%" style="font-size:9pt;line-height:12pt"><img src=face/face1.gif border=0 alt=发贴心情>&nbsp;<?php echo  $thread['TITLE']; ?><b></b><br><?php 
	 $isnormalboard=bbs_normalboard($boardName);
	 $filename=bbs_get_board_filename($boardName, $thread["FILENAME"]);
	 if ($loginok) {
		 bbs_brcaddread($boardName, $thread['ID']);
	 };
	 bbs_printansifile($filename,1,'bbscon?bid='.$boardID.'&id='.$thread['ID']);
?></blockquote></td></tr></table>
</td>

</tr>

<tr><td class=tablebody1 valign=middle align=center width=175><a href=look_ip.asp?boardid=1&userid=4&ip=*.*.*.*&action=lookip target=_blank><img align=absmiddle border=0 width=13 height=15 src="pic/ip.gif" alt="点击查看用户来源及管理<br>发贴IP：*.*.*.*"></a> 2003-6-2 14:04:17</td><td class=tablebody1 valign=middle width=*><table width=100% cellpadding=0 cellspacing=0><tr><td align=left valign=middle> &nbsp;&nbsp;<a href="postagree.asp?boardid=1&id=1&isagree=1" title="同意该帖观点，给他一朵鲜花，将消耗您5点金钱"><img src=pic/xianhua.gif border=0>鲜花</a>(<font color=#FF0000>0</font>)&nbsp;&nbsp;<a href="postagree.asp?boardid=1&id=1&isagree=2" title="不同意该帖观点，给他一个鸡蛋，将消耗您5点金钱"><img src=pic/jidan.gif border=0>鸡蛋</a>(<font color=#FF0000>0</font>)</td><td align=right nowarp valign=bottom width=200></td><td align=right valign=bottom width=4> </td></tr>
</table>
</td></tr>


<?php
}

function showArticleTree($boardName,$boardID,$articleID,$article,$threads,$threadNum,$start) {
?>
<table cellpadding=3 cellspacing=1 class=tableborder1 align=center>
<tr><th align=left width=90% valign=middle> &nbsp;*树形目录</th>
<th width=10% align=right valign=middle height=24 id=tabletitlelink> <a href=#top><img src=pic/gotop.gif border=0>顶端</a>&nbsp;</th></tr>
<?php
	$flags=array();
	showTreeItem($boardName,$articleID,$article,0,$start,$flags);
	for ($i=1;$i<$threadNum;$i++){
		showTreeItem($boardName,$articleID,$threads[$i-1],$i,$start,$flags);
	}
?>
</table>
<?php
}

function showTreeItem($boardName,$articleID,$thread,$threadID,$start,&$flags){
	if (isset($flags[$thread['REID']]) ){
		$flags[$thread['ID']]=$flags[$thread['REID']]+1;
	} else {
		$flags[$thread['ID']]=0;
	}
	echo '<TR><td class=tablebody2 width="100%" height=22 colspan=2>';
	for ($i=0;$i<$flags[$thread['ID']];$i++) {
		echo "&nbsp;&nbsp;";
	}
	if ($threadID==0) {
		echo '主题';
	} else {
		echo '回复';
	}
	echo '：&nbsp;<img src=face/face1.gif height=16 width16>  <a href="disparticle.php?boardName='.$boardName.'&ID='.$articleID.'&start='.$threadID.'&listType=1">';
	if ($start==$threadID) {
		echo "<font color=#FF0000>";
	}
	echo $thread['TITLE'].'</a><I><font color=gray>(37字) － <a href=dispuser.php?name='.$thread['OWNER'].' target=_blank title="作者资料"><font color=gray>'.$thread['OWNER'].'</font></a>，'.strftime("%Y年%m月%d日",$thread['POSTTIME']);
	if ($start==$threadID) {
		echo "</font>";
	}
	echo '</I></td></tr>';	
}
?>