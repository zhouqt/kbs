<?php
	/*
	** ID:windinsn dec 29,2003
	**/
	require("funcs.php");
login_init();
	require("libvote.php");
	
	if ($loginok != 1 || !isset($currentuser) )
		html_nologin();
	
	html_init("gb2312","系统投票","",1);
?>
<br /><p align=center><strong><?php echo BBS_FULL_NAME; ?>系统投票</strong></p>
<?php
	
	$link = sysvote_db_connect();
	$vote = sysvote_load_vote_infor($link,$_GET["svid"]);
	if(!$vote || $vote[active]!=0)
		html_error_quit("对不起，所选择的投票结果不存在！");
	
?>
<center>
<table cellspacing=0 cellpadding=5 width=100% class=t1 border=0>
	<tr>
		<td class=t2 colspan=2>查看投票结果</td>
	</tr>
	<tr>
		<td class=t3 width=100>投票主题</td>
		<td class=t7><?php echo $vote[subject]; ?></td>
	</tr>
	<tr>
		<td class=t3>投票说明</td>
		<td class=t7><?php echo $vote[desc]; ?></td>
	</tr>
	<tr>
		<td class=t3>开启时间</td>
		<td class=t7><?php echo $vote[created]; ?></td>
	</tr>
	<tr>
		<td class=t3>结束时间</td>
		<td class=t7><?php echo $vote[changed]; ?></td>
	</tr>
	<tr>
		<td class=t3>参与人数</td>
		<td class=t7><?php echo $vote[votecount]; ?></td>
	</tr>
	<tr>
		<td class=t3>投票结果</td>
		<td class=t7><?php echo sysvote_display_result($vote); ?>&nbsp;</td>
	</tr>
	
</table>
</center>
<p align=center>
[<a href="/<?php echo MAINPAGE_FILE; ?>">首页导读</a>]
[<a href="bbssysvote.php">系统投票</a>]
[<a href="javascript:history.go(-1)">快速返回</a>]
</p>
<?php	
	sysvote_db_close($link);
	html_normal_quit();
?>
