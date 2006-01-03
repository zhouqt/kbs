<?php
	require("www2-funcs.php");
	login_init();
	bbs_session_modify_user_mode(BBS_MODE_READING);
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
		html_error_quit("系统错误");

	bbs_board_nav_header($brdarr, "模板详细显示");
?>
<h1>模板 (<?php echo $retnum;?>): <?php echo $votearr[0]["TITLE"];?></h1>
<div style="padding-top: 1em;">标题: <?php echo $votearr[0]["TITLE_TMPL"];?></div>
<table class="main adj">
<tr><th>问题序号</th><th>问题</th><th>回答长度</th></tr>
<?php
	for($i = 0; $i < $votearr[0]["CONT_NUM"]; $i++ ){
?>
<tr><td>
<?php echo $i+1;?>
</td><td>
<?php echo $votearr[$i+1]["TEXT"];?>
</td><td>
<?php echo $votearr[$i+1]["LENGTH"];?>
</td></tr>
<?php
	}
?>
</table>
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
<div class="oper">
<a href="bbsdoc.php?board=<?php echo $board;?>">返回本讨论区</a>
<a href="bbsshowtmpl.php?board=<?php echo $board; ?>">全部发文模板</a>
<a href="javascript:history.go(-1)">快速返回</a>
</div>
<?php
	page_footer();
?>
