<?php

$setboard=1;

require("inc/funcs.php");
require("inc/user.inc.php");

show_nav();

showUserMailBoxOrBR();
?>
<script src="inc/loadThread.js"></script>
<?php
	if (BOARDLISTSTYLE == 'simplest') {
?>
<script language="JavaScript">
<!--
	simplestBoardsList = true;
//-->
</script>
<?php
	}
?>
<table cellSpacing=0 cellPadding=0 width=97% border=0 align=center>
<?php
showAnnounce(); 
?>
<tr><td align=center width=100% valign=middle colspan=2>
<hr>
</td></tr>
<?php
showTitle();
?>
</table>
<?php

if ($loginok!=1) {
	FastLogin();
}

showAllSecs();

showUserInfo();
showOnlineUsers();
showSample();

show_footer();

function showAllSecs(){
	GLOBAL $sectionCount;
	
	for ($i=0;$i<$sectionCount;$i++){
		showSecs($i,0,getSecFoldCookie($i));
		flush();
	}
	return false;
}


function showTitle() {
	global $dir_modes;
	$boardID=bbs_getboard('newcomers');
	$num=bbs_countarticles($boardID, $dir_modes['normal']);
	$user=bbs_getarticles("newcomers",$num,1,$dir_modes['normal']);
?>
<TR><TD style="line-height: 20px;">
欢迎新加入会员 <a href=dispuser.php?id=<?php echo $user[0]['OWNER']; ?> target=_blank><b><?php echo $user[0]['OWNER']; ?></b></a>&nbsp;[<a href="board.php?name=newcomers">新进来宾</a>]
<?php /*
<BR>论坛共有 <B><?php  ?></B> 位注册会员 , 主题总数：<b><?php echo $rs[0]; ?></b> , 帖子总数：<b><?php echo $rs[1]; ?></b><BR>今日论坛共发贴：<FONT COLOR="<?php echo $Forum_body[8]; ?>"><B><?php echo $rs[2]; ?></B></FONT> , 昨日发贴：<B><?php echo $rs[5]; ?></B> , 最高日发贴：<B><?php echo $rs[6]; ?></B> */
?>
</td><TD valign=bottom align=right style="line-height: 20px;"><!--<a href=# onclick="alert('本功能尚在开发中！');">查看新贴</a> , --><a href=topten.php>热门话题</a> , <!--<a href=# onclick="alert('本功能尚在开发中！');">发贴排行</a> , --><a href="showonlineuser.php">在线用户</a>
<!--<BR>您最后一次访问是在：<?php echo strftime("%Y-%m-%d %H:%M:%S"); ?>-->
</TD></TR>
<?php
}

function FastLogin()
{
?>
<table cellspacing=1 cellpadding=3 align=center class=TableBorder1>
<form action="logon.php" method=post>
<input type="hidden" name="action" value="doLogon">
<tr>
<th align=left id=TableTitleLink height=25 style="font-weight:normal">
<b>-=&gt; 快速登录入口</b>
[<a href=register.php>注册用户</a>]　<!--[<a href=lostpass.php style="CURSOR: help">忘记密码</a>]-->
</th>
</tr>
<tr>
<td class=TableBody1 height=40 width="100%">
&nbsp;用户名：<input maxLength=16 name=id size=12>　　密码：<input maxLength=20 name=password size=12 type=password>　　<select name=CookieDate><option selected value=0>不保存</option><option value=1>保存一天</option><option value=2>保存一月</option><option value=3>保存一年</option></select><input type=hidden name=comeurl value="<?php echo $_SERVER['PHP_SELF']; ?>">&nbsp;<input type=submit name=submit value="登 录">
</td>
</tr>
</form>
</table><br>
<?php 
}
?>
