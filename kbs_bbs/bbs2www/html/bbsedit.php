<?php
	/**
	 * This file lists articles to user.
	 * $Id$
	 */
	require("funcs.php");
    login_init();
	require("boards.php");
	require("board.inc.php");
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
		bbs_set_onboard($brdnum,1);
		$usernum = $currentuser["index"];
		if (bbs_checkreadperm($usernum, $brdnum) == 0)
			html_error_quit("错误的讨论区");
		$board = $brdarr['NAME'];
		if(bbs_checkpostperm($usernum, $brdnum) == 0) {
                    if (!strcmp($currentuser["userid"],"guest"))
		      html_error_quit("请先注册帐号");
                    else 
		      html_error_quit("错误的讨论区或者您无权在此讨论区发表文章");
                }
		if (bbs_is_readonly_board($brdarr))
			html_error_quit("不能在只读讨论区发表文章");
        
        if (isset($_GET['id']))
            $id = intval($_GET['id']);
        else
            html_error_quit("错误的文编号");
		$articles = array();
		$num = bbs_get_records_from_id($brdarr["NAME"], $id,$dir_modes["NORMAL"],$articles);
		if ($num == 0)
			html_error_quit("错误的文编号");
		$ret=bbs_caneditfile($board,$articles[1]['FILENAME']);
    	switch ($ret) {
        	case -1:
        		html_error_quit("讨论区名称错误");
        		break;
        	case -2:
        		html_error_quit("本版不能修改文章");
        		break;
        	case -3:
        		html_error_quit("本版已被设置只读");
        		break;
        	case -4:
        		html_error_quit("无法取得文件记录");
        		break;
        	case -5:
        		html_error_quit("不能修改他人文章!");
        		break;
        	case -6:
        		html_error_quit("同名ID不能修改老ID的文章");
        		break;
        	case -7:
        		html_error_quit("您的POST权被封");
        		break;
            default:
    	}

		$brd_encode = urlencode($brdarr["NAME"]);
	}
?>
<link rel="stylesheet" type="text/css" href="/ansi.css"/>
<script language=javascript>
<!--
function dosubmit() {
    document.postform.post.value='提交中，请稍候...';
    document.postform.post.disabled=true;
    document.postform.submit();
}
//-->
</script>
<body topmargin="0">
<table width="100%" border="0" cellspacing="0" cellpadding="3">
  <tr> 
    <td class="b2">
	    <a href="bbssec.php" class="b2"><font class="b2"><?php echo BBS_FULL_NAME; ?></font></a>
	    -
	    <a href="/bbsdoc.php?board=<?php echo $brd_encode; ?>"><font class="b2"><?php echo $board; ?></font></a> 版
	    - 修改文章 [使用者: <?php echo $currentuser["userid"]; ?>]
	 </td>
  </tr>
  <tr>
    <td>&nbsp;</td>
  </tr>
  <tr><td align="center">
<?php
if (isset($_GET['do'])) {
    $ret=bbs_updatearticle($board,$articles[1]['FILENAME'],preg_replace("/\\\(['|\"|\\\])/","$1",$_POST['text']));
	switch ($ret) {
		case -1:
			html_error_quit("修改文章失败，文章可能含有不恰当内容");
			break;
		case -10:
			html_error_quit("找不到文件!");
			break;
        case 0:
?>
<b>文章修改成功</b>
<br /><br /><br />
[<a href="/bbsdoc.php?board=<?php echo $brd_encode; ?>">返回<?php echo $brdarr['DESC']; ?>讨论区</a>]
[<a href="/bbscon.php?board=<?php echo $brd_encode; ?>&id=<?php echo $id; ?>">返回《<?php echo htmlspecialchars($articles[1]['TITLE']); ?>》</a>]
<br /><br />
<?php
            bbs_board_foot($brdarr,'');
            break;
        default:
            html_error_quit("系统错误");
        
	}
}
else {
?>
<form name="postform" method="post" action="<?php echo $_SERVER['PHP_SELF']; ?>?board=<?php echo $brd_encode; ?>&id=<?php echo $id; ?>&do">
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
发信人: <a href="/bbsqry.php?userid=<?php echo $articles[1]['OWNER']; ?>"><?php echo $articles[1]['OWNER']; ?></a>, 信区: <?php echo $brd_encode; ?> [<a href="/bbsdoc.php?board=<?php echo $brd_encode; ?>">本讨论区</a>]<br>
标&nbsp;&nbsp;题: <input readonly class="sb1" type="text" name="title" size="40" maxlength="100" value="<?php echo $articles[1]['TITLE']; ?>" />
[<a href="/bbsedittitle.php?board=<?php echo $brd_encode; ?>&id=<?php echo $id; ?>">修改标题</a>]
<br /><br />
<textarea class="sb1" name="text"  onkeydown='if(event.keyCode==87 && event.ctrlKey) {dosubmit(); return false;}'  onkeypress='if(event.keyCode==10) return dosubmit()' rows="20" cols="80" wrap="physical">
<?php
    bbs_printoriginfile($board,$articles[1]['FILENAME']);
?>
</textarea><br>
<center>
<input class="sb1" type="button" onclick="dosubmit();" name="post" value="修改" />
<input class="sb1" type="reset" value="复原" />
<input class="sb1" type="reset" value="返回" onclick="history.go(-1)" />
</center>
</td></tr>
</table></form>
<?php

}
?>
</td></tr>
</table>
<?php
html_normal_quit();
?>
