<?php

	require("funcs.php");
	if ($loginok != 1 || ($currentuser["userid"] == "guest") )
		html_nologin();
	else
	{
		html_init("gb2312");

		if(isset($_GET["board"]))
			$board = $_GET["board"];
		else if(isset($_POST["board"]))
			$board = $_POST["board"];
		else
			html_error_quit("讨论区错误");

		if(isset($_GET["num"]))
			$num = $_GET["num"];
		else if(isset($_POST["num"]))
			$num = $_POST["num"];
		else
			html_error_quit("参数错误1");

		$brdarr = array();
		$brdnum = bbs_getboard($board,$brdarr);
		if($brdnum == 0)
			html_error_quit("错误的讨论区");

		if(bbs_checkreadperm($currentuser["index"],$brdnum)==0)
			html_error_quit("您没有权限");

		$oldvote = array();
		$votearr = array();
		$uservotearr = array();

		for( $i=0; $i<32; $i++){
			$oldvote[$i] = 0;
		}

		if( $_POST["submit"] ){

			if(isset($_GET["type"]))
				$votetype = $_GET["type"];
			else if(isset($_POST["type"]))
				$votetype = $_POST["type"];
			else
				html_error_quit("参数错误2");

			if(isset($_POST["msg"]))
				$msg = $_POST["msg"];
			else
				$msg = "";

			$votevalueint = 0;

			if( $votetype == "单选" || $votetype == "是非" ){
				if(isset($_POST["ITEM"]))
					$itemvalue = $_POST["ITEM"];
				else
					html_error_quit("参数错误3");

				settype($itemvalue,"integer");
				if( $itemvalue < 0 || $itemvalue > 31 )
					html_error_quit("参数错误4");

				if( $votetype == "是非" && ($itemvalue < 0 || $itemvalue > 2) )
					html_error_quit("参数错误5");

				$votevalueint = ( 1 << $itemvalue );

			}else if( $votetype == "复选" ){

				for($i = 0; $i < 32; $i++){
					$itemstr = "ITEM".($i+1);
					if(isset($_POST[$itemstr]) && $_POST[$itemstr]=="on"){
						$votevalueint += ( 1 << $i );
					}
				}

			}else if( $votetype == "数字" ){
				if(isset($_POST["ITEM"]))
					$votevalueint = $_POST["ITEM"];
				else
					html_error_quit("参数错误3");

				settype($votevalueint,"integer");

				if( $votevalueint < 0 )
					html_error_quit("参数错误7");
			}else if( $votetype != "问答" )
				html_error_quit("参数错误8");

			$retnum = bbs_vote_num($board,$num,$votevalueint,$msg);
			if($retnum <= 0)
				html_error_quit("投票错误".$retnum);
			else{
?>
投票成功
<br>
<a href="javascript:history.go(-1)">快速返回</a>
<?php
				html_normal_quit();

			}
		}

		$retnum = bbs_get_vote_from_num($board,$votearr,$num,$uservotearr);

		if($retnum <= 0)
			html_error_quit("该投票不存在");
?>
<body>
<center><p><?php echo BBS_FULL_NAME; ?> -- [投票列表] [用户:<?php echo $currentuser["userid"];?>] 
<?php echo $board; ?>版投票<br></p>
<hr class="default"/>
</center>
<?php
		$descdir = "vote/".$board."/desc.".$votearr[0]["DATE"] ;
		echo bbs_printansifile($descdir);
?>
<center>
<hr class="default"/>
<table width="613">
<tr><td>序号</td><td><?php echo $num;?></tr>
<tr><td>标题</td><td><?php echo $votearr[0]["TITLE"];?></tr>
<tr><td>类型</td><td><?php echo $votearr[0]["TYPE"];?></tr>
<tr><td>开启者</td><td><?php echo $votearr[0]["USERID"];?></tr>
<tr><td>开启日期</td><td><?php echo date("r",$votearr[0]["DATE"]);?></tr>
<tr><td>投票天数</td><td><?php echo $votearr[0]["MAXDAY"];?></tr>
<tr><td><?php if($uservotearr[0]["USERID"]){?>您已经投票，现在可以更改<?php }else{?>您尚未投票<?php }?></td><td></td></tr>
</table>
<hr class="default"/>
<form action="/bbsvote.php?board=<?php echo $board;?>&num=<?php echo $num?>" method="post">
<table width="613">
<?php
		if($uservotearr[0]["USERID"]){
			if( $votearr[0]["TYPE"] != "数字" ){
				for( $i =0; $i < $votearr[0]["TOTALITEMS"]; $i++){
					if( $uservotearr[0]["VOTED"] & (1 << $i) )
						$oldvote[$i] = 1;
				}
			}
		}
		if( $votearr[0]["TYPE"] == "复选" ){

			for( $i=0; $i < $votearr[0]["TOTALITEMS"]; $i++){
				$itemstr = "ITEM".($i+1);
?>
<tr><td><?php echo $i+1;?></td>
<td><input type="checkbox" name="<?php echo $itemstr;?>" <?php if($oldvote[$i]) echo "checked";?>></td>
<td><?php echo $votearr[0][$itemstr];?></td></tr>
<?php
			}
?>
<input type="hidden" name="type" value="复选">
<?php
		}else if( $votearr[0]["TYPE"] == "单选" || $votearr[0]["TYPE"] == "是非" ){

			for( $i=0; $i < $votearr[0]["TOTALITEMS"]; $i++){
				$itemstr = "ITEM".($i+1);
?>
<tr><td><?php echo $i+1;?></td>
<td><input type="radio" name="ITEM" value="<?php echo $i;?>" <?php if($oldvote[$i]) echo "checked";?>></td>
<td><?php echo $votearr[0][$itemstr];?></td></tr>
<?php
			}
?>
<input type="hidden" name="type" value="<?php echo $votearr[0]["TYPE"];?>">
<?php
		}else if( $votearr[0]["TYPE"] == "数字" ){
?>
<tr><td></td><td></td><td>请输入数值，最大<?php echo $votearr[0]["MAXTKT"];?>:
<input type="text" name="ITEM" value="<?php echo $uservotearr[0]["VOTED"];?>"></td>
</tr>
<input type="hidden" name="type" value="<?php echo $votearr[0]["TYPE"];?>">
<?php
		}else{
?>
<input type="hidden" name="type" value="<?php echo $votearr[0]["TYPE"];?>">
<?php
		}
?>
<tr><td></td><td></td><td>请留下您的建议(限制3行80列)</td></tr>
<tr><td></td><td></td><td><textarea name="msg" rows="3" cols="79" wrap="physical">
<?php echo $uservotearr[0]["MSG1"]; echo $uservotearr[0]["MSG2"]; echo $uservotearr[0]["MSG3"];?>
</textarea></td></tr>
</table>
<input type="submit" name="submit" value="确认">
</form>
<hr class="default"/>
<a href="/bbsshowvote.php?board=<?php echo $board;?>">[查看本版所有投票]</a>
<a href="/bbsdoc.php?board=<?php echo $board;?>">[返回本讨论区]</a>
<a href="javascript:history.go(-1)">[快速返回]</a>
</center>
<?php
		html_normal_quit();

	}
?>
