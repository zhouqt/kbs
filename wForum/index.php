<?php


if ( (!isset($_COOKIE['iscookies'])) || ($_COOKIE['iscookies']==''))
{
	setcookie('iscookies','0',time()+3650*24*3600);
	print '<META http-equiv=Content-Type content=text/html; charset=gb2312><meta HTTP-EQUIV=REFRESH CONTENT=3>正在登陆论坛……<br><br>本系统要求使用COOKIES，假如您的浏览器禁用COOKIES，您将不能登录本系统……';
	exit();
} 

require("inc/funcs.php");

preprocess();

show_nav();

?>
<br>
<TABLE cellSpacing=0 cellPadding=0 width=97% border=0 align=center>
<?php

showAnnounce();

showTitle();

?>
</TABLE>
<?php

if ($loginok!=1) {
	FastLogin();
}

showAllSecs();
if (isErrFounded()) {
	html_error_quit();
} else {
	showUserInfo();
	showOnlineUsers();
	showSample();
}
show_footer();

function preprocess(){
	GLOBAL $_GET;
	GLOBAL $_COOKIE;
	GLOBAL $sectionCount;

	$path='';
	if ($_GET['ShowBoards']=='Y') {
		$secNum=intval($_GET['sec']);
		if ( ($secNum>=0)  && ($secNum<$sectionCount)) {
			setcookie('ShowSecBoards'.$secNum, 'Y' ,time()+604800,''); 
			$_COOKIE['ShowSecBoards'.$secNum]='Y';
		}
	}
	if ($_GET['ShowBoards']=='N') {
		$secNum=intval($_GET['sec']);
		if ( ($secNum>=0)  && ($secNum<$sectionCount)) {
			setcookie('ShowSecBoards'.$secNum, '' ,time()+604800);
			$_COOKIE['ShowSecBoards'.$secNum]='';
		}
	}
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
<a href="<?php echo $_SERVER['PHP_SELF'] ; ?>?sec=<?php echo $secNum; ?>&ShowBoards=Y" title="展开论坛列表"><img src="pic/plus.gif" border=0></a><a href="viewsec.php?sec=<?php echo $secNum ; ?>" title=进入本分类论坛><?php echo $section_names[$secNum][0]; ?></a>
<?php
	}
	$boards = bbs_getboards($section_nums[$secNum], 0, 0);
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
		<tr><td class=tablebody1 width=*><a href="board.php?boardid=<?php echo $brd_bid[$i]; ?> ">
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
						$articles = bbs_getarticles($brd_name[$i], $brd_artcnt[$i], 1, $default_dir_mode);
						if ($articles == FALSE) {
		?>
				<B>本版尚无文章</B>
		<?php
						} else {
		?>
				主题：<a href="disparticle.php?boardid=<?php echo $brd_bid[$i]; ?>&id=<?php echo $articles[0]['GROUPID']; ?>&gid=<?php $articles[0]['ID']; ?>"><?php echo $articles[0]['TITLE']; ?></a><BR>作者：<a href="userinfo.php?id=<?php echo $articles[0]['OWNER']; ?>" target=_blank><?php echo $articles[0]['OWNER']; ?></a><BR>日期：<?php echo strftime('%Y-%m-%d %H-%M-%S', intval($articles[0]['POSTTIME'])) ; ?>&nbsp;<a href="disparticle.php?boardid=<?php echo $brd_bid[$i]; ?>&id=<?php echo $articles[0]['GROUPID']; ?>&gid=<?php $articles[0]['ID']; ?>"><IMG border=0 src="pic/lastpost.gif" title="转到：<?php echo $articles[0]['TITLE']; ?>"></a>
	<?php
						}
					}
				}
	?>
</TD></TR><TR><TD width=*><FONT face=Arial><img src=pic/forum_readme.gif align=middle> <?php echo $brd_desc[$i] ?></FONT>
</TD></TR><TR><TD class=tablebody2 height=20 width=*>版主：<?php echo $brd_bm[$i]==''?'暂无':$brd_bm[$i] ; ?> </TD><td width=40 align=center class=tablebody2>&nbsp;</td><TD vAlign=middle class=tablebody2 width=200>
		<table width=100% border=0><tr><td width=25% vAlign=middle><img src=pic/forum_today.gif alt=今日帖 align=absmiddle>&nbsp;<font color=#FF0000>N/A</font></td><td width=30% vAlign=middle><img src=pic/forum_topic.gif alt=主题 border=0  align=absmiddle>&nbsp;N/A</td><td width=45% vAlign=middle><img src=pic/forum_post.gif alt=文章 border=0 align=absmiddle>&nbsp;<?php echo $brd_artcnt[$i]; ?></td></tr>
		</table></TD></TR></TBODY></TABLE></td></tr></table></td></tr>
<?php
			} else {
				if ($isFirst) {
					echo "<tr>";
				}
?>
<td class=tablebody1 width="50%"><TABLE cellSpacing=2 cellPadding=2 width=100% border=0><tr><td width="100%" title="<?php echo $brd_desc[$i] ; ?>" colspan=2><a href="board.php?boardid=<?php echo $brd_bid[$i]; ?>"><font color=#000066><?php echo $brd_name[$i] ; ?></font></a></td></tr><tr>
<?php
				if ($brd_flag[$i] & BBS_BOARD_GROUP) {
?>
<td> <b>本版为二级目录版</b></td>
<?php
				} else {
?>
<td width="50%">今日：<font color=#FF0000>N/A</font></td><td width="50%">发贴：<?php echo $brd_artcnt[$i] ; ?></td>
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
arNews = ["<?php 
		$brdarr = array();
		$brdnum = bbs_getboard($AnnounceBoard, $brdarr);
		if ( ($brdnum==0) || ($brdarr["FLAG"] & BBS_BOARD_GROUP) ) {
			echo "当前没有公告";
		} else {
			$default_dir_mode = $dir_modes["NORMAL"];
			$total = bbs_countarticles($brdnum, $default_dir_mode);
			if ($total <= 0) {
				echo "当前没有公告";
			} else {
				$articles = bbs_getarticles($brdarr["NAME"], $total, 1, $default_dir_mode);
				if ($articles == FALSE) {
					echo "当前没有公告";
				} else {
					echo '<b><a href=\"\">' .$articles[0]['TITLE'] . '</a></b> ('.strftime('%Y-%m-%d %H-%M-%S', intval($articles[0]['POSTTIME'])).')';
				}
			}
		}
?>","","欢迎光临<?php echo $SiteName; ?>","",
"严禁恶意使用粗言秽语，违者经劝告无效，立即封ID。",""
];
</SCRIPT>
<div id="elFader" style="position:relative;visibility:hidden; height:16" ></div>
</td>
</tr>
<tr>
<td align=center width=100% valign=middle colspan=2>
<hr>
</td></tr>
<?php
}

function showTitle() {
?>
<TR><TD style="line-height: 20px;">
欢迎新加入会员 <a href=dispuser.php?name=<?php echo $rs[4]; ?> target=_blank><b><?php echo $rs[4]; ?></b></a>&nbsp;[<a href="toplist.php?orders=2">新进来宾</a>]<BR>论坛共有 <B><?php echo $rs[3]; ?></B> 位注册会员 , 主题总数：<b><?php echo $rs[0]; ?></b> , 帖子总数：<b><?php echo $rs[1]; ?></b><BR>今日论坛共发贴：<FONT COLOR="<?php echo $Forum_body[8]; ?>"><B><?php echo $rs[2]; ?></B></FONT> , 昨日发贴：<B><?php echo $rs[5]; ?></B> , 最高日发贴：<B><?php echo $rs[6]; ?></B></td><TD valign=bottom align=right style="line-height: 20px;"><a href=queryresult.php?stype=3>查看新贴</a> , <a href=hotlist.php?stype=1>热门话题</a> , <a href=toplist.php?orders=1>发贴排行</a> , <a href=toplist.php?orders=7>用户列表</a><BR>您最后一次访问是在：<?php echo strftime("%Y-%m-%d %H:%M:%S"); ?>
</TD></TR>
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
[<a href=reg.php>注册用户</a>]　[<a href=lostpass.php style="CURSOR: help">忘记密码</a>]
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
	$Forum_OnlineNum=bbs_getonlinenumber();
?>
<table cellpadding=5 cellspacing=1 class=tableborder1 align=center style="word-break:break-all;">
<TR><Th colSpan=2 align=left id=tabletitlelink height=25 style="font-weight:normal"><b>-=> 论坛在线统计</b>&nbsp;[<a href=showOnlineUsers.php?action=show>显示详细列表</a>] [<a href=boardstat.php?reaction=online>查看在线用户位置</a>]</Th></TR>
<TR><TD width=100% vAlign=top class=tablebody1>  目前论坛上总共有 <b><?php echo $Forum_OnlineNum ; ?></b> 人在线，其中注册会员 <b><?php echo  $OnlineNum ?></b> 人，访客 <b><?php echo  $GuestNum ?></b> 人。<br>
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

?>