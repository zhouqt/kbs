<?php


$needlogin=1;

require("inc/funcs.php");
require("inc/user.inc.php");
require("inc/board.inc.php");
require("inc/ubbcode.php");

global $boardArr;
global $boardID;
global $boardName;
global $article;
global $articleID;
global $start;
global $listType;

preprocess();

setStat(htmlspecialchars($article['TITLE'],ENT_QUOTES));

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
	$articles = bbs_getthreads($boardName, $articleID, 1);
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
	　<a href="disparticle.php?boardName=<?php 	echo $boardName; ?>&ID=<?php echo $articleID<bbs_getThreadNum($boardID)?$articleID+1:$articleID; ?>"><img src="pic/nextthread.gif" border=0 alt=浏览下一篇主题 width=52 height=12></a>
	</td>
	</tr>
</table>
<?php
}

function dispArticleTitle($boardName,$boardID,$articleID,$article, $threadID){
	global $SiteURL, $SiteName;
?>
<TABLE cellPadding=0 cellSpacing=1 align=center class=tableborder1>
	<tr align=middle> 
	<td align=left valign=middle width="100%" height=25>
		<table width=100% cellPadding=0 cellSpacing=0 border=0>
		<tr>
		<th align=left valign=middle width="73%" height=25>
		&nbsp;* 文章主题</B>： <?php echo htmlspecialchars($article['TITLE'],ENT_QUOTES); ?></th>
		<th width=37% align=right>
		<a href=# onclick="alert('本功能尚未实现');"><img src="pic/saveas.gif" border=0 width=16 height=16 alt=保存该页为文件 align=absmiddle></a>&nbsp;
		<a href=# onclick="alert('本功能尚未实现');"><img src=pic/report.gif alt=报告本帖给版主 border=0></a>&nbsp;
		<a href=# onclick="alert('本功能尚未实现');"><img src="pic/printpage.gif" alt=显示可打印的版本 border=0></a>&nbsp;
		<a href=# onclick="alert('本功能尚未实现');"><img src="pic/pag.gif" border=0 alt=把本贴打包邮递></a>&nbsp;
		<a href=# onclick="alert('本功能尚未实现');"><IMG SRC="pic/fav_add.gif" BORDER=0 alt=把本贴加入论坛收藏夹></a>&nbsp;
		<a href=# onclick="alert('本功能尚未实现');"><img src="pic/emailtofriend.gif" border=0 alt=发送本页面给朋友></a>&nbsp;
		<a href=#><span style="CURSOR: hand" onClick="window.external.AddFavorite(location.href, document.title);"><IMG SRC="pic/fav_add1.gif" BORDER=0 width=15 height=15 alt=把本贴加入IE收藏夹></a>&nbsp;
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
	$threadNum=bbs_get_thread_article_num($boardName,intval($article['ID']));
	$total=$threadNum+1;

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
	if ($listType==1) {
		$num=1;
	} 
	$page=ceil(($start+1)/THREADSPERPAGE);
	$threads=bbs_get_thread_articles($boardName, intval($article['ID']), 	$total-$start-$num,$num);
	$num=count($threads);
	article_bar($boardName,$boardID,$articleID, $article, $start, $listType);
	dispArticleTitle($boardName,$boardID,$articleID,$article,$start);
?>
<TABLE cellPadding=5 cellSpacing=1 align=center class=tableborder1 style=" table-layout:fixed;word-break:break-all">
<?php
	for($i=0;$i<$num;$i++) {
			showArticle($boardName,$boardID,$i+$start,intval($threads[$num-$i-1]['ID']),$threads[$num-$i-1]);
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
		$threadNum=bbs_get_thread_article_num($boardName,intval($article['ID']));
		$total=$threadNum+1;
		$threads=bbs_get_thread_articles($boardName, intval($article['ID']),0,$total);
		$total=count($threads);
		showArticleTree($boardName,$boardID,$articleID,$article,$threads,$total,$start);
	}
}

function showArticle($boardName,$boardID,$num, $threadID,$thread){
	$user=array();
	$user_num=bbs_getuser($thread['OWNER'],$user);
?>
<tr><td class=tablebody1 valign=top width=175>
<table width=100% cellpadding=4 cellspacing=0>
<tr><td width=* valign=middle style="filter:glow(color=#9898BA,strength=2)">&nbsp;<a name=1><font color=#990000><B><?php echo $thread['OWNER']; ?></B></font></a>	</td>
<td width=25 valign=middle><img src=pic/ofmale.gif alt=帅哥哟，离线，有人找我吗？></td>
<td width=16 valign=middle></td></tr></table>&nbsp;&nbsp;<img src=userface/image1.gif width=32 height=32><br>&nbsp;&nbsp;<img src=pic/level10.gif><br>&nbsp;&nbsp;等级：<?php echo bbs_getuserlevel($thread['OWNER']); ?><BR>&nbsp;&nbsp;文章：<?php echo $user['numposts']; ?><br>&nbsp;&nbsp;积分：<?php echo $user['score']; ?><br>&nbsp;&nbsp;注册：<?php echo strftime('%Y-%m-%d',$user['firstlogin']); ?><BR></td>

<td class=tablebody1 valign=top width=*>

<table width=100% ><tr><td width=* valign='center'><a href="javascript:replyMsg('<?php echo $thread['OWNER']; ?>')"><img src="pic/message.gif" border=0 alt="给Roy发送一个短消息"></a>&nbsp;<a href=# onclick="alert('本功能尚未实现');" target=_blank><img src="pic/friend.gif" border=0 alt="把Roy加入好友"></a>&nbsp;<a href="dispuser.asp?id=4" target=_blank><img src="pic/profile.gif" border=0 alt="查看Roy的个人资料"></a>&nbsp;<a href=# onclick="alert('本功能尚未实现');" target=_blank><img src="pic/find.gif" border=0 alt="搜索Roy在测试的所有贴子"></a>&nbsp;<A href="sendmail.php?board=<?php echo $boardName; ?>&reID=<?php echo $thread['ID']; ?>"><IMG alt="点击这里发送电邮给Roy" border=0 src=pic/email.gif></A>&nbsp;<a href="editarticle.php?board=<?php echo $boardName; ?>&reID=<?php echo $thread['ID']; ?>"><img src="pic/edit.gif" border=0 alt=编辑></a>&nbsp;<a href="deletearticle.php?board=<?php echo $boardName; ?>&ID=<?php echo $thread['ID']; ?>" onclick="return confirm('你真的要删除本文吗?')"><img src="pic/delete.gif" border=0 alt=删除>删除</a>&nbsp;<a href="postarticle.php?board=<?php echo $boardName; ?>&reID=<?php echo $thread['ID']; ?>"><img src="pic/reply_a.gif" border=0 alt=回复这个贴子></a></td><td width=50><b><?php echo $num==0?'楼主':'第<font color=#ff0000>'.$num.'</font>楼'; ?></b></td></tr><tr><td bgcolor=#D8C0B1 height=1 colspan=2></td></tr>
</table>

<blockquote><table class=tablebody2 border=0 width=90% style=" table-layout:fixed;word-break:break-all"><tr><td width="100%" style="font-size:9pt;line-height:12pt"><img src=face/face1.gif border=0 alt=发贴心情>&nbsp;<?php echo  htmlspecialchars($thread['TITLE'],ENT_QUOTES); ?><b></b><br><?php 
	 $isnormalboard=bbs_normalboard($boardName);
	 $filename=bbs_get_board_filename($boardName, $thread["FILENAME"]);
	 if ($loginok) {
		 bbs_brcaddread($boardName, $thread['ID']);
	 };
	 echo dvbcode(bbs_printansifile($filename,1,'bbscon?bid='.$boardID.'&id='.$thread['ID']),0);

?></blockquote></td></tr></table>
</td>

</tr>

<tr><td class=tablebody1 valign=middle align=center width=175><a href=# onclick="alert('本功能尚未实现');" target=_blank><img align=absmiddle border=0 width=13 height=15 src="pic/ip.gif" alt="点击查看用户来源及管理<br>发贴IP：*.*.*.*"></a> <?php echo strftime("%Y-%m-%d %H:%M:%S",$thread['POSTTIME']); ?></td><td class=tablebody1 valign=middle width=*><table width=100% cellpadding=0 cellspacing=0><tr><td align=left valign=middle> &nbsp;&nbsp;<a href=# onclick="alert('本功能尚未实现');" title="同意该帖观点，给他一朵鲜花，将消耗您5点金钱"><img src=pic/xianhua.gif border=0>鲜花</a>(<font color=#FF0000>0</font>)&nbsp;&nbsp;<a href=# onclick="alert('本功能尚未实现');" title="不同意该帖观点，给他一个鸡蛋，将消耗您5点金钱"><img src=pic/jidan.gif border=0>鸡蛋</a>(<font color=#FF0000>0</font>)</td><td align=right nowarp valign=bottom width=200></td><td align=right valign=bottom width=4> </td></tr>
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
	$IDs=array();
	$nodes=array();
	$printed=array();
	$level=array();
	$head=0;
	$bottom=0;
	$IDs[$bottom]=intval($threads[$threadNum-1]['ID']);
	$level[$bottom]=0;
	$printed[0]=1;
	$nodes[0]=0;
	$bottom++;
	while($head<$bottom) {
		showTreeItem($boardName,$articleID,$threads[$threadNum-$nodes[$head]-1],$nodes[$head],$start, $level[$head]);
		for ($i=0;$i<$threadNum;$i++){
			if ( (!isset($printed[$i])) && ($threads[$threadNum-$i-1]['REID']==$IDs[$head]) ) {
				$IDs[$bottom]=intval($threads[$threadNum-$i-1]['ID']);
				$level[$bottom]=$level[$head]+1;
				$printed[$i]=1;
				$nodes[$bottom]=$i;
				$bottom++;
			}
		}
		$head++;
	}
?>
</table>
<?php
}


function showTreeItem($boardName,$articleID,$thread,$threadID,$start,$level){

	echo '<TR><td class=tablebody2 width="100%" height=22 colspan=2>';
	for ($i=0;$i<$level;$i++) {
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
	echo htmlspecialchars($thread['TITLE'],ENT_QUOTES).'</a><I><font color=gray>(37字) － <a href=dispuser.php?name='.$thread['OWNER'].' target=_blank title="作者资料"><font color=gray>'.$thread['OWNER'].'</font></a>，'.strftime("%Y年%m月%d日",$thread['POSTTIME']);
	if ($start==$threadID) {
		echo "</font>";
	}
	echo '</I></td></tr>';	
}
?>