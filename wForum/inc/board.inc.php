<?php

function showBoardStaticsTop($boardArr){
?>
<TABLE cellpadding=3 cellspacing=1 class=tableborder1 align=center><TR><Th height=25 width=100% align=left id=tabletitlelink style="font-weight:normal">本版当前共有<b><?php echo $boardArr['CURRENTUSERS'];?></b>人在线 </Th></TR></td></tr></TABLE>
<BR>
<table cellpadding=0 cellspacing=0 border=0 width=97% align=center valign=middle><tr><td align=center width=2> </td><td align=left><a href=announce.php?boardid=2><span class="buttonclass1" border=0 alt=发新帖></span></a>&nbsp;&nbsp;<a href=vote.php?boardid=2><span class="buttonclass2" border=0 alt=发起新投票></span>&nbsp;&nbsp;<a href=smallpaper.php?boardid=2><span class="buttonclass3" border=0 alt=发布小字报></span></a></td><td align=right><img src=pic/team2.gif align=absmiddle>
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

function showBoardContents($boardID,$boardName){
	global $dir_modes;
	$dir_mode = $dir_modes["ORIGIN"];
?>
<form action=admin_batch.asp method=post name=batch><TR align=middle><Th height=25 width=32 id=tabletitlelink><a href=list.asp?name=<?php echo $boardName; ?>&page=&action=batch>状态</a></th><Th width=* id=tabletitlelink>主 题  (点<img src=pic/plus.gif align=absmiddle>即可展开贴子列表)</Th><Th width=80 id=tabletitlelink>作 者</Th><Th width=64 id=tabletitlelink>回复</Th><Th width=195 id=tabletitlelink>最后更新 | 回复人</Th></TR>
<?php
	$total = bbs_countarticles($boardID, $dir_mode);
	if ($total<=0)
	$totalPages=$total;
	$articles = bbs_getarticles($boardName, 0, 100, $dir_modes["ORIGIN"]);
	$articleNum=count($articles);
	for($i=0;$i<$articleNum;$i++){
?>
<TR align=middle><TD class=tablebody2 width=32 height=27><img src="pic/blue/folder.gif" alt=开放主题></TD><TD align=left class=tablebody1 width=* ><img src="pic/nofollow.gif" id="followImg1"><a href="dispbbs.asp?boardName=<?php echo $boardName ;?>&ID=<?php echo $articles[$i]['ID'] ;?>" title="<?php echo $articles[$i]['TITLE'] ;?><br>作者：<?php echo $articles[$i]['OWNER'] ;?><br>发表于2003-6-2 14:04:17<br>最后跟贴：asdfsadf..."><?php echo $articles[$i]['TITLE'] ;?></a></TD><TD class=tablebody2 width=80><a href="dispuser.asp?id=<?php echo $articles[$i]['OWNER'] ;?>" target=_blank><?php echo $articles[$i]['OWNER'] ;?></a></TD><TD class=tablebody1 width=64>0/2</TD><TD align=left class=tablebody2 width=195>&nbsp;&nbsp;<a href="dispbbs.asp?boardid=1&id=1&star=1#1">2003-6-2&nbsp;14:04</a>&nbsp;<font color=#FF0000>|</font>&nbsp;<a href=dispuser.asp?id=4 target=_blank>Roy</a></TD></TR>
<tr style="display:none" id="follow<?php echo $i;?>"><td colspan=5 id="<?php echo $i;?>" style="padding:0px"><div style="width:240px;margin-left:18px;border:1px solid black;background-color:lightyellow;color:black;padding:2px" onclick="loadThreadFollow(<?php echo $i;?>,1)">正在读取关于本主题的跟贴，请稍侯……</div></td></tr>
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

?>