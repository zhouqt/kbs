<?php

require("inc/funcs.php");
require("inc/user.inc.php");
require("inc/board.inc.php");
require("inc/ubbcode.php");
require("inc/userdatadefine.inc.php");
require("inc/treeview.inc.php");
require_once("inc/myface.inc.php");

global $boardArr;
global $boardID;
global $boardName;
global $articles;
global $groupID;
global $start;
global $listType;
global $isbm;
global $total;
global $num;
global $is_tex;

setStat("文章阅读");

preprocess();

setStat(htmlspecialchars($articles[0]['TITLE'] ,ENT_QUOTES) . " " );

show_nav($boardName,$is_tex,getBoardRSS($boardName));

showUserMailBoxOrBR();
board_head_var($boardArr['DESC'],$boardName,$boardArr['SECNUM']);
showArticleThreads($boardName,$boardID,$groupID,$articles,$start,$listType,$total,$num);

show_footer();

function preprocess(){
	global $boardID;
	global $boardName;
	global $currentuser;
	global $boardArr;
	global $articles;
	global $groupID;
	global $dir_modes;
	global $listType;
	global $start;
	global $isbm;
	global $total;
	global $num;
	global $is_tex;
	if (!isset($_GET['boardName'])) {
		foundErr("未指定版面。");
	}
	$boardName=$_GET['boardName'];
	$brdArr=array();
	$boardID= bbs_getboard($boardName,$brdArr);
	$boardArr=$brdArr;
	$boardName=$brdArr['NAME'];
	if ($boardID==0) {
		foundErr("指定的版面不存在");
	}
	$usernum = $currentuser["index"];
	if (bbs_checkreadperm($usernum, $boardID) == 0) {
		foundErr("您无权阅读本版");
	}
	if (!isset($_GET['ID'])) {
		foundErr("您指定的文章不存在！");
	} else {
		$groupID=intval($_GET['ID']);
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

	$isbm = bbs_is_bm($boardID, $usernum);
	
	bbs_set_onboard($boardID,1);
	
	$num = bbs_get_threads_from_gid($boardID, $groupID, $groupID, $articles, $haveprev );
	if ($num==0) {
		foundErr("您指定的文章不存在！");
	}
	if ($start < 0) $start = 0;
	if ($start >= $num) $start = $num - 1;

	$total=count($articles);

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
	
	$is_tex = 0;
	if (SUPPORT_TEX) {
		for($i=0;$i<$num;$i++) {
			if ($articles[$i+$start]["IS_TEX"]) {
				$is_tex = 1;
				break;
			}
		}
	}
	
	return true;
}

function article_bar($boardName,$boardID,$groupID,$article,$startNum,$listType){
	global $dir_modes;
?>
<table cellpadding="2" cellspacing="0" border="0" width="97%" align="center">
	<tr><td width="2"> </td>
	<td align="left" valign="middle" style="height:27"><table cellpadding="0" cellspacing="0" border="0" ><tr>
	<td width="110"><a href="postarticle.php?board=<?php echo $boardName; ?>"><div class="buttonClass1" border="0" title="发新帖"></div></a></td>
<!--	<td width="110"><a href="#" onclick="alert('本功能尚在开发中！')"><div class="buttonClass2" border="0" title="发起新投票"></div></a></td>-->
	<td width="110"><a href="postarticle.php?board=<?php echo $boardName; ?>&amp;reID=<?php echo $article['ID']; ?>"><div class="buttonClass4" border="0" title="回复本主题"></div></a></td>
	</tr></table>
	</td>
	<td align="right" valign="middle">
<!--
	<a href="disparticle.php?boardName=<?php echo $boardName; ?>&ID=<?php echo $groupID>1?$groupID-1:1; ?>"><img src="pic/prethread.gif" border="0" title="浏览上一篇主题" width="52" height="12"/></a>&nbsp;
-->
	<a href="javascript:this.location.reload()"><img src="pic/refresh.gif" border="0" title="刷新本主题" width="40" height="12"/></a> &nbsp;
<?php
	if ($listType==1) {
?>
	<a href="?boardName=<?php echo $boardName; ?>&amp;ID=<?php echo $groupID; ?>&amp;start=<?php echo (ceil(($startNum+1)/THREADSPERPAGE)-1)*THREADSPERPAGE;; ?>&amp;listType=0"><img src="pic/flatview.gif" width="40" height="12" border="0" title="平板显示贴子"/></a>
<?php
	} else {
?>
	<a href="?boardName=<?php echo $boardName; ?>&amp;ID=<?php echo $groupID; ?>&amp;start=<?php echo $startNum; ?>&amp;listType=1"><img src="pic/treeview.gif" width="40" height="12" border="0" title="树形显示贴子"/></a>
<?php
	}
?>
<!--
	　<a href="disparticle.php?boardName=<?php 	echo $boardName; ?>&ID=<?php echo $groupID<bbs_getThreadNum($boardID)?$groupID+1:$groupID; ?>"><img src="pic/nextthread.gif" border="0" title="浏览下一篇主题" width="52" height="12"/></a>
-->
	</td>
	</tr>
</table>
<?php
}

function dispArticleTitle($boardName,$boardID,$groupID,$article, $startNum){
?>
<table cellPadding="0" cellSpacing="1" align="center" class="TableBorder1">
	<tr align="middle"> 
	<td align="left" valign="middle" width="100%" height="25">
		<table width="100%" cellPadding="0" cellSpacing="0" border="0">
		<tr>
		<th align="left" valign="middle" width="73%" height="25">
		&nbsp;* 文章主题： <?php echo htmlspecialchars($article['TITLE'],ENT_QUOTES); ?>  
<?php /* 这里有问题，待讨论 
<script language="JavaScript">
<!--
	var flags = "<?php echo $article['FLAGS'][0]; ?>";
	if (article_is_zhiding(flags)) {
		document.write("[固顶]");
	} else if(article_is_noreply(flags)) {
		document.write("[锁定]");
	} else if(article_is_digest(flags)) {
		document.write("[精华]");
	}
//-->
</script>
*/ ?>
		</th>
		<th width="37%" align="right">
<?php
/*
		<a href=# onclick="alert('本功能尚未实现');"><img src="pic/saveas.gif" border=0 title=保存该页为文件></a>&nbsp;
		<a href=# onclick="alert('本功能尚未实现');"><img src="pic/report.gif" title=报告本帖给版主 border=0></a>&nbsp;
		<a href=# onclick="alert('本功能尚未实现');"><img src="pic/printpage.gif" title=显示可打印的版本 border=0></a>&nbsp;
		<a href=# onclick="alert('本功能尚未实现');"><img src="pic/pag.gif" border=0 title=把本贴打包邮递></a>&nbsp;
		<a href=# onclick="alert('本功能尚未实现');"><IMG SRC="pic/fav_add.gif" BORDER=0 title=把本贴加入论坛收藏夹></a>&nbsp;
		<a href=# onclick="alert('本功能尚未实现');"><img src="pic/emailtofriend.gif" border=0 title=发送本页面给朋友></a>&nbsp;
*/
?>
		<a href="#" onClick="window.external.AddFavorite(location.href, document.title);"><img src="pic/fav_add1.gif" border="0" width="15" height="15" title="把本贴加入IE收藏夹"/></a>&nbsp;
		</th>
		</tr>
		</table>
	</td>
	</tr>
</table>
<?php
}

function showArticleThreads($boardName,$boardID,$groupID,$articles,$start,$listType,$total,$num) {
	global $dir_modes;
	$totalPages=ceil(($total)/THREADSPERPAGE);
	$page=ceil(($start+1)/THREADSPERPAGE);
	article_bar($boardName,$boardID,$groupID, $articles[0], $start, $listType);
	dispArticleTitle($boardName,$boardID,$groupID,$articles[0],$start);
?>
<table cellPadding="5" cellSpacing="1" align="center" class="TableBorder1" style=" table-layout:fixed;word-break:break-all">
<?php
	for($i=0;$i<$num;$i++) {
			if (($i+$start)==0) 
				showArticle($boardName,$boardID,0,intval($articles[0]['ID']),$articles[0],$i%2);
			else 
				showArticle($boardName,$boardID,$i+$start,intval($articles[$i+$start]['ID']),$articles[$i+$start],$i%2);
	}
?>
</table>
<table cellpadding="0" cellspacing="3" border="0" width="97%" align="center"><tr><td valign="middle" nowrap="nowarp">本主题贴数<b><?php echo $total; ?></b>
<?php
	if ($listType!=1) {
?>
，分页： 
<?php
		if ($page>4) {
			echo "<a href=\"?boardName=".$boardName."&amp;ID=".$groupID."&amp;start=0\">[1]</a>";
			if ($page>5) {
				echo " ...";
			}
		} 

		if ($totalPages>$page+3){
			$endpage=$page+3;
		}  else{
			$endpage=$totalPages;
		} 

		for ($i=($page-3>0)?($page-3):1; $i<=$endpage; $i++){
			if ($i==$page)   {
				echo " <font color=\"#ff0000\">[".$i."]</font>";
			} else {
				echo " <a href=\"?boardName=".$boardName."&amp;ID=".$groupID."&amp;start=".($i-1)*THREADSPERPAGE."\">[".$i."]</a>";
			} 
		} 

		if ($endpage<$totalPages) {
			if ($endpage<$totalPages-1){
				echo " ...";
			}
			echo " <a href=\"?boardName=".$boardName."&amp;ID=".$groupID."&amp;start=".($totalPages-1)*THREADSPERPAGE."\">[".$totalPages."]</a>";
		} 
	}
?></td><td valign="middle" nowrap="nowrap" align="right">
<?php 
	boardJump();
?></td></tr></table>
<br/>
<?php
	if ($listType==1) {
?>
<table cellpadding="3" cellspacing="1" class="TableBorder1" align="center">
<tr><th align="left" width="90%" valign="middle"> &nbsp;*树形目录</th>
<th width="10%" align="right" valign="middle" height="24" id="TableTitleLink"> <a href="#top"><img src="pic/gotop.gif" border="0"/>顶端</a>&nbsp;</th></tr>
<?php
		showTree($boardName,$groupID,$articles,"showTreeItem", TREEVIEW_MAXITEM, $start);
?>
</table>
<?php
	}
}

function showArticle($boardName,$boardID,$num, $startNum,$thread,$type){
	global $loginok;
	global $isbm;

	$bgstyle='TableBody'.($type+1);
	$fgstyle='TableBody'.(2-$type);

	/* 文章内容处理部分 */
	$filename=bbs_get_board_filename($boardName, $thread["FILENAME"]);
	if ($loginok) {
		bbs_brcaddread($boardName, $thread['ID']);
	};
	$is_tex = SUPPORT_TEX && $thread["IS_TEX"];
	$articleContents = bbs_printansifile($filename,1,'bbscon.php?bid='.$boardID.'&amp;id='.$thread['ID'],$is_tex,0);
	if (0) { /* 这部分各站点可以自行订制，下面给出一点示例 */
		/* 去掉第一行 */
		$chit = strpos( $articleContents, "标&nbsp;&nbsp;题:" );
		if ($chit !== false) $articleContents = substr($articleContents, $chit);
		
		/* 发文头都去掉 */
		$sf = "<br />  <br />";
		$chit = strpos( $articleContents, $sf );
		if ($chit !== false) $articleContents = substr($articleContents, $chit + strlen($sf));
	
		/* 去掉最后的来源 */
		$search=array("'<font class=\"f[0-9]+\">※&nbsp;来源:·.+\[FROM:&nbsp;[^\]]+\]</font><font class=\"f000\"> <br />'");
        $replace=array("");
        $articleContents = preg_replace($search,$replace,$articleContents);
        
        /* 显示发文时间 */
		$articleContents = "<b>发布于: ".strftime('%Y-%m-%d %H:%M:%S', intval($thread['POSTTIME']))."</b><br /><br />".$articleContents;
	}
	$articleContents = str_replace("&nbsp;", " ", $articleContents);
	$articleContents = str_replace("  ", " &nbsp;", $articleContents);
	$articleContents = DvbTexCode($articleContents,0,$fgstyle,$is_tex);
	/* 文章内容处理结束，此时 $articleContents 应该是能够直接输出的内容 */

	$user=array();
	$user_num=bbs_getuser($thread['OWNER'],$user);
	if ($user_num == 0) {
		$user = false;
	} else if ($thread['POSTTIME'] < $user['firstlogin']) {
		$user = false; //前人发的帖子
	}
?>
<tr><td class="<?php echo $bgstyle ;?>" valign="top" width="175" >
<table width="100%" cellpadding="2" cellspacing="0" >
<tr><td width="*" valign="middle" style="filter:glow(color=#9898BA,strength=2)" >&nbsp;
<?php
	$str = "<font color=\"#990000\"><b>" . $thread['OWNER'] . "</b></font>";
	if ($user !== false) {
		$str = "<a href=\"dispuser.php?id=" . $thread['OWNER']. "\" target=\"_blank\" title=\"查看" . $thread['OWNER'] . "的个人资料\" style=\"TEXT-DECORATION: none;\">" . $str . "</a>";
		if ($isbm) {
			$str .= "&nbsp;[<a href=\"bmdeny.php?board=".$boardName."&amp;userid=".$thread['OWNER']."\" title=\"封禁本文作者\"><font color=\"red\">封</font></a>]";
		}
	}
	echo "<nobr>".$str."</nobr>";
?>
</td>
<td width="25" valign="middle">
<?php
	if ($user !== false) {
		$is_online = bbs_isonline($thread['OWNER']);
		$show_detail = ($user['userdefine0'] & BBS_DEF_SHOWDETAILUSERDATA);
		if ($show_detail) {
			if ( chr($user['gender'])=='M' ){
				$c = "帅哥哟";
				if ($is_online) {
					$img = "pic/Male.gif";
				} else {
					$img = "pic/ofmale.gif";
				}
			} else {
				$c = "美女哟";
				if ($is_online) {
					$img = "pic/Female.gif";
				} else {
					$img = "pic/offemale.gif";
				}
			}
		} else {
			$c = "性别保密哟";
			if ($is_online) {
				$img = "pic/online1.gif";
			} else {
				$img = "pic/offline1.gif";
			}
		}
		if ($loginok && $is_online) {
			echo '<a href="javascript:replyMsg(\''.$thread['OWNER'].'\')"><img src="'.$img.'" border="0" title="'.$c.'，在线，有人找我吗？"/></a>';
		} else {
			echo '<img src="'.$img.'" border="0" title="'.$c.'，'.($is_online?'在线':'离线').'"/>';
		}
	} else {
		echo '<img src="pic/offline1.gif" border="0" title="未知用户"/>';
	}
?>
</td>
<td width="10" valign="middle"></td></tr></table>
<?php
	if ($user !== false) {
?>
&nbsp;&nbsp;<?php echo get_myface($user); ?><br/>
&nbsp;&nbsp;等级：<?php echo bbs_getuserlevel($thread['OWNER']); ?><br/>
&nbsp;&nbsp;文章：<?php echo $user['numposts']; ?><br/>
<?php
		if (SHOW_REGISTER_TIME) {
?>
&nbsp;&nbsp;注册：<?php echo strftime('%Y-%m-%d',$user['firstlogin']); ?><br/>
<?php
		}
		if ($show_detail) {
?>
&nbsp;&nbsp;星座：<?php echo get_astro($user['birthmonth'],$user['birthday']); ?>
<?php
		}
	} else {
?>
&nbsp;&nbsp;<img src="userface/image.gif" align="absmiddle" title="未知用户"/>
<?php
	}
?>
</td>

<td class="<?php echo $bgstyle ;?>" valign="top" width="*">

<table width="100%" ><tr><td width="*">
<a href="queryresult.php?userid=<?php echo $thread['OWNER']; ?>&amp;boardNames=<?php echo $boardName; ?>"><img src="pic/find.gif" border="0" title="搜索<?php echo $thread['OWNER']; ?>在本版的所有贴子"/></a>&nbsp;
<a href="sendmail.php?board=<?php echo $boardName; ?>&amp;reID=<?php echo $thread['ID']; ?>"><img title="点击这里发送信件给<?php echo $thread['OWNER']; ?>" border="0" src="pic/email.gif"/></a>&nbsp;
<a href="editarticle.php?board=<?php echo $boardName; ?>&amp;reID=<?php echo $thread['ID']; ?>"><img src="pic/edit.gif" border="0" title="编辑"/></a>&nbsp;
<a href="deletearticle.php?board=<?php echo $boardName; ?>&amp;ID=<?php echo $thread['ID']; ?>" onclick="return confirm('你真的要删除本文吗?')"><img src="pic/delete.gif" border="0" title="删除"/></a>&nbsp;
<?php
	if (!OLD_REPLY_STYLE && ENABLE_UBB) {
?>
<a href="postarticle.php?board=<?php echo $boardName; ?>&amp;reID=<?php echo $thread['ID']; ?>&amp;quote=1"><img src="pic/reply.gif" border="0" title="引用回复这个贴子"/></a>&nbsp;
<?php
	}
?>
<a href="postarticle.php?board=<?php echo $boardName; ?>&amp;reID=<?php echo $thread['ID']; ?>"><img src="pic/reply_a.gif" border="0" title="回复这个贴子"/></a>
</td>
<td width="50">
<b><?php echo $num==0?'楼主':'第<font color="#ff0000">'.$num.'</font>楼'; ?></b></td></tr><tr><td bgcolor="#D8C0B1" height="1" colspan="2"></td></tr>
</table>

<table border="0" width="98%" style=" table-layout:fixed;word-break:break-all"><tr><td width="100%" style="font-size:9pt;line-height:12pt;padding: 0px 5px;"><?php echo $articleContents; ?></td></tr></table>
</td>

</tr>
<?php
/*
<tr>
<td class=<?php echo $bgstyle ;?> valign=middle align=center width=175><a href=# onclick="alert('本功能尚未实现');" target=_blank><img align=absmiddle border=0 width=13 height=15 src="pic/ip.gif" title="点击查看用户来源及管理<br>发贴IP：*.*.*.*"></a> <?php echo strftime("%Y-%m-%d %H:%M:%S",$thread['POSTTIME']); ?></td>
<td class=<?php echo $bgstyle ;?> valign=middle width=*>
<table width=100% cellpadding=0 cellspacing=0><tr>
  <td align=left valign=middle> &nbsp;&nbsp;<a href=# onclick="alert('本功能尚未实现');" title="同意该帖观点，给他一朵鲜花，将消耗您5点金钱"><img src=pic/xianhua.gif border=0>鲜花</a>(<font color=#FF0000>0</font>)&nbsp;&nbsp;<a href=# onclick="alert('本功能尚未实现');" title="不同意该帖观点，给他一个鸡蛋，将消耗您5点金钱"><img src=pic/jidan.gif border=0>鸡蛋</a>(<font color=#FF0000>0</font>)</td><td align=right nowarp valign=bottom width=200></td>
  <td align=right valign=bottom width=4><a href="bmmanage.php?board=<?php echo $boardName; ?>&ID=<?php echo $thread['ID']; ?>"><img src="pic/jing.gif" border=0 title=切换精华</a></td>
</tr></table>
</td></tr>
<?php
	*/
}

function showTreeItem($boardName,$groupID,$article,$startNum,$level, $lastflag){
	global $start; //不好意思，搞个全局变量 - atppp
	echo '<tr><td class="TableBody'.($start==$startNum?1:2).'" width="100%" height="22" colspan="2">';
	for ($i=0;$i<$level;$i++) {
		if ($lastflag[$i]) {
			if ($i == $level - 1) echo '<img src="pic/treenode2.gif"/>'; // |-
			else echo '<img src="pic/treenode.gif"/>';                   // |
		} else {
			if ($i == $level - 1) echo '<img src="pic/treenode1.gif"/>'; // \
			else echo "&nbsp;&nbsp;";                               // nothing
		}
	}
	if ($article == null) {
		echo ' ... <a href="disparticle.php?boardName='.$boardName.'&amp;ID='.$groupID.'&amp;start='.$startNum.'&amp;listType=1"><span style="color:red">还有更多</span></a> ...';
	} else {
		echo '<img src="face/face1.gif" height="16" width="16"/>  <a href="disparticle.php?boardName='.$boardName.'&amp;ID='.$groupID.'&amp;start='.$startNum.'&amp;listType=1">';
		if ($start==$startNum) echo "<font color=\"red\">";
		echo htmlspecialchars($article['TITLE'],ENT_QUOTES);
		if ($start==$startNum) echo "</font>";
		echo ' </a><i><font color="gray">(';
		if ($article["EFFSIZE"] < 1000) echo $article["EFFSIZE"];
		else {
			printf("%.1f",$article["EFFSIZE"]/1000.0); echo "k";
		}
		echo '字)</font> － <a href="dispuser.php?id='.$article['OWNER'].'" target="_blank" title="作者资料"><font color="gray">'.$article['OWNER'].'</font></a>，'.strftime("%Y年%m月%d日 %T",$article['POSTTIME']);
		echo '</i>';
	}
	echo '</td></tr>';	
}
?>
