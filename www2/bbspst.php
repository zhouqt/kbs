<?php
	require("www2-funcs.php");
	require("www2-board.php");
	login_init();
	bbs_session_modify_user_mode(BBS_MODE_POSTING);
	assert_login();

	if (isset($_GET["board"]))
		$board = $_GET["board"];
	else
		html_error_quit("错误的讨论区");
	// 检查用户能否阅读该版
	$brdarr = array();
	$brdnum = bbs_getboard($board, $brdarr);
	if ($brdnum == 0)
		html_error_quit("错误的讨论区");
	$board = $brdarr["NAME"];
	bbs_set_onboard($brdnum,1);
	$usernum = $currentuser["index"];
	if (bbs_checkreadperm($usernum, $brdnum) == 0)
		html_error_quit("错误的讨论区");
	if(bbs_checkpostperm($usernum, $brdnum) == 0) {
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
		$num = bbs_get_records_from_id($board, $reid,$dir_modes["NORMAL"],$articles);
		if ($num == 0)
		{
			html_error_quit("错误的 Re 文编号");
		}
		if ($articles[1]["FLAGS"][2] == 'y')
			html_error_quit("该文不可回复!");
	}
	$brd_encode = urlencode($board);
	
	bbs_board_nav_header($brdarr, $reid ? "回复文章" : "发表文章");
?>
<link rel="stylesheet" type="text/css" href="ansi.css"/>
<form name="postform" method="post" action="bbssnd.php?board=<?php echo $brd_encode; ?>&reid=<?php echo $reid; ?>" class="large">
<?php
	if (bbs_normalboard($board)) {
?>
<div class="article smaller" id="bbsnot">正在载入讨论区备忘录...</div>
<iframe src="bbsnot.php?board=<?php echo $board; ?>" width="0" height="0" frameborder="0" scrolling="no"></iframe>
<?php
	} else {
?>
<div class="article smaller"><a href="bbsnot.php?board=<?php echo $brd_encode; ?>" target="_blank">查看讨论区备忘录</a></div>
<?php
	}
?>
<fieldset><legend><?php echo $reid ? "回复文章" : "发表文章"; ?></legend>
发信人: <?php echo $currentuser["userid"]; ?>, 信区: <?php echo $brd_encode; ?> [<a href="bbsdoc.php?board=<?php echo $brd_encode; ?>">本讨论区</a>]<br/>
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
标&nbsp;&nbsp;题: <input type="text" tabindex="1" name="title" size="40" maxlength="100" value="<?php echo $nowtitle?htmlspecialchars($nowtitle,ENT_QUOTES)." ":""; ?>" <?php if (!$reid) echo 'id="sfocus"'; ?>/><br/>
<?php
		if (bbs_is_attach_board($brdarr))
		{
?>
附&nbsp;&nbsp;件: <input type="text" name="attachname" size="50" value="" disabled="disabled" />
<a href="bbsupload.php" target="_blank">操作附件</a>(新窗口打开)<br/>
<?php
		}
?>
使用签名档 <select name="signature">
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
?>
<option value="-1" <?php if ($currentuser["signature"] < 0) echo "selected "; ?>>随机签名档</option>
<?php
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
<input type="checkbox" name="mailback" value="1" />re文抄送信箱
<br />
<textarea name="text" tabindex="2" onkeydown='return textarea_okd(dosubmit, event);' wrap="physical" <?php if ($reid) echo 'id="sfocus"'; ?>>
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
                //if (stristr($buf, "</textarea>") == FALSE)  //filter </textarea> tag in the text
                    echo ": ". htmlspecialchars($buf);
            }
			echo "\n\n";
            fclose($fp);
        }
    }
}
?>
</textarea><br/>
<div class="oper">
<input type="button" onclick="dosubmit();" tabindex="3" name="post" value="发表" />
&nbsp;&nbsp;&nbsp;&nbsp;
<input class="sb1" type="reset" value="返回" onclick="history.go(-1)" />
</div>
</fieldset></form>
<?php
page_footer();
?>
