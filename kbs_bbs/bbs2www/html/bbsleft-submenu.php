<?php
require("funcs.php");
if(isset($_GET["fav"]))
{
	$boardList = FALSE;
	$select = intval($_GET["select"]);
	if( bbs_load_favboard($select)==-1 ) exit();
	$subMenu = "submenu_fav_".$select;
	$boards = bbs_fav_boards($select, 1);
	bbs_release_favboard(); 
}
else
{
	$boardList = TRUE;
	$group = intval($_GET["group"]);
	$group2 = intval($_GET["group2"]);
	$yank = intval($_GET["yank"]);
	$subMenu = "submenu_brd_".$group."_".$group2;
	$boards = bbs_getboards($group, $group2, $yank);
}

$brd_name = $boards["NAME"]; // 英文名
$brd_desc = $boards["DESC"]; // 中文描述
$brd_flag = $boards["FLAG"]; //flag
$brd_bid = $boards["BID"]; //flag
$brd_num = sizeof($brd_name);

$innerHtml = "<table width=100% border=0 cellpadding=0 cellspacing=0>";
for($j = 0 ; $j < $brd_num ; $j ++ )
{
	if ($brd_flag[$j]&BBS_BOARD_GROUP)
	{
		if($boardList)
		{
			$innerHtml .= "<tr><td align='right' width='16'>".
				"<a href='javascript:submenu(0,0,".$group.",".$brd_bid[$j].",0)'>".
				"<img id='submenuimg_brd_".$group."_".$brd_bid[$j]."' src='/images/close.gif' border='0'>".
				"</a></td><td align='left'>".
				"<a href='/bbsboa.php?group=".$group."&group2=".$brd_bid[$j]."' target='f3'>".
				"<img src='/images/kfolder1.gif' border='0'>".$brd_desc[$j]."</a>".
				"</td></tr>".
				"<tr id='submenu_brd_".$group."_".$brd_bid[$j]."' style='display:none'>".
				"<td background='/images/line3.gif'> </td>".
				"<td id='submenu_brd_".$group."_".$brd_bid[$j]."_td'><DIV></DIV>".
				"</td></tr>";
		}
		elseif( bbs_load_favboard($brd_bid[$j])!=-1 && bbs_fav_boards($brd_bid[$j], 1) && $brd_bid[$j]!= -1)
		{
			$innerHtml .= "<tr><td align='right' width='16'>".
				"<a href='javascript:submenu(1,".$brd_bid[$j].",0,0,0)'>".
				"<img id='submenuimg_fav_".$brd_bid[$j]."' src='/images/close.gif' border='0'>".
				"</a></td><td align='left'>".
				"<a href='/bbsfav.php?select=".$brd_bid[$j]."&up=".$select."' target='f3'>".
				"<img src='/images/kfolder1.gif' border='0'>".$brd_desc[$j]."</a>".
				"</td></tr>".
				"<tr id='submenu_fav_".$brd_bid[$j]."' style='display:none'>".
				"<td background='/images/line3.gif'> </td>".
				"<td id='submenu_fav_".$brd_bid[$j]."_td'><DIV></DIV>".
				"</td></tr>";
	        }
		else
		{
			
		}
	}
	else
	{
		$brd_link="/bbsdoc.php?board=" . urlencode($brd_name[$j]);
		$innerHtml .= "<tr><td width='16' align='right'>";
		if( $j != $brd_num - 1 )
			$innerHtml .= "<img src='/images/line.gif' align='absmiddle'>";
		else
			$innerHtml .= "<img src='/images/line1.gif' align='absmiddle'>";

		$innerHtml .= "</td><td><A href='".$brd_link."' target='f3'>".$brd_desc[$j]."</A></td></tr>";
	}
}
$innerHtml .= "</table>";
if(!$boards || !$brd_num) $innerHtml = "<font color=#eeeeee>-空目录-</font>"; 
?>
<html>
<script src="/bbsleft.js"></script>
<body>
<script language="javascript">
parent.document.all('<?php echo $subMenu; ?>_td').innerHTML = "<?php echo $innerHtml; ?>" ; 
</script>
</body></html>