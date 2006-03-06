<?php
	require("www2-funcs.php");
	login_init();

	function display_navigation_bar($brdarr,$brdnum,$start,$total,$order)
	{
		$brd_encode = urlencode($brdarr["NAME"]);
?>
<form action="<?php echo $_SERVER["PHP_SELF"]; ?>" method="get">	
<div class="fright">
<?php
			  if($order)
			  {
			if ($start <= $total - 20)
			{
			?>
[<a href="<?php echo $_SERVER["PHP_SELF"]; ?>">第一页</a>]
[<a href="<?php echo $_SERVER["PHP_SELF"]; ?>?start=<?php echo $start + 20; ?>">上一页</a>]
			<?php
			}
			else
			{
			?>
[第一页] 
[上一页]
			<?php
			}
			if ($start > 1)
			{
		?>
[<a href="<?php echo $_SERVER["PHP_SELF"]; ?>?start=<?php if($start>20) echo $start - 20; else echo "1";?>">下一页</a>]
[<a href="<?php echo $_SERVER["PHP_SELF"]; ?>?start=1">最后一页</a>]
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
				if ($start > 1)
			{
		?>
[<a href="<?php echo $_SERVER["PHP_SELF"]; ?>?start=1">第一页</a>]
[<a href="<?php echo $_SERVER["PHP_SELF"]; ?>?start=<?php if($start > 20) echo $start - 20; else echo "1"; ?>">上一页</a>]
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
[<a href="<?php echo $_SERVER["PHP_SELF"]; ?>?start=<?php echo $start + 20; ?>">下一页</a>]
[<a href="<?php echo $_SERVER["PHP_SELF"]; ?>">最后一页</a>]
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
<input type="submit" value="跳转到"/> 第 <input onmouseover="this.focus()" onfocus="this.select()" type="text" name="start" size="3"> 篇 
</div></form>
<?php
}
	
	
	function display_g_articles($brdarr,$articles,$start,$order=FALSE){
?>
<table class="main wide adj">
<caption>推荐文章</caption>
<col width="5%" class="center"/><col width="55%"/><col width="15%" class="center"/><col width="15%" class="center"/><col width="10%" class="center"/>
<tbody>
<tr><th>#</th><th>标题</th><th>推荐人</th><th>版面</th><th> </th></tr>
<?php
		$brd_encode = urlencode($brdarr["NAME"]);
		$i = 0;
//		foreach ($articles as $article)
for( ; $i < 20; $i++)
		{ if (!($article = @$articles[19-$i])) continue;
			$title = $article["TITLE"];
			if (strncmp($title, "Re: ", 4) != 0)
				$title = "● " . $title;
			$flags = $article["FLAGS"];
?>
<tr>
<td><?php echo $start +19- $i; ?></td>
<td><a href="bbsrecon.php?id=<?php echo $article["ID"]; ?>"><?php echo htmlspecialchars($title); ?></a></td>
<td><a href="bbsqry.php?userid=<?php echo $article["OWNER"]; ?>"><?php echo $article["OWNER"]; ?></a></td>
<td><a href="bbsdoc.php?board=<?php echo $article["O_BOARD"];?>"><?php { $brddarr = array(); if(bbs_getboard($article["O_BOARD"], $brddarr)) echo $brddarr["DESC"]; }?></a></td>
<td><a href="bbscon.php?bid=<?php echo $article["O_BID"];?>&id=<?php echo $article["O_ID"];?>">阅读原文</a></td>
</tr>
<?php
		}
?>
</table>
<?php		
	}

	$board = "Recommend";
	$ftype = $dir_modes["DIGEST"];
	$brdarr = array();
	$order_articles = TRUE;
	$brdnum = bbs_getboard($board, $brdarr);
	if ($brdnum == 0)
		html_error_quit("系统错误");
	$total = bbs_countarticles($brdnum, $ftype);
	if ($total <= 0)
		html_error_quit("目前没有文章");

	$dotdirname = bbs_get_board_index($board, $ftype);
	if (cache_header("public",@filemtime($dotdirname),300))
		return;

	if (isset($_GET["start"]))
		$start = $_GET["start"];
	elseif (isset($_POST["start"]))
		$start = $_POST["start"];
	else
		$start = 0;
	settype($start, "integer");
	$artcnt = 20;
	/*
	 * 这里存在一个时间差的问题，可能会导致序号变乱。
	 * 原因在于两次调用 bbs_countarticles() 和 bbs_getarticles()。
	 */
	if ($start == 0 || $start > ($total - $artcnt + 1))
		$start = ($total - $artcnt + 1);
	if ($start < 0)
		$start = 1;
	$articles = bbs_getarticles($board, $start, $artcnt, $dir_modes["DIGEST"]);
	if ($articles == FALSE)
		html_error_quit("读取文章列表失败");

	$brd_encode = urlencode($brdarr["NAME"]);
	page_header("推荐文章");

	display_g_articles($brdarr,$articles,$start,$order=FALSE);
	display_navigation_bar($brdarr, $brdnum, $start, $total, $order_articles);

	page_footer();
?>
