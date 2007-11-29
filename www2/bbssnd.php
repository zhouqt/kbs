<?php
	require("www2-funcs.php");
	require("www2-board.php");
	login_init();
	assert_login();
		
	if( !isset( $_GET["board"] )) {
		html_error_quit("未指定发文版面!");
	}
	$boardName = $_GET["board"];
	$brdArr=array();
	$boardID= bbs_getboard($boardName,$brdArr);
	if( $boardID == 0) html_error_quit("指定的版面不存在!");
	$boardName = $brdArr["NAME"];
	$usernum = $currentuser["index"];
	if (bbs_checkreadperm($usernum, $boardID) == 0) html_error_quit("您无权阅读该版!");
	if (bbs_is_readonly_board($brdArr)) html_error_quit("本版为只读讨论区!");
	if (bbs_checkpostperm($usernum, $boardID) == 0) html_error_quit("您无权在该版面发文!");
	
	if (!isset($_POST["title"])) html_error_quit("没有指定文章标题!");
	if (!isset($_POST["tmpl"])) {
		if (!isset($_POST["text"])) html_error_quit("没有指定文章内容!");
		$tmpl = 0;
	} else {
		$tmpl = 1;
		$filename = "tmp/".$currentuser["userid"].".tmpl.tmp";
		if (($handle = @fopen($filename, "r")) !== false) {
			$contents = fread($handle, filesize($filename));
			fclose($handle);
			unlink($filename);
		} else {
			html_error_quit("无法读取文件");
		}
	}
	if (isset($_GET["reid"])) $reID = $_GET["reid"];
	else
		$reID = 0;
	if (bbs_is_outgo_board($brdArr)) $outgo = intval(@$_POST["outgo"]);
	else $outgo = 0;

	if(@$_POST["havemath"] == "1")
		$is_tex = 1;
	else
		$is_tex = 0;
	
	settype($reID, "integer");
			
	$articles = array();
	if ($reID > 0) {
		$num = bbs_get_records_from_id($boardName, $reID, $dir_modes["NORMAL"], $articles);
		if ($num == 0) html_error_quit("错误的Re文编号!");
		if ($articles[1]["FLAGS"][2] == 'y') html_error_quit("该文不可回复!");
	}
	//post articles
	$anony = isset($_POST["anony"])?intval($_POST["anony"]):0;

	bbs_board_nav_header($brdArr, $reID ? "回复文章" : "发表文章");

	if($tmpl & ($reID > 0)) {
		$filename = "boards/" . $boardName . "/" . $articles[1]["FILENAME"];
		$contents .= bbs_get_quote($filename);
	}
	$ret = bbs_postarticle($boardName, rtrim($_POST["title"]), 
		($tmpl ? $contents :$_POST["text"]), intval(@$_POST["signature"]), $reID, 
		$outgo, $anony, @intval($_POST["mailback"]), $is_tex);
	switch ($ret) {
		case -1:
			html_error_quit("错误的讨论区名称!");
			break;
		case -2: 
			html_error_quit("本版为二级目录版!");
			break;
		case -3: 
			html_error_quit("标题为空!");
			break;
		case -4: 
			html_error_quit("此讨论区是唯读的, 或是您尚无权限在此发表文章!");
			break;		
		case -5:	
			html_error_quit("很抱歉, 你被版务人员停止了本版的post权利!");
			break;	
		case -6:
			html_error_quit("两次发文/信间隔过密,请休息几秒再试!");	
			break;
		case -7: 
			html_error_quit("无法读取索引文件! 请通知站务人员, 谢谢! ");
			break;
		case -8:
			html_error_quit("本文不可回复!");
			break;
		case -9:
			html_error_quit("系统内部错误, 请迅速通知站务人员, 谢谢!");
			break;
        case -21:
            html_error_quit("您的积分不符合当前讨论区的设定, 暂时无法在当前讨论区发表文章...");
            break;
	}
	if ($ret == -10) {
		$prompt = "发文成功！<br/><br/>但是很抱歉，本文可能含有不当内容，需经审核方可发表。<br/><br/>" .
                  "根据《帐号管理办法》，被系统过滤的文章视同公开发表。请耐心等待<br/>" .
                  "站务人员的审核，不要多次尝试发表此文章。<br/><br/>" .
                  "如有疑问，请致信 SYSOP 咨询。";
	} else {
		$prompt = "发文成功！<br/>" . 
                  "本页面将在3秒后自动返回版面文章列表<meta HTTP-EQUIV=REFRESH CONTENT='3; URL=bbsdoc.php?board=" . $boardName . "'/><div id='kbsrcInfo'>con,$boardID,$ret</div>";
	}
	html_success_quit($prompt,
	array("<a href='" . MAINPAGE_FILE . "'>返回首页</a>", 
	"<a href='bbsdoc.php?board=" . $boardName . "'>返回 " . $brdArr['DESC'] . "</a>"));
?>
