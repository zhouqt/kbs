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

		if(isset($_GET["num"]))
			$num = $_GET["num"];
		else
			html_error_quit("参数错误2");

		if($num <= 0)
			html_error_quit("参数错误3");

		$brdarr = array();
		$brdnum = bbs_getboard($board,$brdarr);
		if($brdnum == 0)
			html_error_quit("错误的讨论区");

		if(bbs_checkreadperm($currentuser["index"],$brdnum)==0)
			html_error_quit("您没有权限");

		$votearr = array();
		$retnum = bbs_get_tmpl_from_num($board,$num,$votearr);

		if( $retnum <= 0 )
			html_error_quit("错误");
?>
<body>
<center><p><?php echo BBS_FULL_NAME; ?> -- [摸板详细显示] [用户:<?php echo $currentuser["userid"];?>] 
版面<?php echo $board; ?>第<?php echo $retnum;?>个摸板<br></p>
<hr class="default"/>
<table width="613">
<tr>
<td>摸板说明</td><td><?php echo $votearr[0]["TITLE"];?></td>
</tr>
<tr>
<td>问题个数</td><td><?php echo $votearr[0]["CONT_NUM"];?></td>
</tr>
<tr>
<td>标题格式</td><td><?php echo $votearr[0]["TITLE_TMPL"];?></td>
</tr>
</table>
<hr class="default"/>
<?php
		if( $votearr[0]["FILENAME"] != "" ){
?>
正文格式如下:<br>
</center>
<?php
			echo bbs_printansifile($votearr[0]["FILENAME"]);
?>
<center>
<hr class="default"/>
<?php
		}
?>
<table width="613">
<tr><td>问题序号</td><td>问题</td><td>回答长度</td></tr>
<?php
		for($i = 0; $i < $votearr[0]["CONT_NUM"]; $i++ ){
?>
<tr><td>
<?php echo $i+1;?>
</td><td>
<?php echo $votearr[$i+1]["TEXT"];?>
</td><td>
<?php echo $votearr[$i+1]["LENGTH"];?>
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
