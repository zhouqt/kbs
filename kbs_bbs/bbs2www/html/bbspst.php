<?php
	/**
	 * This file lists articles to user.
	 * $Id$
	 */
	require("funcs.php");
	require("boards.php");
	if ($loginok != 1)
		html_nologin();
	else
	{
		html_init("gb2312");
		if (isset($_GET["board"]))
			$board = $_GET["board"];
		else
			html_error_quit("错误的讨论区");
		// 检查用户能否阅读该版
		$brdarr = array();
		$brdnum = bbs_getboard($board, $brdarr);
		if ($brdnum == 0)
			html_error_quit("错误的讨论区");
		bbs_set_onboard($brcnum,1);
		$usernum = $currentuser["index"];
		if (bbs_checkreadperm($usernum, $brdnum) == 0)
			html_error_quit("错误的讨论区");
		if(bbs_checkpostperm($usernum, $brdnum) == 0) {
                    if (!strcmp($currentuser["userid"],"guest"))
		      html_error_quit("请先注册帐号");
                    else 
		      html_error_quit("错误的讨论区或者您无权在此讨论区发表文章");
                }
		if (bbs_is_readonly_board($brdarr))
			html_error_quit("不能在只读讨论区发表文章");
		if (isset($_GET["reid"]))
		{
			$reid = $_GET["reid"];
			if(!strcmp($board,"News") || !strcmp($board,"Original") || !strcmp($board,"AD_Agent"))
				html_error_quit("本版只可发表文章,不可回复文章!");
		}
		else {
			$reid = 0;
		}
		settype($reid, "integer");
		$articles = array();
		if ($reid > 0)
		{
			$num = bbs_get_records_from_id($brdarr["NAME"], $reid,$dir_modes["NORMAL"],$articles);
			if ($num == 0)
			{
				html_error_quit("错误的 Re 文编号");
			}
			if ($articles[1]["FLAGS"][2] == 'y')
				html_error_quit("该文不可回复!");
		}
		$brd_encode = urlencode($brdarr["NAME"]);
	}
?>
<body>
<center>
<?php echo BBS_FULL_NAME; ?> -- 发表文章 [使用者: <?php echo $currentuser["userid"]; ?>]
<hr class="default" />
<form name="postform" method="post" action="/cgi-bin/bbs/bbssnd?board=<?php echo $brd_encode; ?>&reid=<?php echo $reid; ?>">
<table border="1">
<tr>
<td>
<?php
		$notes_file = bbs_get_vote_filename($brdarr["NAME"], "notes");
		$fp = FALSE;
		if(file_exists($notes_file))
		{
		    $fp = fopen($notes_file, "r");
		    if ($fp == FALSE)
		    {
    	    	$notes_file = "vote/notes";
                if(file_exists($notes_file))
	    		    $fp = fopen($notes_file, "r");
    		}
		}
		if ($fp == FALSE)
    	{
?>
<font color="green">发文注意事项: <br />
发文时应慎重考虑文章内容是否适合公开场合发表，请勿肆意灌水。谢谢您的合作。<br/></font>
<?php
		}
        else
		{
		    fclose($fp);
			echo bbs_printansifile($notes_file);
		}
?>
</td>
</tr>
<tr><td>
作者: <?php echo $currentuser["userid"]; ?><br />
<?php
		if ($reid)
		{
	        if(!strncmp($articles[1]["TITLE"],"Re: ",4))$nowtitle = $articles[1]["TITLE"];
	        else
	            $nowtitle = "Re: " . $articles[1]["TITLE"];
?>
标题: <input type="text" name="title" size="40" maxlength="100" value="<?php echo $nowtitle; ?>

" /><br />
<?php
		}
		else
		{
?>
标题: <input type="text" name="title" size="40" maxlength="100" value=""><br />
<?php
		}
?>
看版: [<?php echo $brd_encode; ?>]<br />
<?php
		if (bbs_is_attach_board($brdarr))
		{
?>
附件: <input type="text" name="attachname" size="50" value="" disabled="disabled" />
<?php
		}
?>
使用签名档 <select name="signature">
<?php
		if ($currentuser["signature"] == 0)
		{
?>
<option value="0" selected="selected">不使用签名档</option>
<?php
		}
		else
		{
?>
<option value="0">不使用签名档</option>
<?php
			for ($i = 1; $i < 6; $i++)
			{
				if ($currentuser["signature"] == $i)
				{
?>
<option value="<?php echo $i; ?>" selected="selected">第 <?php echo $i; ?> 个</option>
<?php
				}
				else
				{
?>
<option value="<?php echo $i; ?>">第 <?php echo $i; ?> 个</option>
<?php
				}
			}
		}
?>
</select>
 [<a target="_balnk" href="bbssig.php">查看签名档</a>]
<input type="checkbox" name="outgo" value="1" />转信<br />
<textarea name="text"  onkeydown='if(event.keyCode==87 && event.ctrlKey) {document.postform.submit(); return false;}'  onkeypress='if(event.keyCode==10) return document.postform.submit()' rows="20" cols="80" wrap="physical">
<?php
    if($reid > 0){
    $filename = $articles[1]["FILENAME"];
    $filename = "boards/" . $board . "/" . $filename;
	if(file_exists($filename))
	{
	    $fp = fopen($filename, "r");
        if ($fp) {
		    $lines = 0;
            $buf = fgets($fp,256);       /* 取出第一行中 被引用文章的 作者信息 */
			$end = strrpos($buf,")");
			$start = strpos($buf,":");
			if($start != FALSE && $end != FALSE)
			    $quser=substr($buf,$start+2,$end-$start-1);

            echo "\n【 在 " . $quser . " 的大作中提到: 】\n";
            for ($i = 0; $i < 3; $i++) {
                if (($buf = fgets($fp,500)) == FALSE)
                    break;
            }
            while (1) {
                if (($buf = fgets($fp,500)) == FALSE)
                    break;
                if (strncmp($buf, ": 【", 4) == 0)
                    continue;
                if (strncmp($buf, ": : ", 4) == 0)
                    continue;
                if (strncmp($buf, "--\n", 3) == 0)
                    break;
                if (strncmp($buf,'\n',1) == 0)
                    continue;
                if (++$lines > 10) {
                    echo ": ...................\n";
                    break;
                }
                /* */
                if (stristr($buf, "</textarea>") == FALSE)  //filter </textarea> tag in the text
                    echo ": ". $buf;
            }
            fclose($fp);
        }
    }
}
?>
</textarea></td></tr>
<tr><td class="post" align="center">
<input type="submit" value="发表" />
<input type="reset" value="清除" />
<?php
		if (bbs_is_attach_board($brdarr))
		{
?>
<input type="button" name="attach22" value="附件" onclick="return GoAttachWindow()" />
<?php
		}
?>
<script language="JavaScript">
<!--
   function GoAttachWindow(){

   	var hWnd = window.open("bbsupload.php","_blank","width=600,height=300,scrollbars=yes");

	if ((document.window != null) && (!hWnd.opener))

		   hWnd.opener = document.window;

	hWnd.focus();

   	return false;

   }
-->
</script>
</td></tr>
</table></form>

<center><a href="/bbsdoc.php?board=<?php echo $brd_encode; ?>">本讨论区</a></center>

</html>
<?php
?>
