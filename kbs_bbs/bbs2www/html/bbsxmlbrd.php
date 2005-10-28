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
<table class="main wide">
<caption><?php echo $xmltitle; ?></caption>
<col width="2%" class="center"/><col width="2%"/><col width="24%"/><col width="10%" class="center"/><col width="47%"/><col class="center" width="15%"/>
<tbody>
<tr><th>#</th><th> </th><th>讨论区名称</th><th>类别</th><th>中文描述</th><th>版主</th></tr>
<?php
	# shift through the array
	$i = 0;
	while($board = array_shift($boards))
	{
		if ($board->node_type() == XML_TEXT_NODE)
			continue;
		$ename = find_content($board, $nodename);
		$brdnum = bbs_getboard($ename, $brdarr);
		if ($brdnum == 0)
			continue;
		$brd_encode = urlencode($brdarr["NAME"]);
		$i ++ ;
?>
<tr>
<td><?php echo $i; ?></td>
<td>
	<img src="images/newgroup.gif" />
</td>
<td>
	<a href="bbsdoc.php?board=<?php echo $brd_encode; ?>"><?php echo $brdarr["NAME"]; ?></a>
</td>
<td><?php echo $brdarr["CLASS"]; ?></td>
<td><a href="bbsdoc.php?board=<?php echo $brd_encode; ?>"><?php echo $brdarr["DESC"]; ?></a></td>
<td>
<?php
		$bms = explode(" ", trim($brdarr["BM"]));
		if (strlen($bms[0]) == 0 || $bms[0][0] <= chr(32))
			echo "诚征版主中";
		else
		{
			if (!ctype_print($bms[0][0]))
				echo $bms[0];
			else
			{
?>
<a href="bbsqry.php?userid=<?php echo $bms[0]; ?>"><?php echo $bms[0]; ?></a>
<?php
			}
		}
?>
</td></tr>
<?php
	} //end while
?>
</tbody></table>
<?php
	bbs_boards_navigation_bar();
	page_footer();
?>
