<?php
	/**
	 * This file lists boards rank to user.
	 * windinsn Apr 8 , 2004
	 */
	require("funcs.php");
login_init();
	require("board.inc.php");

	if ($loginok !=1 )
		html_nologin();
	else
	{
		$boardrank_file = BBS_HOME . "/xml/board.xml";
		if (cache_header("public, must-revalidate",filemtime($boardrank_file),3600))
               		return;
		$doc = domxml_open_file($boardrank_file);
		if (!$doc)
			html_error_quit("目前尚无人气排名信息");
			
		$root = $doc->document_element();
		$boards = $root->child_nodes();
		$brdarr = array();
		
		html_init("gb2312");
		
?>
<body topmargin="0">
<table width="100%" border="0" cellspacing="0" cellpadding="3" >
  <tr> 
    <td colspan="2" class="kb2" colspan=2>
	    <a class="kts1"  href="<?php echo MAINPAGE_FILE; ?>"><?php echo BBS_FULL_NAME; ?></a>  - <a class="kts1"  href="bbssec.php">分类讨论区</a> - [讨论区人气排名]    </td>
  </tr>
   <tr valign=bottom align=center> 
    <td align="left" class="kb4">&nbsp;&nbsp;&nbsp;&nbsp; 讨论区人气排名</td>
     <td align="right" class="kb1" >&nbsp;</td>
  </tr>
   <tr> 
    <td colspan="2" height="9" background="images/dashed.gif"> </td>
  </tr>
   <tr><td colspan="2" align="center">
    	<table width="100%" border="0" cellspacing="0" cellpadding="3" class="kt1">	
<form name="form1" action="bbsdoc.php" method="get">
<input type="hidden" name="board" value="Botany">
<tr>
<td class="kt2" width="5%"> </td>
<td class="kt2" width="2%"> </td>
<td class="kt2" width="24%">讨论区名称</td>
<td class="kt2" width="10%">类别</td>
<td class="kt2" width="41%">中文描述</td>
<td class="kt2" width="18%">版主</td>
</tr>
<?php
	# shift through the array
	while($board = array_shift($boards))
	{
		if ($board->node_type() == XML_TEXT_NODE)
			continue;
		$ename = find_content($board, "EnglishName");
		$brdnum = bbs_getboard($ename, $brdarr);
		if ($brdnum == 0)
			continue;
		$brd_encode = urlencode($brdarr["NAME"]);
		$i ++ ;
?>
<tr>
<td class="kt3 c2" align=center height=25><?php echo $i; ?></td>
<td class="kt4 c1" >
	<img src="images/newgroup.gif" height="15" width="20" title="讨论区人气排名">
</td>
<td class="kt3 c1">
	<a class="kts1"  href="/bbsdoc.php?board=<?php echo $brd_encode; ?>"><?php echo $brdarr["NAME"]; ?></a>
</td>
<td class="kt3 c2" align="center">
	<?php echo $brdarr["CLASS"]; ?>
</td>
<td class="kt3 c1">
&nbsp;&nbsp;<a class="kts1" href="/bbsdoc.php?board=<?php echo $brd_encode; ?>"><?php echo $brdarr["DESC"]; ?></a>
</td>
<td class="kt3 c2" align="center">
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
<a class="kts1" href="/bbsqry.php?userid=<?php echo $bms[0]; ?>"><?php echo $bms[0]; ?></a>
<?php
				}
			}
		} //end while
?>
</table>
   <tr> 
    <td colspan="2" height="9" background="images/dashed.gif"> </td>
  </tr>
  </table>
</center>
<?php
		bbs_boards_navigation_bar();
		html_normal_quit();
	}
?>
