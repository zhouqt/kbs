<?php
	/**
	 * This file lists boards to user.
	 * $Id$
	 */
	require("funcs.php");
	if ($loginok !=1 )
		html_nologin();
	else
	{
		html_init("gb2312");
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
	
?>
<body>
<table width="100%" border="0" cellspacing="0" cellpadding="3" >
  <tr> 
    <td colspan="2" class="kb2" colspan=2>
	    <a class="kts1"  href="mainpage.php"><?php echo BBS_FULL_NAME; ?></a>  - <a class="kts1"  href="bbssec.php">分类讨论区</a> - [<?php echo $section_names[$group][0]; ?>]    </td>
  </tr>
   <tr valign=bottom align=center> 
    <td align="left" class="kb4">&nbsp;&nbsp;&nbsp;&nbsp; <?php echo $section_names[$group][0]; ?>分区</td>
     <td align="right" class="kb1" >
<?php
	if( $group2 != -2 ){
		if ($yank == 0)
		{
?>
	    <a class="kts1" href="<?php echo $_SERVER["PHP_SELF"]; ?>?group=<?php echo $group; ?>&yank=1">本区所有看板</a> 
<?php
		}
		else
		{
?>
	   <a class="kts1" href="<?php echo $_SERVER["PHP_SELF"]; ?>?group=<?php echo $group; ?>">本区已订阅看板</a>
<?php
		}
	}
?>
    </td>
         
  </tr>
   <tr> 
    <td colspan="2" height="9" background="images/dashed.gif"> </td>
  </tr>
   <tr><td colspan="2" align="center">
    	<table width="100%" border="0" cellspacing="0" cellpadding="3" class="kt1">	
<form name="form1" action="bbsdoc.php" method="get">
<input type="hidden" name="board" value="Botany">
<tr>
<td class="kt2" width="2%"> </td>
<td class="kt2" width="2%"> </td>
<td class="kt2" width="24%">讨论区名称</td>
<td class="kt2" width="10%">类别</td>
<td class="kt2" width="41%">中文描述</td>
<td class="kt2" width="15%">版主</td>
<td class="kt2" width="6%">篇数</td>
</tr>
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
	if ($group2>0) 
	{	
?>
	<tr>
	<td class="kt3 c2" height="25"> </td>
	<td class="kt4 c2"> <img src="images/groupgroup.gif" height="15" width="20" alt="up" title="回到上一级"></td>
	<td class="kt3 c3" colspan="5" align="left"><a class="kts1"  href="bbsboa.php?group=<?php echo $group; ?>">回到上一级</a>
	</td>
	</TR>
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
		  $brd_link="/bbsboa.php?group=" . $group . "&group2=" . $brd_bid[$i];
		else
		  $brd_link="/bbsdoc.php?board=" . urlencode($brd_name[$i]);
?>
<tr>
<td class="kt3 c2" align=center height=25><?php echo $i+1; ?></td>

<?php
			if ($brd_flag[$i]&BBS_BOARD_GROUP)
			{
?>
<td class="kt4 c2"> <img src="images/groupgroup.gif" height="15" width="20"  alt="＋" title="版面组"></td><td class="kt3 c3">
<?php
			}
			else if ($brd_unread[$i] == 1)
			{
?>
<td class="kt4 c1"> <img src="images/newgroup.gif"  height="15" width="20" alt="◆" title="未读标志"></td><td class="kt3 c1">
<?php
			}
			else
			{
?>
<td class="kt4 c1" > <img src="images/oldgroup.gif" height="15" width="20"   alt="◇" title="已读标志"></td><td class="kt3 c1">
<?php
			}
			if ($yank == 1)
				{
				if ($brd_zapped[$i] == 1)
					echo "*";
				else
					echo "&nbsp;";
				}	
 ?><a class="kts1"  href="<?php echo $brd_link; ?>"><?php echo $brd_name[$i]; ?></a>
</td>
<?php
if ($brd_flag[$i]&BBS_BOARD_GROUP)
	{
?>
<td class="kt3 c3" align="center"><?php echo $brd_class[$i]?> </td>
<td class="kt3 c3"  colspan="3" align="left">&nbsp;&nbsp;<a class="kts1"  href="<?php echo $brd_link; ?>"><?php echo $brd_desc[$i]; ?></a>[目录]</td>
<?php
 	}
else
	{
?>
	<td class="kt3 c2" align="center"><?php echo $brd_class[$i]; ?></td>
<td class="kt3 c1">&nbsp;&nbsp;<a class="kts1" href="<?php echo $brd_link; ?>"><?php echo $brd_desc[$i]; ?></a>
</td>
<td class="kt3 c2" align="center">
<?php
			$bms = explode(" ", trim($brd_bm[$i]));
			if (strlen($bms[0]) == 0 || $bms[0][0] <= chr(32))
				echo "诚征版主中";
			else
			{
				if (!ctype_print($bms[0][0]))
					echo $bms[0];
				else
				{
?>
<a class="kts1" href="/bbsqry.php?userid=<?php echo $bms[0]; ?>"><?php echo $bms[0]; ?></a>
<?php
				}
			}
?>
</td>
<td class="kt3 c1"><?php echo $brd_artcnt[$i]; ?></td>
<?php
	}
?>
</tr>
<?php
		} //		for ($i = 0; $i < $rows; $i++)
?>
</table>
   <tr> 
    <td colspan="2" height="9" background="images/dashed.gif"> </td>
  </tr>
  </table>
</center>
<?php
		html_normal_quit();
	}
?>
