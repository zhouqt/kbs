<?php
	require("www2-funcs.php");
	login_init();
	bbs_session_modify_user_mode(BBS_MODE_SELECT);
	require("www2-board.php");

	if (isset($_GET["group"]))
		$group = $_GET["group"];
	else
		$group = 0;
	settype($group, "integer");
	if (isset($_GET["group2"]))
		$group2 = $_GET["group2"];
	else
		$group2 = 0;
	settype($group2, "integer");
	if ($group < 0 || $group >= BBS_SECNUM)
		html_error_quit("错误的参数");
	$boards = bbs_getboards(constant("BBS_SECCODE".$group), $group2, 8);
	if ($boards === FALSE)
		html_error_quit("该目录尚未有版面");

	page_header(constant("BBS_SECNAME".$group."_0"), "", "<meta name='kbsrc.brd' content='' />");
?>
<h1><?php echo constant("BBS_SECNAME".$group."_0"); ?>分区</h1>
<script type="text/javascript"><!--
var o = new brdWriter(<?php echo $group; ?>, <?php echo $group2; ?>);
<?php
	$rows = sizeof($boards);
	$board_list = array();
	$list_gnum = $rows;
	$list_bnum = $rows;
	for ($i = $rows - 1; $i>=0 ; $i--)
	{
		if ($boards[$i]["FLAG"]&BBS_BOARD_GROUP)
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
		$board = $boards[$board_list[$j]];		
		$isGroup = ($board["FLAG"]&BBS_BOARD_GROUP) ? "true" : "false";
?>
o.o(<?php echo $isGroup; ?>,<?php echo ($board["UNREAD"] == 1) ? "1" : "0"; ?>,<?php echo $board["BID"]; ?>,<?php echo $board["LASTPOST"]; 
?>,'<?php echo $board["CLASS"]; ?>','<?php echo addslashes($board["NAME"]); ?>','<?php echo addslashes($board["DESC"]);
?>','<?php echo $board["BM"]; ?>',<?php echo $board["ARTCNT"]; ?>,0,<?php echo $board["CURRENTUSERS"];?>);
<?php		
	}
?>
o.t();
//-->
</script>
<?php
	bbs_boards_navigation_bar();	
	page_footer();
?>
