<?php

function showUserMailBoxOrBR() { //用于普通用户就可以访问的页面
	global $loginok;
	if ($loginok==1) {
		showUserMailBox();
	} else {
		echo "<br/>";
	}
}

function showUserMailbox(){ //这个函数直接调用必须保证 $loginok==1
	global $currentuser;
?>
<table cellSpacing="0" cellPadding="0" width="97%" border="0" align="center">
<tr><td width="65%" >
</td><td width="35%" align="right">
<?php   
	bbs_getmailnum($currentuser["userid"],$total,$unread,0,0);
	if ($unread>0)  {
?>
<bgsound src="sound/newmail.wav" border="0"/>
<img src="pic/msg_new_bar.gif" /> <a href="usermailbox.php?boxname=inbox">我的收件箱</a> (<a href="usermail.php?boxname=inbox&amp;num=<?php echo $total-1;?>" target="_blank"><font color="#FF0000"><?php  echo $unread; ?> 新</font></a>)
<?php   }
    else
  {
?>
<img src="pic/msg_no_new_bar.gif" /> <a href="usermailbox.php?boxname=inbox">我的收件箱</a> (<font color="gray">0 新</font>)
<?php
  }
?>
</td></tr>
</table>
<?php
}

function getSecFoldCookie($secNum, $isShow = true) {
	if ($isShow) $cn = "ShowSecBoards";
	else $cn = "HideSecBoards";
	if (isset($_COOKIE[$cn])) {
		$ssb = $_COOKIE[$cn];
		settype($ssb, "integer");
	} else {
		if (SECTION_DEF_CLOSE && !$isShow) $ssb = ~0;
		else $ssb = 0;
	}
	return (($ssb & (1 << ($secNum+1))) != 0);
}

function showSecsJS($secNum,$group,$isFold,$isFav,$isHide) {
	global $yank;
	global $section_nums;
?>
<script language="JavaScript">
<!--
	boards = new Array();
<?php
	if ($isFav) {
		$select = $secNum; //代码相似性 :D
		echo "j_select = $select;\n";
	}

	if (!$isHide) {
		if (!$isFav) {
			if (!$isFold) $flag = $yank | 2;
			else $flag = $yank;
			$boards = bbs_getboards($section_nums[$secNum], $group, $flag);
		} else {
			$boards = bbs_fav_boards($select, 1);
			if ($boards == FALSE) {
	    		//foundErr("读取版列表失败");
			}
		}
		if ($boards !== FALSE) {
			$brd_name = $boards["NAME"]; // 英文名
			$brd_desc = $boards["DESC"]; // 中文描述
			$brd_class = $boards["CLASS"]; // 版分类名
			$brd_bm = $boards["BM"]; // 版主
			$brd_artcnt = $boards["ARTCNT"]; // 文章数
			$brd_unread = $boards["UNREAD"]; // 未读标记
			$brd_zapped = $boards["ZAPPED"]; // 是否被 z 掉
			if ($isFav) {
				$brd_position= $boards["POSITION"];//位置
				$brd_npos= $boards["NPOS"];//位置
			}
			$brd_flag = $boards["FLAG"]; //flag
			$brd_bid = $boards["BID"]; //flag
			$brd_currentusers = $boards["CURRENTUSERS"];
			$rows = sizeof($brd_name);
			$showed = 0;
			for ($i = 0; $i < $rows; $i++)	{
				if ($brd_name[$i]=='Registry')
					continue;
				if ($brd_name[$i]=='bbsnet')
					continue;
				if ($brd_name[$i]=='undenypost')
					continue;
				
				if ($isFav && ($brd_bid[$i] == -1)) {
					continue;
				}				

				$isFavGroup = ($isFav && ($brd_flag[$i] == -1));
				$isBoardGroup = (($brd_flag[$i] & BBS_BOARD_GROUP) && !$isFavGroup);
				
				$j_isBoardGroup = ($isBoardGroup ? 1 : 0);
				$j_isFavGroup = ($isFavGroup ? 1 : 0);
				$j_boardDesc = "'" . addslashes(htmlspecialchars($brd_desc[$i], ENT_QUOTES)) . "'";
				if ($isFavGroup) {
					$j_boardName = $j_boardDesc;
				} else {
					$j_boardName = "'" . $brd_name[$i] . "'";
				}

				if ($isFavGroup | $isBoardGroup) {
					$j_todayNum = $j_nArticles = 0;
				} else {
					$j_todayNum = bbs_get_today_article_num($brd_name[$i]);
					$j_nArticles = $brd_artcnt[$i];
				}
				if ($isFav) {
					$j_npos = $brd_npos[$i];
					$j_bid = $brd_bid[$i];
				} else {
					$j_npos = $j_bid = 0;
				}
				$j_currentusers = intval($brd_currentusers[$i]);
				if ($isFold) {
					$j_isUnread = ($brd_unread[$i] == 1 ? 1 : 0);
					if ($j_nArticles > 0) {
						$j_nThreads = bbs_getthreadnum($brd_bid[$i]);
						$articles = bbs_getthreads($brd_name[$i], 0, 1, 0 ); //$brd_artcnt[$i], 1, $default_dir_mode);
						if ($articles == FALSE) {
							$j_nArticles = $j_lastID = $j_lastTitle = $j_lastOwner = $j_lastPosttime = 0;
						} else {
							$j_lastID = $articles[0]['origin']['ID'];
							$j_lastTitle = "'" . addslashes(htmlspecialchars($articles[0]['origin']['TITLE'],ENT_QUOTES)) . " '";
							$j_lastOwner = "'" . $articles[0]['lastreply']['OWNER'] . "'";
							$j_lastPosttime = "'" . strftime('%Y-%m-%d %H:%M:%S', intval($articles[0]['lastreply']['POSTTIME'])) . "'";
						}
					} else {
						$j_nThreads = $j_nArticles = $j_lastID = $j_lastTitle = $j_lastOwner = $j_lastPosttime = 0;
					}
					$j_bm = "'".$brd_bm[$i]."'";
					echo "boards[boards.length] = new Board($j_isBoardGroup,$j_isFavGroup,$j_isUnread,$j_boardName,$j_boardDesc,$j_lastID,$j_lastTitle,$j_lastOwner,$j_lastPosttime,$j_bm,$j_todayNum,$j_nArticles,$j_nThreads,$j_npos,$j_bid,$j_currentusers);\n";
				} else {
					echo "boards[boards.length] = BoardS($j_isBoardGroup,$j_isFavGroup,$j_boardName,$j_boardDesc,$j_todayNum,$j_nArticles,$j_npos,$j_bid,$j_currentusers);\n";
				}
			}
		}
	}
?>
	boards<?php echo $secNum; ?> = boards;
	curfold<?php echo $secNum; ?> = foldflag<?php echo $secNum; ?> = <?php echo ($isHide ? 0 : ($isFold ? 2 : 1)); ?>;
//-->
</script>
<?php
}


/*
 * $isFav = true 的时候表示载入收藏夹，参数说明：$secNum 相当于 bbs2www/html/bbsfav.php 里头的 $select, $group 参数此时没有作用 - atppp
 * isFold: true when show detailed boards list.
 */
function showSecs($secNum,$group,$isFold,$isFav = false,$isHide = false) {
	global $section_names;
?>
<table cellspacing=0 cellpadding=0 align=center width="97%" class=TableBorder1>
<TR><Th height=25 align=left id=TableTitleLink>&nbsp;
<?php
	if (!$isFav) {
		if ($group == 0) {
			if ($isFold) {
?>
<img src="pic/nofollow.gif" id="followImg<?php echo $secNum; ?>" style="cursor:hand;" onclick="loadSecFollow(<?php echo $secNum; ?>)" border=0 title="折叠版面列表">
<?php
			} else {
?>
<img src="pic/plus.gif" id="followImg<?php echo $secNum; ?>" style="cursor:hand;" onclick="loadSecFollow(<?php echo $secNum; ?>)" border=0 title="展开版面列表">
<?php
			}
		} else {
?>
<img src="pic/nofollow.gif" border=0>
<?php
		}
?>
<a href="section.php?sec=<?php echo $secNum ; ?>" title=进入本分类讨论区><?php echo $section_names[$secNum][0]; ?></a>
<?php
		if ($group == 0) {
?>
[<a id="toogleHide<?php echo $secNum ; ?>" href="javascript:toogleHide(<?php echo $secNum; ?>)"><?php echo $isHide ? "展开" : "关闭"; ?>版面列表</a>]
<?php
		}
	} else {
		$select = $secNum; //代码相似性 :D
		if ($isFold) {
?>
<img src="pic/nofollow.gif" id="followImg<?php echo $select; ?>" style="cursor:hand;" onclick="loadFavFollow()" border=0 title="折叠版面列表">
<?php
		} else {
?>
<img src="pic/plus.gif" id="followImg<?php echo $select; ?>" style="cursor:hand;" onclick="loadFavFollow()" border=0 title="展开版面列表">
<?php
		}
?>
用户收藏夹
<?php
		if ($select != 0) {
			$list_father = bbs_get_father($select);
?>
&nbsp;[<a href="favboard.php?select=<?php echo $list_father; ?>">回到上一级</a>]
<?php
		}
?>
&nbsp;[<a href="modifyfavboards.php?select=<?php echo $select; ?>">管理本收藏夹目录</a>]
<?php
	}
?>
</th></tr>
<?php
	showSecsJS($secNum,$group,$isFold,$isFav,$isHide);
?>
<tr style="display:none" id="followTip<?php echo $secNum; ?>"><td style="text-align:center;border:1px solid black;background-color:lightyellow;color:black;padding:2px" onclick="loadSecFollow('<?php echo $secNum; ?>')">正在读取版面列表数据，请稍侯……</div></td></tr>
<TR><Td id="followSpan<?php echo $secNum; ?>">
<script language="JavaScript">
<!--
	str = showSec(<?php echo ($isFold ? 1 : 0); ?>, <?php echo ($isFav ? 1 : 0); ?>, boards, <?php echo $secNum ?>, <?php echo ($isHide ? 1 : 0); ?>);
	document.write(str);
//-->
</script>
</td></tr>
</table><br>
<?php
}

function showAnnounce(){
	global $AnnounceBoard;
	global $SiteName;
?>
<tr>
<td align=center width=100% valign=middle colspan=2>
<link rel="stylesheet" type="text/css" href="css/fader.css">
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
					echo '"当前没有公告","",';
				} else {
					$num=count($articles);
					for ($i=0;$i<$num;$i++) {
					echo "'<b><a href=\"disparticle.php?boardName=".$brdarr['NAME']."&ID=".$articles[$i]['origin']['ID']."\">" .htmlspecialchars($articles[$i]['origin']['TITLE'],ENT_QUOTES) . "</a></b> (".strftime('%Y-%m-%d %H:%M:%S', intval($articles[$i]['origin']['POSTTIME'])).")',\"\",";
					}
				}
			}
		}
?>"<b>欢迎光临<?php echo $SiteName; ?></b>",""
];
</SCRIPT>
<div id="elFader" style="position:relative; visibility:visible; height:20px; padding:4px;" ></div>
</td>
</tr>
<?php
}

function usersysinfo($info){
	if (USEBROWSCAP == 0) { //FireFox, Opera 都判断不对 - atppp
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
		} else {
			$function_ret="未知，未知";
		}
	} else {
		$browser = get_browser($info);
		$str1 = $browser->parent;
		$str1 = str_replace("IE","Internet Explorer",$str1);
		if ($str1 == "") $str1 = "未知";
		$str2 = $browser->platform;
		$str2 = str_replace("Win","Windows ",$str2);
		if ($str2 == "") $str2 = "未知";
		$function_ret = "浏 览 器：".$str1."，操作系统：".$str2;
	}
	return $function_ret;
} 

function showUserInfo(){
	global $fromhost;
	return; //disabled  - atppp 20041204
?>
<table cellpadding=5 cellspacing=1 class=TableBorder1 align=center style="word-break:break-all;" width="97%">
<TR><Th align=left colSpan=2 height=25>-=&gt; 用户来访信息</Th></TR>
<TR><TD vAlign=top class=TableBody1 height=25 width=100% >
<?php
echo '您的真实ＩＰ是：'. trim($fromhost). '，';
echo usersysinfo($_SERVER["HTTP_USER_AGENT"]);

?>
</TD></TR></table><br>
<?php

}

function showOnlineUsers(){
?>
<table cellpadding=5 cellspacing=1 class=TableBorder1 align=center style="word-break:break-all;" width="97%">
<TR><Th colSpan=2 align=left id=TableTitleLink height=25 style="font-weight:normal">
	<b>-=&gt; 论坛在线统计</b>&nbsp;[<a href=showonlineuser.php>显示详细列表</a>]
	<!--[<a href=boardstat.php?reaction=online>查看在线用户位置</a>]-->
</Th></TR>
<TR><TD width=100% vAlign=top class=TableBody1>  目前论坛上总共有 <b><?php echo bbs_getonlinenumber() ; ?></b> 人在线，其中注册用户 <b><?php echo bbs_getonlineusernumber(); ?></b> 人，访客 <b><?php echo bbs_getwwwguestnumber() ; ?></b> 人。<!--<br>
历史最高在线纪录是 <b> </b> 人同时在线-->
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
