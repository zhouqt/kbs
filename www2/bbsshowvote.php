<?php
	require("www2-funcs.php");
	login_init();
	bbs_session_modify_user_mode(BBS_MODE_VOTING);
	assert_login();
	if(isset($_GET["board"]))
		$board = $_GET["board"];
	else
		html_error_quit("讨论区错误");

	$brdarr = array();
	$brdnum = bbs_getboard($board,$brdarr);
	if($brdnum == 0)
		html_error_quit("错误的讨论区");

	if(bbs_checkreadperm($currentuser["index"],$brdnum)==0)
		html_error_quit("您没有权限");

	$votearr = array();
	$retnum = bbs_get_votes($board,$votearr);

	if( $retnum < 0 )
		$retnum = 0;

	bbs_board_nav_header($brdarr, "投票列表");
?>
<table class="main adj">
<caption>版面 <?php echo $board; ?> 共有 <?php echo $retnum;?> 个投票</caption>
<tr><th>序号</th><th>标题</th><th>类型</th><th>开启者</th><th>开启日期</th><th>投票天数</th></tr>
<?php
	for($i = 0; $i < $retnum; $i++ ){
?>
<tr><td>
<?php echo $i+1;?>
</td><td>
<a href="bbsvote.php?board=<?php echo $board;?>&num=<?php echo $i+1;?>"><?php echo $votearr[$i]["TITLE"];?></a>
</td><td>
<?php echo $votearr[$i]["TYPE"];?>
</td><td>
<?php echo $votearr[$i]["USERID"];?>
</td><td>
<?php echo date("r",$votearr[$i]["DATE"]);?>
</td><td>
<?php echo $votearr[$i]["MAXDAY"];?>
</td></tr>
<?php
	}
?>
</table>
<div class="oper">
<a href="bbsdoc.php?board=<?php echo $board;?>">返回本讨论区</a>
<a href="javascript:history.go(-1)">快速返回</a>
</div>
<?php
	page_footer();
?>
