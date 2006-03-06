<?php
	require("www2-funcs.php");
	login_init();
	bbs_session_modify_user_mode(BBS_MODE_READING);
	assert_login();

	if( !isset($_GET["board"]) && !isset($_POST["board"]))
		html_error_quit("错误的讨论区");
	if( isset($_GET["board"]) )
		$board = $_GET["board"];
	else
		$board = $_POST["board"];

	$brdarr = array();
	$brdnum = bbs_getboard($board, $brdarr);
	if ($brdnum == 0){
		html_error_quit("错误的讨论区");
	}
	$usernum = $currentuser["index"];
	if (bbs_checkreadperm($usernum, $brdnum) == 0){
		html_error_quit("错误的讨论区");
	}

	if( !isset($_GET["id"]) && !isset($_POST["id"]))
		html_error_quit("错误的文章号");
	if( isset($_GET["id"]) )
		$id = $_GET["id"];
	else
		$id = $_POST["id"];

	$articles = array ();
	$num = bbs_get_records_from_id($brdarr["NAME"], $id, $dir_modes["NORMAL"] , $articles);
	if($num == 0)
		html_error_quit("错误的文章号");
	$brd_encode = urlencode($board);
	
	bbs_board_nav_header($brdarr, "文章转寄");
	if (!isset($_GET["do"])){
?>
<form action="bbsfwd.php?do" method="post" class="medium"/>
<input type="hidden" name="board" value="<?php echo $brdarr["NAME"];?>"/>
<input type="hidden" name="id" value="<?php echo $id;?>"/>
	<fieldset>
		<legend>转寄文章：<?php echo $articles[1]["OWNER"];?> 的 <a href="bbscon.php?bid=<?php echo $brdnum; ?>&id=<?php echo $id; ?>"><?php echo htmlspecialchars($articles[1]["TITLE"]); ?></a></legend>
		<div class="inputs">
			<label>把文章转寄给 (请输入对方的id或email地址):</label>
			<input type="text" name="target" size="40" maxlength="69" id="sselect" value="<?php echo $currentuser["email"];?>"><br/>
			<input type="checkbox" name="big5" id="big5" value="1"/><label for="big5" class="clickable">使用BIG5码</label>
			<input type="checkbox" name="noansi" id="noansi" value="1" checked /><label for="noansi" class="clickable">过滤ANSI控制符</label>
		</div>
	</fieldset>
	<div class="oper"><input type="submit" value="确定转寄"></div>
</form>
<?php
	} else {
		if( isset($_POST["target"]) )
			$target =  $_POST["target"];
		else
			$target = "";
		if($target == "")
			html_error_quit("请指定对象");

		if( isset($_POST["big5"]) )
			$big5 = $_POST["big5"];
		else
			$big5=0;
		settype($big5, "integer");

		if( isset($_POST["noansi"]) )
			$noansi = $_POST["noansi"];
		else
			$noansi=0;
		settype($noansi, "integer");

		$ret = bbs_doforward($brdarr["NAME"], $articles[1]["FILENAME"], $articles[1]["TITLE"], $target, $big5, $noansi);
		if($ret < 0)
			html_error_quit("系统错误:".$ret);
		
		html_success_quit("文章已转寄给" . $target);
	}
	page_footer();
?>
