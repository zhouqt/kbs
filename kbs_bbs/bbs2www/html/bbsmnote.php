<?php
	/**
	 * $Id$ 
	 */
	require("funcs.php");
	if ($loginok !=1 )
		html_nologin();
	else
	{
		html_init("gb2312");
                $usernum = $currentuser["index"];
		if (isset($_GET["board"]))
			$board = $_GET["board"];
		else 
			html_error_quit("错误的讨论区");
                $brdarr = array();
                $brdnum = bbs_getboard($board, $brdarr);
                if ($brdnum == 0)
			html_error_quit("错误的讨论区");
		if (!bbs_is_bm($brdnum,$usernum))
			html_error_quit("你不是版主" . $brdnum . "!!" . $usernum);
		$top_file="vote/" . $board . "/notes";
		if ($_GET["type"]=="update") {
			$fp = fopen($top_file, "w");
			if ($fp==FALSE) {
				html_error_quit("无法打开文件");
			} else {
				$data=$_POST["text"];
				fwrite($fp,str_replace("\r\n","\n",$data));
				fclose($fp);
?>
<body>
<center>
修改备忘录成功! <br />
[<a href=bbsdoc.php?board=<?php echo $board; ?>>本讨论区</a>]
</center>
</body>
<?php
			}
		} else {
			$fp = fopen($top_file, "r");
?>
<body>
<center><?php echo BBS_FULL_NAME; ?> -- 备忘录 [讨论区: <?php echo $board; ?>]<hr color=green>
<form method="post" action=<?php echo "\"bbsmnote?type=update&board=" . $board . "\""; ?>>
<table width="610" border="1"><tr><td><textarea name="text" rows="20" cols="80" wrap="physical">
<?php
			if ($fp != FALSE) {
				for ($i=0;!feof($fp)&&$i<200;$i++)
					echo(fgets($fp,256));
				fclose($fp);
	                }
?> 
</textarea></td></tr></table>
<input type="submit" value="存盘"> <input type="reset" value="复原">
</form>
<hr>
[<a href=bbsdoc.php?board=<?php echo $board; ?>>本讨论区</a>]
</center>
</body>
<?php
        	}
		html_normal_quit();
	}
?>
