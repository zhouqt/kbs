<?php
	require("www2-funcs.php");
	require("www2-board.php");
	login_init();
	$xmls = array(array("/xml/rcmdbrd.xml", "推荐讨论区", "EnglishName"),
	              array("/xml/board.xml", "讨论区人气排名", "EnglishName"),
	              array("/xml/newboards.xml", "新开讨论区", "filename")
	              );
	
	$index = @intval($_GET["flag"]);
	if ($index < 0 || $index >= sizeof($xmls)) $index = 0;
	$xmlfile = $xmls[$index][0];
	$xmltitle = $xmls[$index][1];
	$nodename = $xmls[$index][2];
	
	$boardrank_file = BBS_HOME . $xmlfile;
	if (cache_header("public",filemtime($boardrank_file),3600))
		return;
	$doc = domxml_open_file($boardrank_file);
	if (!$doc)
		html_error_quit("目前尚无" . $xmltitle);
		
	$root = $doc->document_element();
	$boards = $root->child_nodes();
	$brdarr = array();
	
	page_header($xmltitle);
?>
<script type="text/javascript"><!--
var ta = new tabWriter(1,'main wide','<?php echo $xmltitle; ?>',
[['#','2%','center'],[' ','2%',0],['讨论区名称','24%',0],['类别','10%','center'],['中文描述','47%',0],['版主','15%','center']]);
<?php
	$i = 0;
	while($board = array_shift($boards))
	{
		if ($board->node_type() == XML_TEXT_NODE)
			continue;
		$ename = find_content($board, $nodename);
		$brdnum = bbs_getboard($ename, $brdarr);
		if ($brdnum == 0)
			continue;
		if ((++$i) > 100) break;
		$brd_encode = urlencode($brdarr["NAME"]);
		$col1 = "putImageCode('newgroup.gif','')";
		$col2 = '<a href="bbsdoc.php?board=' . $brd_encode . '">' . $brdarr["NAME"] . '</a>';
		$col3 = $brdarr["CLASS"];
		$col4 = '<a href="bbsdoc.php?board=' . $brd_encode . '">' . htmlspecialchars($brdarr["DESC"], ENT_QUOTES) . '</a>';
		$bms = explode(" ", trim($brdarr["BM"]));
		if (strlen($bms[0]) == 0 || $bms[0][0] <= chr(32))
			$col5 = "诚征版主中";
		else
		{
			if (!ctype_print($bms[0][0]))
				$col5 = $bms[0];
			else
			{
				$col5 = '<a href="bbsqry.php?userid=' . $bms[0] . '">' . $bms[0] . '</a>';
			}
		}
		echo "ta.r($col1,'$col2','$col3','$col4','$col5');\n";
	} //end while
?>
ta.t();
//-->
</script>
<?php
	bbs_boards_navigation_bar();
	page_footer();
?>
