<?php
	/**
	 * This file lists articles to user.
	 * $Id$
	 */
	require("funcs.php");
login_init();
	require("boards.php");
	if ($loginok != 1)
		html_nologin();
	else
	{
		html_init("gb2312","","",1);
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
			if(bbs_is_noreply_board($brdarr))
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
<link rel="stylesheet" type="text/css" href="/ansi.css"/>
<script language=javascript>
<!--
function dosubmit() {
    document.postform.post.value='发表中，请稍候...';
    document.postform.post.disabled=true;
    document.postform.submit();
}
//-->
</script>
<body>
<table width="100%" border="0" cellspacing="0" cellpadding="3">
  <tr> 
    <td class="b2">
	    <a href="bbssec.php" class="b2"><font class="b2"><?php echo BBS_FULL_NAME; ?></font></a>
	    -
	    <a href="/bbsdoc.php?board=<?php echo $brd_encode; ?>"><font class="b2"><?php echo $board; ?></font></a> 版
	    - 发表文章 [使用者: <?php echo $currentuser["userid"]; ?>]
	 </td>
  </tr>
  <tr>
    <td>&nbsp;</td>
  </tr>
  <tr><td align="center">

<form name="postform" method="post" action="bbssnd.php?board=<?php echo $brd_encode; ?>&reid=<?php echo $reid; ?>">
<table border="0" cellspacing="5">
<tr>
<td class="b2 sb5">
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
<tr><td class="b2 sb5">
发信人: <?php echo $currentuser["userid"]; ?>, 信区: <?php echo $brd_encode; ?> [<a href="/bbsdoc.php?board=<?php echo $brd_encode; ?>">本讨论区</a>]<br>
<?php
		if ($reid)
		{
	        if(!strncmp($articles[1]["TITLE"],"Re: ",4))$nowtitle = $articles[1]["TITLE"];
	        else
	            $nowtitle = "Re: " . $articles[1]["TITLE"];
	    } else {
	        $nowtitle = "";
	    }
?>
标&nbsp;&nbsp;题: <input class="sb1" type="text" name="title" size="40" maxlength="100" value="<?php echo $nowtitle; ?>  " /><br />
<?php
		if (bbs_is_attach_board($brdarr))
		{
?>
附&nbsp;&nbsp;件: <input class="sb1" type="text" name="attachname" size="50" value="" disabled="disabled" />
<a href="#" onclick="return GoAttachWindow()" class="b2">操作附件</a><br />
<?php
		}
?>
使用签名档 <select class="sb1" name="signature">
<?php
		if ($currentuser["signum"] == 0)
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
			for ($i = 1; $i <= $currentuser["signum"]; $i++)
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
 [<a target="_blank" href="bbssig.php">查看签名档</a>]
<?php
    if (bbs_is_anony_board($brdarr))
    {
?>
<input type="checkbox" name="anony" value="1" />匿名
<?php
    }
    if (bbs_is_outgo_board($brdarr)) {
        $local_save = 0;
        if ($reid > 0) $local_save = !strncmp($articles[1]["INNFLAG"], "LL", 2);
?>
<input type="checkbox" name="outgo" value="1"<?php if (!$local_save) echo " checked=\"checked\""; ?> />转信
<?php
    }
?>
<br />
<textarea class="sb1" name="text"  onkeydown='if(event.keyCode==87 && event.ctrlKey) {dosubmit(); return false;}'  onkeypress='if(event.keyCode==10) return dosubmit()' rows="20" cols="80" wrap="physical">
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
                if (strncmp($buf, "【", 2) == 0)
                    continue;
                if (strncmp($buf, ": ", 2) == 0)
                    continue;
                if (strncmp($buf, "--\n", 3) == 0)
                    break;
                if (strncmp($buf, "\n", 1) == 0)
                    continue;
                if (++$lines > QUOTED_LINES) {
                    echo ": ...................\n";
                    break;
                }
                if (stristr($buf, "</textarea>") == FALSE)  //filter </textarea> tag in the text
                    echo ": ". $buf;
            }
			echo "\n\n";
            fclose($fp);
        }
    }
}
?>
</textarea><br>
<center>
<input class="sb1" type="button" onclick="dosubmit();" name="post" value="发表" />
<input class="sb1" type="reset" value="清除" />
<?php
		if (bbs_is_attach_board($brdarr))
		{
?>
<input class="sb1" type="button" name="attach22" value="附件" onclick="return GoAttachWindow()" />
<?php
		}
?>
</center>
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

</td></tr>
</table>
<?php
html_normal_quit();
?>
