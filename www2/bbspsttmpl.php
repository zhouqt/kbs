<?php
	require("www2-funcs.php");
	login_init();
	bbs_session_modify_user_mode(BBS_MODE_POSTTMPL);
	assert_login();

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

	if( $votearr[0]["CONT_NUM"] <= 0 )
		html_error_quit("本模板暂不可使用");
	
	if(isset($_GET["reid"]))
	{
		$restr = "&reid=" . $_GET["reid"];
		$reid = intval($_GET["reid"]);
		$articles = array();
		if ($reid > 0)
		{
			$anum = bbs_get_records_from_id($board, $reid, $dir_modes["NORMAL"], $articles);
			if ($anum == 0)
			{
				html_error_quit("错误的 Re 文编号");
			}
			if ($articles[1]["FLAGS"][2] == 'y')
				html_error_quit("该文不可回复!");
		}
	}
	else
	{
		$restr = "";
		$reid = 0;
	}
	
	bbs_board_nav_header($brdarr, "模板发文");

	if(isset($_GET["do"])){

		$tmpfile = "tmp/".$currentuser["userid"].".tmpl.tmp" ;
		$title = bbs_make_tmpl_file($board, $num, @$_POST["text0"], @$_POST["text1"], @$_POST["text2"], @$_POST["text3"], @$_POST["text4"], @$_POST["text5"], @$_POST["text6"], @$_POST["text7"], @$_POST["text8"], @$_POST["text9"], @$_POST["text10"], @$_POST["text11"], @$_POST["text12"], @$_POST["text13"], @$_POST["text14"], @$_POST["text15"], @$_POST["text16"], @$_POST["text17"], @$_POST["text18"], @$_POST["text19"], @$_POST["text20"]);

		if( $title == "" )
			html_error_quit("请输入标题");
?>
<h1>模板发文预览</h1>
<div class="large"><div class="article">
标题: <?php echo $title; ?><br/><br/>
<?php
		echo bbs_printansifile($tmpfile);
?>
</div></div>
<form method="post" action="bbssnd.php?board=<?php echo $board; echo $restr; ?>">
<input type="hidden" name="tmpl" value="1"/>
<input type="hidden" name="title" value="<?php echo $title;?>"/>
<input type="submit" value="发表" />
</form>
<?php
	}else{
?>
<h1 class="bt">模板 (<?php echo $retnum;?>): <?php echo $votearr[0]["TITLE"];?></h1>
<?php
		if( $votearr[0]["FILENAME"] != "" ){
?>
<div class="large"><div class="article">
<h2>正文格式</h2>
<?php
			bbs_print_article($votearr[0]["FILENAME"]);
?>
</div></div>
<?php
		}
?>
<form action="<?php echo $_SERVER['PHP_SELF'];?>?do&board=<?php echo $board;?>&num=<?php echo $num; echo $restr; ?>" method="post" class="large">
	<fieldset><legend>模板发表</legend><div class="input">
<?php
		if( $votearr[0]["TITLE_TMPL"]=="" || strstr($votearr[0]["TITLE_TMPL"], "[$0]") ){
			$retitle = "";
			if ($reid)
			{
		        if(!strncmp($articles[1]["TITLE"],"Re: ",4))
		        	$retitle = $articles[1]["TITLE"] . " ";
		        else
		            $retitle = "Re: " . $articles[1]["TITLE"] . " ";
		    }
?>
	<label>文章标题([$0]):</label><input type="text" name="text0" size="50" maxlength="50" value="<?php echo $retitle; ?>"/><br/><br/>
<?php
		} else {
?>
	标题: <?php echo $votearr[0]["TITLE_TMPL"];?><br/><br/>
<?php
		}
		for($i = 0; $i < $votearr[0]["CONT_NUM"]; $i++ ){
?>

	<label>问题<?php echo $i+1;?>:</label><?php echo $votearr[$i+1]["TEXT"];?> (最长<?php echo $votearr[$i+1]["LENGTH"];?>字节)( [$<?php echo $i+1;?>])
<?php
			if( strstr( $votearr[0]["TITLE_TMPL"], "[$".($i+1)."]" ) ){
				$maxline = ( $votearr[$i+1]["LENGTH"] > 80 ) ? "80" : $votearr[$i+1]["LENGTH"];
?>
	<input type="text" name="text<?php echo $i+1;?>" size="<?php echo $maxline; ?>" maxlength="<?php echo $votearr[$i+1]["LENGTH"];?>" /><br/><br/>
<?php
			} else {
?>
	<textarea name="text<?php echo $i+1;?>" class="shorter" wrap="physical" maxlength="<?php echo $votearr[$i+1]["LENGTH"];?>"></textarea><br/><br/>
<?php
			}
		}
?>
	</div></fieldset>
	<div class="oper"><input type="submit" value="预览发文"/></div>
</form>
<?php
	}
?>
<div class="oper">
<a href="bbsdoc.php?board=<?php echo $board;?>">返回本讨论区</a>
<a href="javascript:history.go(-1)">快速返回</a>
</div>
<?php
	page_footer();
?>
