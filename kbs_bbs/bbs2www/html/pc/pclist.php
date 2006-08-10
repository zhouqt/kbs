<?php
require("pcfuncs.php");
require("pcstat.php");
require("pcmainfuncs.php");

if(pc_update_cache_header(60*24))
	return;

function pcmain_hot_users($link,$period,$color)
{
	$users = getHotUsersByPeriod($link,$period,50);
	for($i = 0;$i < count($users) ; $i ++)
		echo "<font color=\"".$color."\">".($i+1)."</font>&nbsp;<a href=\"/pc/index.php?id=".$users[$i]["username"]."\" title=\"".htmlspecialchars($users[$i]["corpusname"])."\">".html_format(html_format_fix_length($users[$i]["corpusname"],20))."</a>".
		     "\n<a href=\"/bbsqry.php?userid=".$users[$i]["username"]."\"><font class=low2>".$users[$i]["username"]."</font></a><br />";
}

function pcmain_score_top_users($link, $color)
{
	$users = getScoreTopUsers($link, 50);
	for($i = 0;$i < count($users) ; $i ++)
		echo "<font color=\"".$color."\">".($i+1)."</font>&nbsp;<a href=\"/pc/index.php?id=".$users[$i]["username"]."\" title=\"".htmlspecialchars($users[$i]["corpusname"])."\">".html_format(html_format_fix_length($users[$i]["corpusname"],20))."</a>".
		     "\n<a href=\"/bbsqry.php?userid=".$users[$i]["username"]."\"><font class=low2>".$users[$i]["username"]."</font></a><br />";
}

function pcmain_hot_nodes($link,$period,$color)
{
	$nodes = getHotNodesByPeriod($link,$period,50);
	for($i = 0;$i < count($nodes) ; $i ++)
		echo "<font color=\"".$color."\">".($i+1)."</font>&nbsp;<a href=\"/pc/pccon.php?id=".$nodes[$i]["uid"]."&nid=".$nodes[$i]["nid"]."&s=all\" title=\"".htmlspecialchars($nodes[$i]["subject"])."\">".html_format(html_format_fix_length($nodes[$i]["subject"],32))."</a><br />";
}

function pcmain_hot_topics($link,$period,$color)
{
	$topics = getHotTopicsByPeriod($link,$period,50);
	for($i = 0;$i < count($topics) ; $i ++)
		echo "<font color=\"".$color."\">".($i+1)."</font>&nbsp;<a href=\"/pc/pcdoc.php?userid=".$topics[$i]["username"]."&tid=".$topics[$i][1]."&tag=0\" title=\"".htmlspecialchars($topics[$i]["topicname"])."\">".html_format(html_format_fix_length($topics[$i]["topicname"],32))."</a>".
		     "\n<a href=\"/pc/index.php?id=".$topics[$i]["username"]."\"><font class=low2>".$topics[$i]["username"]."</font></a><br />";
}

$link = pc_db_connect();
pcmain_html_init();

?>
<tbody>
<form action="pcsearch.php" method="get" onsubmit="if(this.keyword.value==''){alert('请输入关键字');return false;}">
<tr>
	<td bgcolor="#F6F6F6">
	<table width="100%"><tr><td align="left">
	BLOG搜索
	<input name="keyword" type="text" class="textinput" size="20"> 
	<input type="hidden" name="exact" value="0">
	<select name="key">
	<option value="u" selected>用户名</option>
	<option value="c">Blog名</option>
	<option value="d">Blog描述</option>
	</select>
	<input type="submit" class="textinput" value="开始搜">
	</td><td align="right">
	<a href="/pc/pc.php">注册用户</a> <?php echo getUsersCnt($link); ?> 人
	<a href="/pc/pcnew.php">日志</a> <?php echo getNodesCnt($link); ?> 个
	<a href="/pc/pcnew.php?t=c">评论</a> <?php echo getCommentsCnt($link); ?> 篇
	</td></tr></table>
	</td>
</tr>
</tbody>
<tr>
	<td>
	<table width="100%" cellpadding="0" cellspacing="0" border="0">
<tbody>
		<tr>
		<td width="33%" valign="top">
			<table class="table2" width="100%" cellspacing="0" cellpadding="3" >
				<tr>
					<td class="td3" bgcolor="#adbe00" align="center"><b><font color="white">最热日志(24小时内)</font></b></td>			
				</tr>
				<tr>
					<td align="left" class="td4">
					<?php pcmain_hot_nodes($link,"day","#adbe00"); ?>
					</td>			
				</tr>
			</table>
		</td>
		<td width="33%" valign="top">
			<table class="table2" width="100%" cellspacing="0" cellpadding="3" >
				<tr>
					<td class="td3" bgcolor="#dfd581" align="center"><b><font color="white">最热日志(30天内)</font></b></td>			
				</tr>
				<tr>
					<td align="left" class="td4">
					<?php pcmain_hot_nodes($link,"month","#dfd581"); ?>
					</td>			
				</tr>
			</table>
		</td>
		<td width="33%" valign="top">
			<table class="table2" width="100%" cellspacing="0" cellpadding="3" >
				<tr>
					<td align="center" class="td3" bgcolor="#ff00cc"><b><font color="white">最热日志</font></b></td>			
				</tr>
				<tr>
					<td align="left" class="td3">
					<?php pcmain_hot_nodes($link,"all","#ff00cc"); ?>
					</td>			
				</tr>
			</table>
		</td>
		</tr>
	</tbody>
	<tbody>	
		<tr>
		<td colspan="3" bgcolor="#999999" height="3"> </td>
		</tr>
		<tr>
		<td width="33%" valign="top">
			<table class="table2" width="100%" cellspacing="0" cellpadding="3" >
				<tr>
					<td class="td3" bgcolor="#ffb600" align="center"><b><font color="white">最热栏目(24小时内)</font></b></td>			
				</tr>
				<tr>
					<td align="left" class="td4">
					<?php pcmain_hot_topics($link,"day","#ffb600"); ?>
					</td>			
				</tr>
			</table>
		</td>
		<td width="33%" valign="top">
			<table class="table2" width="100%" cellspacing="0" cellpadding="3" >
				<tr>
					<td class="td3" bgcolor="#00b6ef" align="center"><b><font color="white">最热栏目(30天内)</font></b></td>			
				</tr>
				<tr>
					<td align="left" class="td4">
					<?php pcmain_hot_topics($link,"month","#00b6ef"); ?>
					</td>			
				</tr>
			</table>
		</td>
		<td width="33%" valign="top">
			<table class="table2" width="100%" cellspacing="0" cellpadding="3" >
				<tr>
					<td align="center" class="td3" bgcolor="#f75151"><b><font color="white">最热栏目</font></b></td>			
				</tr>
				<tr>
					<td align="left" class="td3">
					<?php pcmain_hot_topics($link,"all","#f75151"); ?>
					</td>			
				</tr>
			</table>
		</td>
		</tr>
	</tbody>
	<tbody>
		<tr>
		<td colspan="3" bgcolor="#999999" height="3"> </td>
		</tr>
		<tr>
		<td width="33%" valign="top">
			<table class="table2" width="100%" cellspacing="0" cellpadding="3" >
				<tr>
					<td class="td3" bgcolor="#f75151" align="center"><b><font color="white">最热BLOG(24小时内)</font></b></td>			
				</tr>
				<tr>
					<td align="left" class="td4">
					<?php pcmain_hot_users($link,"day","#f75151"); ?>
					</td>			
				</tr>
			</table>
		</td>
		<td width="33%" valign="top">
			<table class="table2" width="100%" cellspacing="0" cellpadding="3" >
				<tr>
					<td class="td3" bgcolor="#00b6ef" align="center"><b><font color="white">最热BLOG(30天内)</font></b></td>			
				</tr>
				<tr>
					<td align="left" class="td4">
					<?php pcmain_hot_users($link,"month","#00b6ef"); ?>
					</td>			
				</tr>
			</table>
		</td>
		<td width="33%" valign="top">
			<table class="table2" width="100%" cellspacing="0" cellpadding="3" >
				<tr>
					<td align="center" class="td3" bgcolor="#4cb81c"><b><font color="white">最热BLOG</font></b></td>			
				</tr>
				<tr>
					<td align="left" class="td3">
<?php 
	if(defined("_BLOG_SCORE_STAT_"))
		pcmain_score_top_users($link, "#4cb81c");
	else
		pcmain_hot_users($link,"all","#4cb81c");
?>
					</td>			
				</tr>
			</table>
		</td>
		</tr>
	</tbody>
	</table>
	</td>
</tr>
<tr>
	<td height="5"> </td>
</tr>
</form>
<?php
pc_db_close($link);
pcmain_html_quit()
?>
