<?php
	require("funcs.php");
	if ($loginok != 1)
		html_nologin();
	else
	{
		html_init("gb2312");
        if ($currentuser["userid"]=="guest")
            html_error_quit("匆匆过客不能转寄文章");

		if( !isset($_GET["board"]) && !isset($_POST["board"]))
			html_error_quit("错误的讨论区");
		if( isset($_GET["board"]) )
			$board = $_GET["board"];
		else
			$board = $_POST["board"];

		$brdarr = array();
		$brdnum = bbs_getboard($board, $brdarr);
		if ($brdnum == 0){
			html_error_quit("错误的讨论区1");
		}
		$usernum = $currentuser["index"];
		if (bbs_checkreadperm($usernum, $brdnum) == 0){
			html_error_quit("错误的讨论区2");
		}

		if( !isset($_GET["id"]) && !isset($_POST["id"]))
			html_error_quit("错误的文章号");
		if( isset($_GET["id"]) )
			$id = $_GET["id"];
		else
			$id = $_POST["id"];

		$articles = array ();
		$num = bbs_get_records_from_id($brdarr["NAME"], $id, 
			$dir_modes["NORMAL"] , $articles);
		if($num == 0)
			html_error_quit("错误的文章号");
?>
<center>
<?php echo BBS_FULL_NAME; ?> -- 转寄文章 [使用者: <?php echo $currentuser["userid"];?>]
<hr color="green"><br>
</center>
<?php
		if ( !isset($_POST["submit"])){
?>
文章标题: <?php echo htmlspecialchars($articles[1]["TITLE"]);?><br>
文章作者: <?php echo $articles[1]["OWNER"];?><br>
原讨论区: <?php echo $brdarr["NAME"];?><br>
<form action="/bbsfwd.php" method="post">
<input type="hidden" name="board" value="<?php echo $brdarr["NAME"];?>">
<input type="hidden" name="id" value="<?php echo $id;?>">
把文章转寄给 <input type="text" name="target" size="40" maxlength="69" value="<?php echo $currentuser["email"];?>"> (请输入对方的id或email地址). <br>
<input type="checkbox" name="big5" value="1"> 使用BIG5码<br>
<input type="checkbox" name="noansi" value="1" checked> 过滤ANSI控制符<br>
<input type="submit" name="submit" value="确定转寄">
</form>
<?php
			html_normal_quit();
		}

		if( isset($_POST["target"]) )
			$target =  $_POST["target"];
		else
			$target = "";
		if($target == "")
			html_error_quit("请指定对象");

		if( isset($_POST["big5"]) )
			$big5 = $_POST["big5"];
		else
			$big5=0;
		settype($big5, "integer");

		if( isset($_POST["noansi"]) )
			$noansi = $_POST["noansi"];
		else
			$noansi=0;
		settype($noansi, "integer");

		$ret = bbs_doforward($brdarr["NAME"], $articles[1]["FILENAME"], $articles[1]["TITLE"], $target, $big5, $noansi);
		if($ret < 0)
			html_error_quit("系统错误:".$ret);
?>
文章已转寄给'<?php echo $target;?>'<br>
[<a href="javascript:history.go(-2)">返回</a>]
<?php
	}
?>
