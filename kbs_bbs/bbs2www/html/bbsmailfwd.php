<?php
	require("funcs.php");
	if ($loginok != 1)
		html_nologin();
	else
	{
		html_init("gb2312");
        if ($currentuser["userid"]=="guest")
            html_error_quit("匆匆过客不能转寄文章");

		if( !isset($_GET["dir"]) && !isset($_POST["dir"]))
			html_error_quit("错误的信箱");
		if( isset($_GET["dir"]) )
			$dirname = $_GET["dir"];
		else
			$dirname = $_POST["dir"];

		if( !isset($_GET["id"]) && !isset($_POST["id"]))
			html_error_quit("错误的文章号");
		if( isset($_GET["id"]) )
			$num = $_GET["id"];
		else
			$num = $_POST["id"];
		settype($num, "integer");

    	if (strstr($dirname, "..") || strstr($dirname, "/")){
			html_init("gb2312");
			html_error_quit("错误的参数2");
		}
		$dir = "mail/".strtoupper($currentuser["userid"]{0})."/".$currentuser["userid"]."/".$dirname ;

		$total = filesize( $dir ) / 256 ;
		if( $total <= 0 ){
			html_init("gb2312");
			html_error_quit("信笺不存在");
		}
		if( $num >= $total ){
			html_init("gb2312");
			html_error_quit("信笺不存在1");
		}

		$articles = array ();
		if( bbs_get_records_from_num($dir, $num, $articles) ) {
			$file = $articles[0]["FILENAME"];
		}else{
			html_init("gb2312");
			html_error_quit("错误的参数4");
		}

		$filename = "mail/".strtoupper($currentuser["userid"]{0})."/".$currentuser["userid"]."/".$file ;
		if(! file_exists($filename)){
			html_init("gb2312");
			html_error_quit("信件不存在...");
		}
?>
<center>
<?php echo BBS_FULL_NAME; ?> -- 转寄文章 [使用者: <?php echo $currentuser["userid"];?>]
<hr color="green"><br>
</center>
<?php
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

		$ret = bbs_domailforward($filename, $articles[0]["TITLE"], $target, $big5, $noansi);
		if($ret < 0)
			html_error_quit("系统错误:".$ret);
?>
信件已转寄给'<?php echo $target;?>'<br>
[<a href="javascript:history.go(-1)">返回</a>]
<?php
	}
?>
