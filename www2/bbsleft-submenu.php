<?php
require_once("www2-funcs.php");
login_init();
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
	$yank = intval($_GET["yank"]);
	if ($yank) $yank = 1;
	$subMenu = "submenu_brd_".$group."_".$group2;
	if($group < 0 || $group >= BBS_SECNUM )
		exit();
	$boards = bbs_getboards(constant("BBS_SECCODE".$group), $group2, $yank | 2);
}
$brd_name = $boards["NAME"]; // 英文名
$brd_desc = $boards["DESC"]; // 中文描述
$brd_flag = $boards["FLAG"]; //flag
$brd_bid = $boards["BID"]; //flag
$brd_num = sizeof($brd_name);

$innerHtml = "";
for($j = 0 ; $j < $brd_num ; $j ++ )
{
	if ($brd_flag[$j]&BBS_BOARD_GROUP)
	{
		if($boardList)
		{
			$innerHtml .= "<div class='fi'>".
				"<a href='javascript:submenu(0,0,".$group.",".$brd_bid[$j].",0)' target='_self'>".
				"<img id='submenuimg_brd_".$group."_".$brd_bid[$j]."' src='images/close.gif' class='pm' alt='+'>".
				"</a><a href='bbsboa.php?group=".$group."&group2=".$brd_bid[$j]."'>".
				"\"+putImageCode('kfolder1.gif','class=\"s16x16\"')+\"".$brd_desc[$j]."</a></div>".
				"<div id='submenu_brd_".$group."_".$brd_bid[$j]."' class='lineback'></div>";
			continue;
		}
		elseif( $brd_bid[$j] == -1) { //空收藏目录
			continue;
		} elseif ($brd_flag[$j] == -1) { //收藏目录
			$innerHtml .= "<div class='fi'>".
				"<a href='javascript:submenu(1,".$brd_bid[$j].",0,0,0)' target='_self'>".
				"<img id='submenuimg_fav_".$brd_bid[$j]."' src='images/close.gif' class='pm' alt='+'>".
				"</a><a href='bbsfav.php?select=".$brd_bid[$j]."&up=".$select."'>".
				"\"+putImageCode('kfolder1.gif','class=\"s16x16\"')+\"".$brd_desc[$j]."</a></div>".
				"<div id='submenu_fav_".$brd_bid[$j]."' class='lineback'></div>";
			continue;
		} else {
			//普通目录版面！！
		}
	}
	$brd_link="bbsdoc.php?board=" . urlencode($brd_name[$j]);
	if( $j != $brd_num - 1 )
		$innerHtml .= "<div class='lb'><div class='mi'><a href='".$brd_link."'>".$brd_desc[$j]."</a></div></div>";
	else
		$innerHtml .= "<div class='lmi'><a href='".$brd_link."'>".$brd_desc[$j]."</a></div>";
}
if (!$innerHtml) $innerHtml = "-空目录-";
$innerHtml = "<div class='b1'>" . $innerHtml . "</div>";
cache_header("nocache");
?>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=gb2312">
</head>
<script src="static/www2-main.js"></script>
<script src="static/bbsleft.js"></script>
<body>
<script type="text/javascript"><!--
parent.document.getElementById('<?php echo $subMenu; ?>').innerHTML = "<?php echo $innerHtml; ?>" ; 
//-->
</script>
</body></html>
