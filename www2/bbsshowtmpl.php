<?php
	require("www2-funcs.php");
	login_init();
	bbs_session_modify_user_mode(BBS_MODE_READING);
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
	$retnum = bbs_get_tmpls($board,$votearr);

	if( $retnum < 0 )
		$retnum = 0;

	if(isset($_GET["id"]))
		$restr = "&reid=" . $_GET["id"];
	else
		$restr = "";
		
	
	bbs_board_nav_header($brdarr, "模板列表");
?>
<table class="main adj">
<caption>版面 <?php echo $board; ?> 共有 <?php echo $retnum;?> 个模板</caption>
<tr><th>序号</th><th>标题</th><th>类型</th><th>问题个数</th><th></th></tr>
<?php
		for($i = 0; $i < $retnum; $i++ ){
?>
<tr><td>
<?php echo $i+1;?>
</td><td>
<a href="bbsatmpl.php?board=<?php echo $board;?>&num=<?php echo $i+1;?>"><?php echo $votearr[$i]["TITLE"];?></a>
</td><td>
<?php echo $votearr[$i]["TITLE"];?>
</td><td>
<?php echo $votearr[$i]["CONT_NUM"];?>
</td><td>
<a href="bbspsttmpl.php?board=<?php echo $board;?>&num=<?php echo $i+1; echo $restr;?>">使用本模板发文</a>
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
