<?php
require_once("inc/funcs.php");
if(isset($_GET["fav"]))
{
	$boardList = FALSE;
	$select = intval($_GET["select"]);
	if( bbs_load_favboard($select)==-1 ) exit();
	$subMenu = "submenu_fav_".$select;
	$boards = bbs_fav_boards($select, 1);
}
else
{
	$boardList = TRUE;
	$group = intval($_GET["group"]);
	$group2 = intval($_GET["group2"]);
	if ($yank) $yank = 1;
	else $yank = 0;
	$subMenu = "submenu_brd_".$group."_".$group2;
	if($group < 0 || $group > sizeof($section_nums) )
		exit();
	$boards = bbs_getboards($section_nums[$group], $group2, $yank | 2);
}

$brd_name = $boards["NAME"]; // 英文名
$brd_desc = $boards["DESC"]; // 中文描述
$brd_flag = $boards["FLAG"]; //flag
$brd_bid = $boards["BID"]; //flag
$brd_num = sizeof($brd_name);

$innerHtml = "<table width='100%' border='0' cellspacing='0' cellpadding='1'><col width='16px'/><col align='left'/>";
for($j = 0 ; $j < $brd_num ; $j ++ )
{
	if ($brd_flag[$j]&BBS_BOARD_GROUP)
	{
		if($boardList)
		{
			$innerHtml .= "<tr><td>".
				"<a href='javascript:submenu(0,0,".$group.",".$brd_bid[$j].",0)'>".
				"<img id='submenuimg_brd_".$group."_".$brd_bid[$j]."' src='pic/plus.gif' border='0'>".
				"</a></td><td>".
				"<a href='board.php?name=".$brd_name[$j]."' target='main'><nobr>".$brd_desc[$j]."</nobr></a>".
				"</td></tr>".
				"<tr id='submenu_brd_".$group."_".$brd_bid[$j]."' style='display:none'>".
				"<td> </td>".
				"<td id='submenu_brd_".$group."_".$brd_bid[$j]."_td'><DIV></DIV>".
				"</td></tr>";
		}
		elseif( bbs_load_favboard($brd_bid[$j])!=-1 && bbs_fav_boards($brd_bid[$j], 1) && $brd_bid[$j]!= -1)
		{
			$innerHtml .= "<tr><td>".
				"<a href='javascript:submenu(1,".$brd_bid[$j].",0,0,0)'>".
				"<img id='submenuimg_fav_".$brd_bid[$j]."' src='pic/plus.gif' border='0'>".
				"</a></td><td>".
				"<a href='favboard.php?select=".$brd_bid[$j]."&up=".$select."' target='main'><nobr>".$brd_desc[$j]."</nobr></a>".
				"</td></tr>".
				"<tr id='submenu_fav_".$brd_bid[$j]."' style='display:none'>".
				"<td> </td>".
				"<td id='submenu_fav_".$brd_bid[$j]."_td'><div></div>".
				"</td></tr>";
	        }
		else
		{
			
		}
	}
	else
	{
		$brd_link="board.php?name=" . urlencode($brd_name[$j]);
		$innerHtml .= "<tr><td>·</td>";
		$innerHtml .= "<td><a href='".$brd_link."' target='main'><nobr>".$brd_desc[$j]."</nobr></a></td></tr>";
	}
}
$innerHtml .= "</table>";
if(!$boards || !$brd_num) $innerHtml = "<font color=#eeeeee>-空目录-</font>"; 
html_init();
?>
<script src="/bbsleft.js"></script>
<body>
<script language="javascript">
parent.document.getElementById('<?php echo $subMenu; ?>_td').innerHTML = "<?php echo $innerHtml; ?>" ; 
</script>
</body></html>
