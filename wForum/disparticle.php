<?php


if ( (!isset($_COOKIE['iscookies'])) || ($_COOKIE['iscookies']==''))
{
	setcookie('iscookies','0',time()+3650*24*3600);
	print '<META http-equiv=Content-Type content=text/html; charset=gb2312><meta HTTP-EQUIV=REFRESH CONTENT=3>正在登陆论坛……<br><br>本系统要求使用COOKIES，假如您的浏览器禁用COOKIES，您将不能登录本系统……';
	exit();
} 

$needlogin=1;

require("inc/funcs.php");
require("inc/user.inc.php");
require("inc/board.inc.php");

global $boardArr;
global $boardID;
global $boardName;
global $article;
global $articleID;
global $page;

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

	article_bar($boardName,$boardID,$articleID,$article);

	dispArticleTitle($boardName,$boardID,$articleID,$article);

	showArticleThreads($boardName,$boardID,$articleID,$article,$page);
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
	if (!isset($_GET['page'])) {
		$page=1;
	} else {
		$page=intval($_GET['page']);
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

function article_bar($boardName,$boardID,$articleID,$article){
	global $dir_modes;
?>
<table cellpadding=0 cellspacing=0 border=0 width=97% align=center>
	<tr>
	<td align=left width="30%" valign=middle>&nbsp; 
	<a href="announce.asp?name=<?php echo $boardName; ?>"><span class="buttonclass1" border=0 alt=发表一个新主题></span></a>&nbsp;&nbsp;<a href="vote.php?name=<?php echo $boardName; ?>"><span class="buttonclass2" border=0 alt=发表一个新投票></span>&nbsp;&nbsp;<a href="reply?name=<?php echo $boardName; ?>&id=<?php echo $article['ID']; ?>"><span class="buttonclass4" border=0 alt=回复本主题></span></a>
	</td>
	<td align=right width="70%" valign=middle><a href="disparticle.php?boardName=<?php echo $boardName; ?>&ID=<?php echo $articleID>1?$articleID-1:1; ?>"><img src="pic/prethread.gif" border=0 alt=浏览上一篇主题 width=52 height=12></a>&nbsp;
	<a href="javascript:this.location.reload()"><img src="pic/refresh.gif" border=0 alt=刷新本主题 width=40 height=12></a> &nbsp;
	<a href="?BoardID=1&replyID=1&id=1&star=1&skin=1"><img src="pic/treeview.gif" width=40 height=12 border=0 alt=树形显示贴子></a>　<a href="disparticle.php?boardName=<?php 
	echo $boardName; ?>&ID=<?php echo $articleID<bbs_countarticles($boardID, $dir_modes['ORIGIN'])?$articleID+1:$articleID; ?>"><img src="pic/nextthread.gif" border=0 alt=浏览下一篇主题 width=52 height=12></a>
	</td>
	</tr>
</table>
<?php
}

function dispArticleTitle($boardName,$boardID,$articleID,$article){
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

function showArticleThreads($boardName,$boardID,$articleID,$article,$page) {
	global $dir_modes;
?>
<TABLE cellPadding=5 cellSpacing=1 align=center class=tableborder1 style=" table-layout:fixed;word-break:break-all">
<?php
	$threads=bbs_get_threads_from_id($boardID, intval($article['ID']), $dir_modes["NORMAL"], 100000);
	$total=count($threads);
	$totalPages=ceil($total/THREADSPERPAGE);
	if (($page>$totalPages)) {
		$page=$totalPages;
	} else if ($page<1) {
		$page=1;
	}
	$start=($page-1)* THREADSPERPAGE;
	$num=THREADSPERPAGE;
	if (($start+$num)>$total) {
		$num=$total-$start;
	}
	if ($start==0) {
		showArticle($boardName,$boardID,intval($article['ID']), $article);
		if ($num==THREADSPERPAGE){
			$num--;
		}
	}
	for($i=0;$i<$num;$i++) {
		showArticle($boardName,$boardID,intval($threads[$start+$i]['ID']),$threads[$start+$i]);
	}
?>
</table>
<?php
}

function showArticle($boardName,$boardID,$threadID,$thread){
?>
<tr><td class=tablebody1 valign=top width=175>
<table width=100% cellpadding=4 cellspacing=0>
<tr><td width=* valign=middle style="filter:glow(color=#9898BA,strength=2)">&nbsp;<a name=1><font color=#990000><B><?php echo $thread['OWNER']; ?></B></font></a>	</td>
<td width=25 valign=middle><img src=pic/ofmale.gif alt=帅哥哟，离线，有人找我吗？></td>
<td width=16 valign=middle></td></tr></table>&nbsp;&nbsp;<img src=userface/image1.gif width=32 height=32><br>&nbsp;&nbsp;<img src=pic/level10.gif><br>&nbsp;&nbsp;等级：版主<BR>&nbsp;&nbsp;文章：2<br>&nbsp;&nbsp;积分：60<br>&nbsp;&nbsp;注册：2003-5-27<BR></td>

<td class=tablebody1 valign=top width=*>

<table width=100% ><tr><td width=*><a href="javascript:openScript('messanger.asp?action=new&touser=Roy',500,400)"><img src="pic/message.gif" border=0 alt="给Roy发送一个短消息"></a>&nbsp;<a href="friendlist.asp?action=addF&myFriend=Roy" target=_blank><img src="pic/friend.gif" border=0 alt="把Roy加入好友"></a>&nbsp;<a href="dispuser.asp?id=4" target=_blank><img src="pic/profile.gif" border=0 alt="查看Roy的个人资料"></a>&nbsp;<a href="queryResult.asp?stype=1&nSearch=3&keyword=Roy&BoardID=1&SearchDate=ALL" target=_blank><img src="pic/find.gif" border=0 alt="搜索Roy在测试的所有贴子"></a>&nbsp;<A href="mailto:roy@zixia.net"><IMG alt="点击这里发送电邮给Roy" border=0 src=pic/email.gif></A>&nbsp;<a href="reannounce.asp?BoardID=1&replyID=1&id=1&star=1&reply=true"><img src="pic/reply.gif" border=0 alt=引用回复这个贴子></a>&nbsp;<a href="reannounce.asp?BoardID=1&replyID=1&id=1&star=1"><img src="pic/reply_a.gif" border=0 alt=回复这个贴子></a></td><td width=50><b>楼主</b></td></tr><tr><td bgcolor=#D8C0B1 height=1 colspan=2></td></tr>
</table>

<blockquote><table class=tablebody2 border=0 width=90% style=" table-layout:fixed;word-break:break-all"><tr><td width="100%" style="font-size:9pt;line-height:12pt"><img src=face/face1.gif border=0 alt=发贴心情>&nbsp;<?php echo  $thread['TITLE']; ?><b></b><br><?php 
	 $isnormalboard=bbs_normalboard($boardName);
	 $filename=bbs_get_board_filename($boardName, $thread["FILENAME"]);
	 if ($loginok) {
		 bbs_brcaddread($boardName, $thread['ID']);
	 };
	 bbs_printansifile($filename,1,$_SERVER['PHP_SELF'] . '?' . $_SERVER['QUERY_STRING']);
?></blockquote></td></tr></table>
</td>

</tr>

<tr><td class=tablebody1 valign=middle align=center width=175><a href=look_ip.asp?boardid=1&userid=4&ip=*.*.*.*&action=lookip target=_blank><img align=absmiddle border=0 width=13 height=15 src="pic/ip.gif" alt="点击查看用户来源及管理<br>发贴IP：*.*.*.*"></a> 2003-6-2 14:04:17</td><td class=tablebody1 valign=middle width=*><table width=100% cellpadding=0 cellspacing=0><tr><td align=left valign=middle> &nbsp;&nbsp;<a href="postagree.asp?boardid=1&id=1&isagree=1" title="同意该帖观点，给他一朵鲜花，将消耗您5点金钱"><img src=pic/xianhua.gif border=0>鲜花</a>(<font color=#FF0000>0</font>)&nbsp;&nbsp;<a href="postagree.asp?boardid=1&id=1&isagree=2" title="不同意该帖观点，给他一个鸡蛋，将消耗您5点金钱"><img src=pic/jidan.gif border=0>鸡蛋</a>(<font color=#FF0000>0</font>)</td><td align=right nowarp valign=bottom width=200></td><td align=right valign=bottom width=4> </td></tr>
</table>
</td></tr>


<?php
}
?>