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

		$brdarr = array();
		$brdnum = bbs_getboard($board,$brdarr);
		if($brdnum == 0)
			html_error_quit("错误的讨论区");

		if(bbs_checkreadperm($currentuser["index"],$brdnum)==0)
			html_error_quit("您没有权限");

		$votearr = array();
		$retnum = bbs_get_votes($board,$votearr);

		if( $retnum < 0 )
			$retnum = 0;
?>
<body>
<center><p><?php echo BBS_FULL_NAME; ?> -- [投票列表] [用户:<?php echo $currentuser["userid"];?>] 
版面<?php echo $board; ?>共有<?php echo $retnum;?>个投票<br></p>
<hr class="default"/>
<table width="613">
<tr><td>序号</td><td>标题</td><td>类型</td><td>开启者</td><td>开启日期</td><td>投票天数</td></tr>
<?php
		for($i = 0; $i < $retnum; $i++ ){
?>
<tr><td>
<?php echo $i+1;?>
</td><td>
<a href="/bbsvote.php?board=<?php echo $board;?>&num=<?php echo $i+1;?>"><?php echo $votearr[$i]["TITLE"];?></a>
</td><td>
<?php echo $votearr[$i]["TYPE"];?>
</td><td>
<?php echo $votearr[$i]["USERID"];?>
</td><td>
<?php echo date("r",$votearr[$i]["DATE"]);?>
</td><td>
<?php echo $votearr[$i]["MAXDAY"];?>
</td></tr>
<?php
		}
?>
</table>
<?php
		html_normal_quit();

	}
?>
