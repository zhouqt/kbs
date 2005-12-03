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
	if($_GET["svid"])
	{
		$vote = sysvote_load_vote_infor($link,$_GET["svid"]);
		if(!$vote)
			html_error_quit("对不起，所选择的投票不存在！");
		if($vote[active]!=1)
			echo "<script language=javascript>window.location.href=\"bbsssysvote.php?svid=".$vote[svid]."\";</script>";
		$per = sysvote_user_can_vote($link,$vote,$currentuser);
		if($per == -1)
			html_error_quit("对不起，本次投票需要登录！");
		elseif($per == -2)
			html_error_quit("对不起，您已经投过票了！");
		elseif($per == -3)
			html_error_quit("对不起，您所在的位置已经投过".$vote[votesperip]."次票了！");
		else
		{
			if($_GET["sv"]=="sv")
			{
				$ans = "";
				for($i=0;$i< count($vote[probs]) ; $i ++)
				{
					if($ans!="") $ans .= "|";
					if($vote[probs][$i][type]==4)//单选
						$ans .= base64_encode($_POST["ans".$i]);
					elseif($vote[probs][$i][type]==1 || $vote[probs][$i][type]==3)//多选
					{
						$l = 0;
						for($j = 0 ; $j < count($vote[probs][$i][etems]) ; $j ++)
						{
							if($_POST["ans".$i."e".$j])
							{
								if($l != 0)
									$ans .= "&";
								$ans .= ($j + 1);
								$l ++ ;
							}
						}
					}	
					else
						$ans .= (int)($_POST["ans".$i]);	
					if($vote[probs][$i][type]==2 || $vote[probs][$i][type]==3)
						$ans .= "&".base64_encode($_POST["ans".$i."p"]);
				}
				sysvote_user_vote($link,$vote,$currentuser,$ans);
?>
<br /><br /><br />
<p align=center><a href="/<?php echo MAINPAGE_FILE; ?>">投票成功，点击返回</a></p>
<?php				
			}
			else
			{
?>
<hr size=1>
<center>
<form action="<?php echo $_SERVER["PHP_SELF"]."?svid=".$vote[svid]."&sv=sv"; ?>" method="post">
<table cellspacing=0 cellpadding=5 width=700 class=t1>
	<tr><td class=t2>[投票] <?php echo $vote[subject]; ?></td></tr>
	<tr><td class=t5><?php echo $vote[desc]; ?></td></tr>
	<tr><td class=t7><?php echo sysvote_display_probs($vote[probs]); ?></td></tr>
	<tr><td class=t2>
		<input type=submit value="投票" class=b9>
		<input type=button value="返回" class=b9 onclick="window.location.href='/<?php echo MAINPAGE_FILE; ?>';">	
	</td></tr>
</table></form>
</center>
<?php
			}
		}
	}
	else
	{
		$votes = sysvote_load_votes($link,(int)($_GET["pno"]));
?>
<hr size=1>
<center>
<table cellspacing=0 cellpadding=5 width=95% class=t1>
	<tr>
		<td width=30 class=t2>序号</td>
		<td width=45 class=t2>状态</td>
		<td class=t2>主题</td>
		<td width=150 class=t2>创建时间</td>
		<td width=60 class=t2>投票人数</td>
		<td width=60 class=t2>投票天数</td>
	</tr>
<?php
		$j = 1;
		foreach($votes as $vote)
		{
			echo "<tr>".
			     "<td class=t3>".$j."</td><td class=t4>";
			if($vote[active]==1)
			{
				echo "投票中";
				$linkUrl = "bbssysvote.php?svid=".$vote[svid];
			}
			else
			{
				echo "已结束";
				$linkUrl = "bbsssysvote.php?svid=".$vote[svid];
			}	
			echo "</td><td class=t5><span title=\"".$vote[desc]."\"><a href=\"".$linkUrl."\">".$vote[subject]."</a><span></td>".
			     "<td class=t4>".$vote[created]."</td>".
			     "<td class=t3>".$vote[votecount]."</td>".
			     "<td class=t4>".$vote[timelong]."</td></tr>\n";
			$j ++;
		}
?>
</table>
</center>
<?php
	echo "<p align=right>";
	$pno = $_GET["pno"];
	if($pno < 1) $pno = 1;
	if($pno > 1) echo "<a href=\"".$_SERVER["PHP_SELF"]."?pno=".($pno-1)."\">上一页</a>\n";
	if(count($votes) == $sysVoteConfig["PAGESIZE"]) echo "<a href=\"".$_SERVER["PHP_SELF"]."?pno=".($pno + 1)."\">下一页</a>\n";
	echo "</p>";	
	}
?>
<hr size=0>
<p align=center><a href="/<?php echo MAINPAGE_FILE; ?>"><?php echo BBS_FULL_NAME; ?></a>
<?php
	if(sysvote_is_admin($currentuser)) echo "[<a href=\"bbsmsysvote.php\">系统投票管理界面</a>]";
?>
</p>
<?php	
	sysvote_db_close($link);
	html_normal_quit();
?>
