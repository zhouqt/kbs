<?php
function showUserMailbox(){
	global $currentuser;
?>
<tr><td width=65% >
</td><td width=35% align=right>
<?php   
	bbs_getmailnum($currentuser["userid"],$total,$unread);
	if ($unread>0)  {
?>
<bgsound src="sound/msg.wav" border=0>
<img src="pic/msg_new_bar.gif" /> <a href="usermailbox.php?boxname=inbox">我的收件箱</a> (<a href="usermail.php?boxname=inbox&num=<?php echo $total-1;?>" target=_blank><font color="#FF0000"><?php  echo $unread; ?> 新</font></a>)
<?php   }
    else
  {
?>
<img src="pic/msg_no_new_bar.gif" /> <a href="usermailbox.php?boxname=inbox">我的收件箱</a> (<font color=gray>0 新</font>)
<?php
  }
?>
</td></tr>
<?php
}

function showAllSecs(){
	GLOBAL $sectionCount;
	GLOBAL $_COOKIE;
	GLOBAL $_GET;

	for ($i=0;$i<$sectionCount;$i++){
		if ($_COOKIE['ShowSecBoards'.$i]=='Y') {
			showSecs($i,true);
		} else {
			showSecs($i,false);
		}
	}
	return false;
}

function showSecs($secNum=0,$isFold) {
	extract($GLOBALS);
	if ( ($secNum<0)  || ($secNum>=$sectionCount)) {
		foundErr("版面参数错误！");
		return false;
	}
?>
<table cellspacing=1 cellpadding=0 align=center width="97%" class=tableBorder1>
<TR><Th colSpan=2 height=25 align=left id=TableTitleLink>&nbsp;
<?php
	if ($isFold) {
?>
<a href="<?php echo $_SERVER['PHP_SELF'] ; ?>?sec=<?php echo $secNum; ?>&ShowBoards=N" title="关闭论坛列表"><img src="pic/nofollow.gif" border=0></a><a href="section.php?sec=<?php echo $secNum ; ?>" title=进入本分类论坛><?php echo $section_names[$secNum][0]; ?></a>
</th></tr>
<?php
	} else {
?>
<a href="<?php echo $_SERVER['PHP_SELF'] ; ?>?sec=<?php echo $secNum; ?>&ShowBoards=Y" title="展开论坛列表"><img src="pic/plus.gif" border=0></a><a href="section.php?sec=<?php echo $secNum ; ?>" title=进入本分类论坛><?php echo $section_names[$secNum][0]; ?></a>
<?php
	}
	$boards = bbs_getboards($section_nums[$secNum], 0, bbs_is_yank()?0:1);
	if ($boards == FALSE) {
?>
		<TR><TD colspan="2" class=tablebody1>&nbsp;本分区尚无版面</td></tr>
<?php
	} else {
		$brd_name = $boards["NAME"]; // 英文名
		$brd_desc = $boards["DESC"]; // 中文描述
		$brd_class = $boards["CLASS"]; // 版分类名
		$brd_bm = $boards["BM"]; // 版主
		$brd_artcnt = $boards["ARTCNT"]; // 文章数
		$brd_unread = $boards["UNREAD"]; // 未读标记
		$brd_zapped = $boards["ZAPPED"]; // 是否被 z 掉
		$brd_flag = $boards["FLAG"]; //flag
		$brd_bid = $boards["BID"]; //flag
		$rows = sizeof($brd_name);
		$isFirst=false;
		for ($i = 0; $i < $rows; $i++)	{
			$isFirst=!$isFirst;
			if ($isFold){
?>
			<TR><TD align=middle width="100%" class=tablebody1>
		<table width="100%" cellspacing=0 cellpadding=0><TR><TD align=middle width=46 class=tablebody1>
<?php	
				if ( $brd_unread[$i] == 1) {
					echo "<img src=pic/forum_isnews.gif alt=有新帖子>";
				} else  {
					echo "<img src=pic/forum_nonews.gif alt=无新帖子>";
				}
?>
		</TD>
		<TD width=1 bgcolor=#7a437a>
		<TD vAlign=top width=* class=tablebody1>
		
		<TABLE cellSpacing=0 cellPadding=2 width=100% border=0>
		<tr><td class=tablebody1 width=*><a href="board.php?name=<?php echo $brd_name[$i]; ?> ">
		<font color=#000066><?php echo $brd_name[$i] ?> </font></a>
				</td>
		<td width=40 rowspan=2 align=center class=tablebody1></td><td width=200 rowspan=2 class=tablebody1><?php
				if ($brd_flag[$i] & BBS_BOARD_GROUP) {
		?>
				<B>本版为二级目录版</B>
		<?php
				} else {
					if ($brd_artcnt[$i] <= 0) {
		?>
				<B>本版尚无文章</B>
		<?php
					} else {
						$articles = bbs_getthreads($brd_name[$i], 0, 1,0 ); //$brd_artcnt[$i], 1, $default_dir_mode);
						if ($articles == FALSE) {
		?>
				<B>本版尚无文章</B>
		<?php
						} else {
						//	$threads = bbs_getthreads($brd_name[$i],0,1); 
						//	$start = bbs_get_thread_article_num( $brd_name[$i], $articles[0]['GROUPID']); 
		?>
				主题：<a href="disparticle.php?boardName=<?php echo $brd_name[$i]; ?>&ID=0"><?php echo htmlspecialchars($articles[0]['TITLE'],ENT_QUOTES); ?></a><BR>作者：<a href="userinfo.php?id=<?php echo $articles[0]['OWNER']; ?>" target=_blank><?php echo $articles[0]['OWNER']; ?></a><BR>日期：<?php echo strftime('%Y-%m-%d %H:%M:%S', intval($articles[0]['POSTTIME'])) ; ?>&nbsp;<a href="disparticle.php?boardName=<?php echo $brd_name[$i]; ?>&ID=0&start=<?php echo $start?>"><IMG border=0 src="pic/lastpost.gif" title="转到：<?php echo $articles[0]['TITLE']; ?>"></a>
	<?php
						}
					}
				}
	?>
</TD></TR><TR><TD width=*><FONT face=Arial><img src=pic/forum_readme.gif align=middle> <?php echo $brd_desc[$i] ?></FONT>
</TD></TR><TR><TD class=tablebody2 height=20 width=*>版主：<?php echo $brd_bm[$i]==''?'暂无':$brd_bm[$i] ; ?> </TD><td width=40 align=center class=tablebody2>&nbsp;</td><TD vAlign=middle class=tablebody2 width=200>
		<table width=100% border=0><tr><td width=25% vAlign=middle><img src=pic/forum_today.gif alt=今日帖 align=absmiddle>&nbsp;<font color=#FF0000><?php echo bbs_get_today_article_num($brd_name[$i]) ?></font></td><td width=30% vAlign=middle><img src=pic/forum_topic.gif alt=主题 border=0  align=absmiddle>&nbsp;<?php echo bbs_getthreadnum($brd_bid[$i]) ?></td><td width=45% vAlign=middle><img src=pic/forum_post.gif alt=文章 border=0 align=absmiddle>&nbsp;<?php echo $brd_artcnt[$i]; ?></td></tr>
		</table></TD></TR></TBODY></TABLE></td></tr></table></td></tr>
<?php
			} else {
				if ($isFirst) {
					echo "<tr>";
				}
?>
<td class=tablebody1 width="50%"><TABLE cellSpacing=2 cellPadding=2 width=100% border=0><tr><td width="100%" title="<?php echo $brd_desc[$i] ; ?>" colspan=2><a href="board.php?name=<?php echo $brd_name[$i]; ?>"><font color=#000066><?php echo $brd_name[$i] ; ?></font></a></td></tr><tr>
<?php
				if ($brd_flag[$i] & BBS_BOARD_GROUP) {
?>
<td> <b>本版为二级目录版</b></td>
<?php
				} else {
?>
<td width="50%">今日：<font color=#FF0000><?php echo bbs_get_today_article_num($brd_name[$i])?></font></td><td width="50%">发贴：<?php echo $brd_artcnt[$i] ; ?></td>
<?php
				}
?>
</tr></table></td>
<?php
				if (!$isFirst) {
					echo "</tr>";
				}
			}
		}
		if ($isFirst) {
?>
<td class=tablebody1 width="50%"></td></tr>
<?php
		}
	}

?>
</table><br>
<?php
}

function showAnnounce(){
	global $AnnounceBoard;
	global $SiteName;
?>
<tr>
<td align=center width=100% valign=middle colspan=2>
<SCRIPT LANGUAGE='JavaScript' SRC='inc/fader.js' TYPE='text/javascript'></script>
<SCRIPT LANGUAGE='JavaScript' TYPE='text/javascript'>
prefix="";
arNews = [<?php 
		$brdarr = array();
		$brdnum = bbs_getboard($AnnounceBoard, $brdarr);
		if ( ($brdnum==0) || ($brdarr["FLAG"] & BBS_BOARD_GROUP) ) {
			echo '"当前没有公告","",';
		} else {
			$total = bbs_getThreadNum($brdnum);
			if ($total <= 0) {
				echo '"当前没有公告","",';
			} else {
				$articles = bbs_getthreads($brdarr['NAME'], 0, ANNOUNCENUMBER,1); 
				if ($articles == FALSE) {
					echo '"当前没有公告2","",';
				} else {
					$num=count($articles);
					for ($i=0;$i<$num;$i++) {
					echo '"<b><a href=\"disparticle.php?boardName='.$brdarr['NAME'].'&ID='.($i).'\">' .htmlspecialchars($articles[$i]['TITLE'],ENT_QUOTES) . '</a></b> ('.strftime('%Y-%m-%d %H:%M:%S', intval($articles[$i]['POSTTIME'])).')","",';
					}
				}
			}
		}
?>"欢迎光临<?php echo $SiteName; ?>",""
];
</SCRIPT>
<div id="elFader" style="position:relative;visibility:hidden; height:16" ></div>
</td>
</tr>
<?php
}

function FastLogin()
{
extract($GLOBALS);
?>
<table cellspacing=1 cellpadding=3 align=center class=tableBorder1>
<form action="logon.php" method=post>
<input type="hidden" name="action" value="doLogon">
<tr>
<th align=left id=tabletitlelink height=25 style="font-weight:normal">
<b>-=> 快速登录入口</b>
[<a href=register.php>注册用户</a>]　[<a href=lostpass.php style="CURSOR: help">忘记密码</a>]
</th>
</tr>
<tr>
<td class=tablebody1 height=40 width="100%">
&nbsp;用户名：<input maxLength=16 name=id size=12>　　密码：<input maxLength=20 name=password size=12 type=password>　　<select name=CookieDate><option selected value=0>不保存</option><option value=1>保存一天</option><option value=2>保存一月</option><option value=3>保存一年</option></select><input type=hidden name=comeurl value="<?php echo $_SERVER['PHP_SELF']; ?>"><input type=submit name=submit value="登 陆">
</td>
</tr>
</form>
</table><br>
<?php 
} 

function usersysinfo($info){
   if (strpos($info,';')!==false)  {
	  $usersys=explode(';',$info);
	  if (count($usersys)>=2)  {
		  $usersys[1]=str_replace("MSIE","Internet Explorer",$usersys[1]);
		  $usersys[2]=str_replace(")","",$usersys[2]);
		  $usersys[2]=str_replace("NT 5.1","XP",$usersys[2]);
		  $usersys[2]=str_replace("NT 5.0","2000",$usersys[2]);
		  $usersys[2]=str_replace("9x","Me",$usersys[2]);
		  $usersys[1]="浏 览 器：".trim($usersys[1]);
		  $usersys[2]="操作系统：".trim($usersys[2]);
		  $function_ret=$usersys[1].'，'.$usersys[2];
	  }  else  {
		  	  $function_ret='浏 览 器：未知，操作系统：未知';
	  }
  }  else {
    if ($getinfo==1)  {
      $function_ret="未知，未知";
    } 
  } 
  return $function_ret;
} 

function showUserInfo(){
?>
<table cellpadding=5 cellspacing=1 class=tableborder1 align=center style="word-break:break-all;">
<TR><Th align=left colSpan=2 height=25>-=> 用户来访信息</Th></TR>
<TR><TD vAlign=top class=tablebody1 height=25 width=100% >
<?php
$userip = $_SERVER["HTTP_X_FORWARDED_FOR"];
$userip2 = $_SERVER["REMOTE_ADDR"];
if  ($userip=='') {
	echo '您的真实ＩＰ 是：'. $userip2. '，';
} else {
	echo '您的真实ＩＰ 是：'. $userip .'，';
}
echo usersysinfo($_SERVER["HTTP_USER_AGENT"]);

?>
</TD></TR></table><br>
<?php

}

function showOnlineUsers(){
?>
<table cellpadding=5 cellspacing=1 class=tableborder1 align=center style="word-break:break-all;">
<TR><Th colSpan=2 align=left id=tabletitlelink height=25 style="font-weight:normal"><b>-=> 论坛在线统计</b>&nbsp;[<a href=showOnlineUsers.php?action=show>显示详细列表</a>] [<a href=boardstat.php?reaction=online>查看在线用户位置</a>]</Th></TR>
<TR><TD width=100% vAlign=top class=tablebody1>  目前论坛上总共有 <b><?php echo bbs_getonlinenumber() ; ?></b> 人在线，其中注册用户 <b><?php echo bbs_getonlineusernumber(); ?></b> 人，访客 <b><?php echo bbs_getwwwguestnumber() ; ?></b> 人。<br>
历史最高在线纪录是 <b><?php echo  $Maxonline ?></b> 人同时在线
</td></tr>
</table><br>
<?php
}

function showSample(){
?>
<table cellspacing=1 cellpadding=3 width="97%" border=0 align=center>
<tr><td align=center><img src="pic/forum_nonews.gif" align="absmiddle">&nbsp;没有新的帖子&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<img src="pic/forum_isnews.gif" align="absmiddle">&nbsp;有新的帖子&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<img src="pic/forum_lock.gif" align="absmiddle">&nbsp;被锁定的论坛</td></tr>
</table><br>
<?php
}

function showMailSampleIcon(){
?>
<table cellspacing=1 cellpadding=3 width="97%" border=0 align=center>
<tr><td align=center><img src="pic/m_news.gif" align="absmiddle">&nbsp;未读邮件&nbsp<img src="pic/m_olds.gif" align="absmiddle">&nbsp;已读邮件&nbsp<img src="pic/m_replys.gif" align="absmiddle">&nbsp;已回复邮件&nbsp;<img src="pic/m_newlocks.gif" align="absmiddle">&nbsp;锁定的未读邮件&nbsp;<img src="pic/m_oldlocks.gif" align="absmiddle">&nbsp;锁定的已读邮件&nbsp;<img src="pic/m_lockreplys.gif" align="absmiddle">&nbsp;锁定的已回复邮件</td></tr>
</table><br>
<?php
}


?>
