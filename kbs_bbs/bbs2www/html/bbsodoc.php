<?php
	/**
	 * This file lists articles to user.
	 * $Id$
	 */
	require("funcs.php");
login_init();
	require("board.inc.php");
	
	function display_navigation_bar($brdarr,$brdnum,$start,$total,$page,$order=FALSE)
	{
		global $section_names,$currentuser;
		$brd_encode = urlencode($brdarr["NAME"]);
	?>
<table width="100%" border="0" cellspacing="0" cellpadding="3" class="b1">		
<form name="form1" action="<?php echo $_SERVER["PHP_SELF"]; ?>" method="get">
<input type="hidden" name="board" value="<?php echo $brdarr["NAME"]; ?>"/>
<tbody><tr>
<td>
<a href="bbspst.php?board=<?php echo $brd_encode; ?>"><img src="images/postnew.gif" border="0" alt="发表话题"></a>
</td>
<td align="right">
 	<?php
		      if($order)
		      {
		   	if ($start <= $total - 20)
			{
		    ?>
[<a class="b1" href="<?php echo $_SERVER["PHP_SELF"]; ?>?board=<?php echo $brd_encode; ?>"><font class="b1"><u>第一页</u></font></a>]
[<a class="b1" href="<?php echo $_SERVER["PHP_SELF"]; ?>?board=<?php echo $brd_encode; ?>&page=<?php echo $page + 1; ?>"><font class="b1"><u>上一页</u></font></a>]
		    <?php
			}
			else
			{
		    ?>
[第一页] 
[上一页]
		    <?php
			}
			if ($page > 1)
			{
		?>
[<a class="b1" href="<?php echo $_SERVER["PHP_SELF"]; ?>?board=<?php echo $brd_encode; ?>&page=<?php echo $page - 1; ?>"><font class="b1"><u>下一页</u></font></a>]
[<a class="b1" href="<?php echo $_SERVER["PHP_SELF"]; ?>?board=<?php echo $brd_encode; ?>&page=1"><font class="b1"><u>最后一页</u></font></a>]
		    <?php
			}
			else
			{
		    ?>
[下一页] 
[最后一页]
		    <?php
			}
		     }
		     else
		     {	
		     	if ($page > 1)
			{
		?>
[<a class="b1" href="<?php echo $_SERVER["PHP_SELF"]; ?>?board=<?php echo $brd_encode; ?>&page=1"><font class="b1"><u>第一页</u></font></a>]
[<a class="b1" href="<?php echo $_SERVER["PHP_SELF"]; ?>?board=<?php echo $brd_encode; ?>&page=<?php echo $page - 1; ?>"><font class="b1"><u>上一页</u></font></a>]
		    <?php
			}
			else
			{
		    ?>
[第一页] 
[上一页]
		    <?php
			}
			if ($start <= $total - 20)
			{
		    ?>
[<a class="b1" href="<?php echo $_SERVER["PHP_SELF"]; ?>?board=<?php echo $brd_encode; ?>&page=<?php echo $page + 1; ?>"><font class="b1"><u>下一页</u></font></a>]
[<a class="b1" href="<?php echo $_SERVER["PHP_SELF"]; ?>?board=<?php echo $brd_encode; ?>"><font class="b1"><u>最后一页</u></font></a>]
		    <?php
			}
			else
			{
		    ?>
[下一页] 
[最后一页]
		    <?php
			}
		    }
	?>
<input type="submit" class="b5" value="跳转到"/> 第 <input type="text" name="start" size="3"  onmouseover=this.focus() onfocus=this.select() class="b5"> 篇 
</td></tr></tbody></form></table>
	<?php
	}


	function display_articles($brdarr,$articles,$start,$order=FALSE)
	{
		global $dir_modes;
		global $default_dir_mode;
		$brd_encode = urlencode($brdarr["NAME"]);
?>
<table width="100%" border="0" cellspacing="0" cellpadding="3" class="t1">
<col width="50px"/><col width="30px"/><col width="85px"/><col width="50px"/><col width="*" align="left"/>
<tbody><tr><td class="t2">序号</td><td class="t2">标记</td><td class="t2">作者</td><td class="t2">日期</td><td class="t2">标题</td></tr>
</tbody>
<?php
		$ding_cnt = 0;
		foreach ($articles as $article)
		{
			$flags = $article["FLAGS"];
			if (!strncmp($flags,"D",1)||!strncmp($flags,"d",1))
				$ding_cnt++;
		}
		$i = 0;
		if ($order) {
			$articles = array_reverse($articles);
			$i = count($articles) - $ding_cnt - 1;
		}
		foreach ($articles as $article)
		{
			$title = $article["TITLE"];
			if (strncmp($title, "Re: ", 4) != 0)
				$title = "● " . $title;

			$flags = $article["FLAGS"];
?>
<tbody>
<tr>
<?php
			if (!strncmp($flags,"D",1)||!strncmp($flags,"d",1)) {
?>
<td class="t6">提示</td>
<td class="t3"><img src="images/istop.gif" alt="提示" align="absmiddle"></td><?php

			} else {
?>
<td class="t3"><?php echo $start+$i; ?></td>
<td class="t4">
<?php
			if ($flags[1] == 'y')
			{
				if ($flags[0] == ' ')
					echo "&nbsp;";
				else
					echo $flags[0];
			}
                         elseif ($flags[0] == 'N' || $flags[0] == '*'){
                                 if ($flags[0] == ' ') 
                                         echo "&nbsp;"; 
                                 else
                                         echo "&nbsp;"; //$flags[0];  //不要未读标记 windinsn
                         }
                         else{
                                 if ($flags[0] == ' ')
                                         echo "&nbsp;"; 
                                 else
                                         echo $flags[0];
                         }   
                         echo $flags[3]; 
 ?> 
</td>
<?php
	}//置顶
?>
<td class="t3"><a class="ts1" href="/bbsqry.php?userid=<?php echo $article["OWNER"]; ?>"><?php echo $article["OWNER"]; ?></a></td>
<td class="t4"><?php echo strftime("%b&nbsp;%e", $article["POSTTIME"]); ?></td>
<td class="t5"><strong>
<?php
	switch ($default_dir_mode)
	{
	case $dir_modes["ORIGIN"]:
		if (!strncmp($flags,"D",1)||!strncmp($flags,"d",1))
		{
?>
<a href="/bbscon.php?board=<?php echo $brd_encode; ?>&id=<?php echo $article["ID"]; ?>&ftype=<?php echo $dir_modes["ZHIDING"]; ?>"><?php echo htmlspecialchars($title); ?>

</a></strong>
<?php
		}
		else
		{
?>
<a href="/bbstcon.php?board=<?php echo $brd_encode; ?>&gid=<?php echo $article["GROUPID"]; ?>"><?php echo htmlspecialchars($title); ?> <font class="<?php if($article["EFFSIZE"] >= 1000) echo "mb2"; else echo "b1";?>">(<?php if($article["EFFSIZE"] < 1000) echo $article["EFFSIZE"]; else { printf("%.1f",$article["EFFSIZE"]/1000.0); echo "k";} ?>)</font>

</a></strong>
<?php
		}
		break;
	case $dir_modes["NORMAL"]:
	default:
?>
<a href="/bbscon.php?board=<?php echo $brd_encode; ?>&id=<?php echo $article["ID"]; ?><?php if (!strncmp($flags,"D",1)||!strncmp($flags,"d",1)) echo "&ftype=" . $dir_modes["ZHIDING"]; ?>"><?php echo htmlspecialchars($title); ?> </a></strong><font class="<?php if($article["EFFSIZE"] >= 1000) echo "mb2"; else echo "b1";?>">(<?php if($article["EFFSIZE"] < 1000) echo $article["EFFSIZE"]; else { printf("%.1f",$article["EFFSIZE"]/1000.0); echo "k";} ?>)</font>

<?php
	}
?>
</td>
</tr>
</tbody>
<?php
			if ($order)
				$i--;
			else
				$i++;
		}
?>
</table>
<?php
	}

	if ($loginok != 1)
		html_nologin();
	else
	{
		$board_list_error=FALSE;
		if (isset($_GET["board"]))
			$board = $_GET["board"];
		else{
		        html_init("gb2312","","",1);
			html_error_quit("错误的讨论区");
			$board_list_error=TRUE;
			}
		// 检查用户能否阅读该版
		$brdarr = array();
		$brdnum = bbs_getboard($board, $brdarr);
		if ($brdnum == 0){
		        html_init("gb2312","","",1);
			html_error_quit("错误的讨论区");
			$board_list_error=TRUE;
		}
		$board = $brdarr["NAME"];
		$usernum = $currentuser["index"];
		if (bbs_checkreadperm($usernum, $brdnum) == 0){
		        html_init("gb2312","","",1);
			html_error_quit("错误的讨论区");
			$board_list_error=TRUE;
			}
		if ($brdarr["FLAG"]&BBS_BOARD_GROUP) {
			for ($i=0;$i<sizeof($section_nums);$i++)
				if (!strcmp($section_nums[$i],$brdarr["SECNUM"])) {
			         Header("Location: bbsboa.php?group=" . $i . "&group2=" . $brdnum);
			         return;
                                }
		        html_init("gb2312","","",1);
			html_error_quit("错误的讨论区");
			$board_list_error=TRUE;
		}
		
		$brd_encode = urlencode($brdarr["NAME"]);
		
		/* BBS Board Envelop Code START
		** add by windinsn , Mar 13 ,2004 */
		if( defined("HAVE_BRDENV") && !isset($_GET["env"]))
		{
			if( bbs_board_have_envelop($board))
			{
				if( !stristr($_COOKIE["BBSVISITEDBRD"],"|".$board."|") )
				{
					setcookie("BBSVISITEDBRD",$_COOKIE["BBSVISITEDBRD"]."|".$board."|",time()+360000,"");
					header("Location: /bbsenv.php?board=".$brd_encode);
				}
			}
		}	
		/* BBS Board Envelop Code END */

		if (!isset($default_dir_mode))
			$default_dir_mode = $dir_modes["ORIGIN"];
                $isnormalboard=bbs_normalboard($board);

        	bbs_set_onboard($brdnum,1);
        	bbs_checkorigin($brdarr["NAME"]);
		if ($isnormalboard) {
                        $dotdirname=BBS_HOME . "/boards/" . $brdarr["NAME"] . "/.DIR";
       			if (cache_header("public, must-revalidate",filemtime($dotdirname),10))
               			return;
               	}
		html_init("gb2312","","",1);
		$total = bbs_countarticles($brdnum, $default_dir_mode);
		if ($total <= 0)
		    if (strcmp($currentuser["userid"], "guest") != 0){
			html_error_quit("本讨论区目前没有文章<br /><a href=\"bbspst.php?board=" . $board . "\">发表文章</a>");
			$board_list_error=TRUE;
			}
                    else{
			html_error_quit("本讨论区目前没有文章");
			$board_list_error=TRUE;
			}

		$artcnt = 20;
		if (isset($_GET["page"]))
			$page = $_GET["page"];
		elseif (isset($_POST["page"]))
			$page = $_POST["page"];
		else
		{
			if (isset($_GET["start"]))
			{
				$start = $_GET["start"];
				settype($start, "integer");
				$page = ($start + $artcnt - 1) / $artcnt;
			}
			else
				$page = 0;
		}
		settype($page, "integer");
		if ($page > 0)
			$start = ($page - 1) * $artcnt + 1;
		else
			$start = 0;
		/*
		 * 这里存在一个时间差的问题，可能会导致序号变乱。
		 * 原因在于两次调用 bbs_countarticles() 和 bbs_getarticles()。
		 */
		if ($start == 0 || $start > ($total - $artcnt + 1))
		{
			if ($total <= $artcnt)
			{
				$start = 1;
				$page = 1;
			}
			else
			{
				$start = ($total - $artcnt + 1);
				$page = ($start + $artcnt - 1) / $artcnt + 1;
			}
		}
		else
			$page = ($start + $artcnt - 1) / $artcnt;
		settype($page, "integer");
		$articles = bbs_getarticles($brdarr["NAME"], $start, $artcnt, $default_dir_mode);
		if ($articles == FALSE){
			html_error_quit("读取文章列表失败");
			$board_list_error=TRUE;
			}
		if (!isset($order_articles))
			$order_articles = FALSE;
			
	if($board_list_error==FALSE)
	{		
		bbs_board_header($brdarr,$total);
		display_navigation_bar($brdarr, $brdnum, $start, $total, $page,$order_articles );
		display_articles($brdarr, $articles, $start, $order_articles );
		display_navigation_bar($brdarr, $brdnum, $start, $total, $page,$order_articles);
		bbs_board_foot($brdarr,"ORIGIN");
	}
	html_normal_quit();
}
?>
