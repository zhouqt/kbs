<?php

	require("funcs.php");
	if ($loginok != 1 || ($currentuser["userid"] == "guest") ){
		html_init("gb2312");
		html_error_quit("匿名用户没有发文权限");
	}else
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
		$retnum = bbs_get_tmpls($board,$votearr);

		if( $retnum < 0 )
			$retnum = 0;
?>
<body>
<center><p><?php echo BBS_FULL_NAME; ?> -- [摸板列表] [用户:<?php echo $currentuser["userid"];?>] 
版面<?php echo $board; ?>共有<?php echo $retnum;?>个摸板<br></p>
<hr class="default"/>
<table width="613">
<tr><td>序号</td><td>标题</td><td>类型</td><td>问题个数</td><td></td></tr>
<?php
		for($i = 0; $i < $retnum; $i++ ){
?>
<tr><td>
<?php echo $i+1;?>
</td><td>
<a href="/bbsatmpl.php?board=<?php echo $board;?>&num=<?php echo $i+1;?>"><?php echo $votearr[$i]["TITLE"];?></a>
</td><td>
<?php echo $votearr[$i]["TITLE"];?>
</td><td>
<?php echo $votearr[$i]["CONT_NUM"];?>
</td><td>
<a href="/bbspsttmpl.php?board=<?php echo $board;?>&num=<?php echo $i+1;?>">使用本摸板发文</a>
</td></tr>
<?php
		}
?>
</table>
<hr class="default"/>
<a href="/bbsdoc.php?board=<?php echo $board;?>">返回本讨论区</a>
<a href="javascript:history.go(-1)">快速返回</a>
</center>
<?php
		html_normal_quit();

	}
?>
