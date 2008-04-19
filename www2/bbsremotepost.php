<?php
	// this script deals with inter-site post cross.
	require("www2-funcs.php");
	require("www2-board.php");

	if($_SERVER["REMOTE_ADDR"] != "127.0.0.1")
		exit;

	$rpid = $_POST["rpid"];
	if($rpid == "")
		exit;

    // check user exists
	$userid = $_POST["user"];
	$uarr = array();
	if(($userid == "") || (bbs_getuser($userid, $uarr) == 0)) {
		print("用户 {$userid} 不存在。");
		exit;
	}
	$uid = $uarr["index"];

	// check board exists
	$bname = $_POST["board"];
	$barr = array();
	$bid = bbs_getboard($bname, $barr);
	if($bid == 0) {
		print("版面 {$bname} 不存在。");
		exit;
	}
	$bname = $barr["NAME"];

	// check if can post
	if(bbs_checkreadperm($uid, $bid) == 0) {
		print("没有阅读权限。");
		exit;
	}
	if(bbs_is_readonly_board($barr)) {
		print("版面为只读状态。");
		exit;
	}
	if(bbs_checkpostperm($uid, $bid) == 0) {
		print("没有发表权限。");
		exit;
	}

	// check from
	$fromsite = $_POST["site"];
	$fromboard = $_POST["fromboard"];

	// check title and content
	$title = $_POST["title"];
	if($title == "") {
		print("没有文章标题。");
		exit;
	}
	$content = $_POST["content"];
	if($content == "") {
		print("没有文章内容");
		exit;
	}

	// write content into file
	$fname = "tmp/remotepost_{$rpid}";
	$fp = fopen($fname, "w");
	if(!$fp) {
		print("无法写入文件。");
		exit;
	}
	fwrite($fp, "【 以下文字转载自 {$fromsite} 的 {$fromboard} 讨论区 】\n");
	fwrite($fp, $content);
	fclose($fp);
	$ret = bbs_post_file_alt($fname, $userid, $title, $bname, NULL, 0x04, 0, 0);
	unlink($fname);
	if($ret == 0) {
		print("转载成功。");
		exit;
	}
	else {
		print("转载失败，错误号{$ret}。");
		exit;
	}

?>
