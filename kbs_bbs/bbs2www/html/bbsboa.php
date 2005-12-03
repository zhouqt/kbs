<?php
	require("www2-funcs.php");
	login_init();
	require("www2-board.php");

	if (isset($_GET["group"]))
		$group = $_GET["group"];
	else
		$group = 0;
	settype($group, "integer");
	if (isset($_GET["yank"]))
		$yank = $_GET["yank"];
	else
		$yank = 0;
	settype($yank, "integer");
	if (isset($_GET["group2"]))
		$group2 = $_GET["group2"];
	else
		$group2 = 0;
	settype($group, "integer");
	if ($group < 0 || $group > sizeof($section_nums))
		html_error_quit("错误的参数");
	$boards = bbs_getboards($section_nums[$group], $group2, $yank);
	//print_r($boards);
	if ($boards == FALSE)
		html_error_quit("该目录尚未有版面");

	page_header($section_names[$group][0]);
?>
<h1><?php echo $section_names[$group][0]; ?>分区</h1>
<div class="right smaller">
<?php
	if( $group2 != -2 ){
		if ($yank == 0) {
?>
<a href="<?php echo $_SERVER["PHP_SELF"]; ?>?group=<?php echo $group; ?>&yank=1">本区所有看板</a> 
<?php
		} else {
?>
<a href="<?php echo $_SERVER["PHP_SELF"]; ?>?group=<?php echo $group; ?>">本区已订阅看板</a>
<?php
		}
	}
?>
</div>
<table class="main wide adj">
<col width="2%" class="center"/><col width="2%"/><col width="24%"/><col width="10%" class="center"/><col width="39%"/><col class="center" width="15%"/><col class="right" width="8%"/>
<tr><th>#</th><th> </th><th>讨论区名称</th><th>类别</th><th>中文描述</th><th>版主</th><th>篇数</th></tr>
<?php
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
	if ($group2>0) {	
?>
<tr>
	<td> </td>
	<td> <img src="images/groupgroup.gif" alt="up" title="回到上一级"></td>
	<td colspan="5"><a href="bbsboa.php?group=<?php echo $group; ?>">回到上一级</a></td>
</tr>
<?php
	}
	
	$board_list = array();
	$list_gnum = $rows;
	$list_bnum = $rows;
	for ($i = $rows - 1; $i>=0 ; $i--)
	{
		if ($brd_flag[$i]&BBS_BOARD_GROUP)
		{
			$board_list[$list_gnum] = $i;
			$list_gnum = $list_gnum + 1;
		}
		else
		{
			$list_bnum  = $list_bnum - 1;
			$board_list[$list_bnum] = $i;
		}
	} 
	if ($list_gnum > $rows)
	{
		for  ($i = $rows; $i < $list_gnum; $i++)
		{
		$board_list[$i - $rows]= $board_list[$i];
		}
	}
	for ($j = 0; $j< $rows; $j++)	
	{
		$i = $board_list[$j];
		if ($brd_flag[$i]&BBS_BOARD_GROUP)
		  $brd_link="bbsboa.php?group=" . $group . "&group2=" . $brd_bid[$i];
		else
		  $brd_link="bbsdoc.php?board=" . urlencode($brd_name[$i]);
?>
<tr>
<td><?php echo $i+1; ?></td>
<?php
		if ($brd_flag[$i]&BBS_BOARD_GROUP) {
?>
<td> <img src="images/groupgroup.gif" alt="＋" title="版面组"></td>
<?php
		} else if ($brd_unread[$i] == 1) {
?>
<td> <img src="images/newgroup.gif" alt="◆" title="未读标志"></td>
<?php
		} else {
?>
<td> <img src="images/oldgroup.gif" alt="◇" title="已读标志"></td>
<?php
		}
?>
<td>
<?php
		if ($yank == 1) {
			if ($brd_zapped[$i] == 1)
				echo "*";
			else
				echo "&nbsp;";
		}	
?>
<a href="<?php echo $brd_link; ?>"><?php echo $brd_name[$i]; ?></a></td>
<?php
		if ($brd_flag[$i]&BBS_BOARD_GROUP) {
?>
<td><?php echo $brd_class[$i]?> </td>
<td colspan="3"><a href="<?php echo $brd_link; ?>"><?php echo $brd_desc[$i]; ?></a>[目录]</td>
<?php
		} else {
?>
<td><?php echo $brd_class[$i]; ?></td>
<td><a href="<?php echo $brd_link; ?>"><?php echo $brd_desc[$i]; ?></a></td>
<td>
<?php
			$bms = explode(" ", trim($brd_bm[$i]));
			if (strlen($bms[0]) == 0 || $bms[0][0] <= chr(32))
				echo "诚征版主中";
			else {
				if (!ctype_print($bms[0][0]))
					echo $bms[0];
				else {
?>
<a href="bbsqry.php?userid=<?php echo $bms[0]; ?>"><?php echo $bms[0]; ?></a>
<?php
				}
			}
?>
</td>
<td><?php echo $brd_artcnt[$i]; ?></td>
<?php
		}
?>
</tr>
<?php
	} //for
?>
</table>
<?php
	bbs_boards_navigation_bar();	
	page_footer();
?>
