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
		$usernum = $currentuser["index"];
		if (bbs_checkreadperm($usernum, $brdnum) == 0)
			html_error_quit("错误的讨论区");
		// TODO: Added bbs_check_post_perm();
		if (bbs_is_readonly_board($brdarr))
			html_error_quit("不能在只读讨论区发表文章");
		if (isset($_GET["reid"]))
			$reid = $_GET["reid"];
		else {
			$reid = 0;
		}
		settype($reid, "integer");
		if ($reid > 0)
		{
			$articles = bbs_get_records_from_id($brdarr["NAME"], $reid, 
					$dir_modes["NORMAL"]);
			if ($articles == FALSE)
			{
				html_error_quit("错误的 Re 文编号");
			}
		}
		$brd_encode = urlencode($brdarr["NAME"]);
	}
?>
<body>
<center>
<?php echo $BBS_FULL_NAME; ?> -- 发表文章 [使用者: <?php echo $currentuser["userid"]; ?>]
<hr class="default" />
<form name="postform" method="post" action="/cgi-bin/bbs/bbssnd?board=<?php echo $brd_encode; ?>&reid=<?php echo $reid; ?>">
<table border="1">
<tr>
<td>
<?php
		$notes_file = bbs_get_vote_filename($brdarr["NAME"], "notes");
		$fp = fopen($notes_file, "r");
		if ($fp == FALSE)
		{
			//html_error_quit("现在没有备忘录");
			$notes_file = "vote/notes";
			$fp = fopen($notes_file, "r");
			if ($fp == FALSE)
			{
?>
<font color="green">发文注意事项: <br />
发文时应慎重考虑文章内容是否适合公开场合发表，请勿肆意灌水。谢谢您的合作。<br/></font>
<?php
			}
		}
		if ($fp != FALSE)
		{
			fclose($fp);
			bbs_printansifile($notes_file);
		}
?>
</td>
</tr>
<tr><td>
作者: <?php echo $currentuser["userid"]; ?><br />
标题: <input type="text" name="title" size="40" maxlength="100" value="Re: <?php echo $articles[1]["TITLE"]; ?>"><br />
看版: [<?php echo $brd_encode; ?>]<br />
<?php
		if ($brdarr["FLAG"]&BBS_BOARD_ATTACH)
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
<textarea name="text" rows="20" cols="80" wrap="physical">

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

</html>
<?php
?>
