<?php

	require("funcs.php");
	if ($loginok != 1 || ($currentuser["userid"] == "guest") )
		html_nologin();
	else
	{
		html_init("gb2312");

		if(isset($_GET["board"]))
			$board = $_GET["board"];
		else
			html_error_quit("讨论区错误");

		if(isset($_GET["num"]))
			$num = $_GET["num"];
		else
			html_error_quit("参数错误1");

		$brdarr = array();
		$brdnum = bbs_getboard($board,$brdarr);
		if($brdnum == 0)
			html_error_quit("错误的讨论区");

		if(bbs_checkreadperm($currentuser["index"],$brdnum)==0)
			html_error_quit("您没有权限");

		$votearr = array();
		$retnum = bbs_get_vote_from_num($board,$votearr,$num);

		if($retnum <= 0)
			html_error_quit("该投票不存在");
?>
<body>
<center><p><?php echo BBS_FULL_NAME; ?> -- [投票列表] [用户:<?php echo $currentuser["userid"];?>] 
<?php echo $board; ?>版投票<br></p>
<hr class="default"/>
<?php
		$descdir = "vote/".$board."/desc.".$votearr[0]["DATE"] ;
		bbs_printansifile($descdir);
?>
<hr class="default"/>
<table width="613">
<tr><td>序号</td><td><?php echo $num;?></tr>
<tr><td>标题</td><td><?php echo $votearr[0]["TITLE"];?></tr>
<tr><td>类型</td><td><?php echo $votearr[0]["TYPE"];?></tr>
<tr><td>开启者</td><td><?php echo $votearr[0]["USERID"];?></tr>
<tr><td>开启日期</td><td><?php echo date("r",$votearr[0]["DATE"]);?></tr>
<tr><td>投票天数</td><td><?php echo $votearr[0]["MAXDAY"];?></tr>
</table>
<hr class="default"/>
<form action="/bbsvote.php" method="get">
<table width="613">
<?php
		if( $votearr[0]["TYPE"] == "复选" ){

			for( $i=0; $i < $votearr[0]["TOTALITEMS"]; $i++){
				$itemstr = "ITEM".($i+1);
?>
<tr><td><?php echo $i+1;?></td>
<td><input type="checkbox" name="<?php echo $itemstr;?>"></td>
<td><?php echo $votearr[0][$itemstr];?></td></tr>
<?php
			}
		}
?>
</table>
<input type="hidden" name="board" value="<?php echo $board;?>">
<input type="hidden" name="num" value="<?php echo $num;?>">
<input type="submit" name="submit" value="确认">
</form>
<?php
		html_normal_quit();

	}
?>
