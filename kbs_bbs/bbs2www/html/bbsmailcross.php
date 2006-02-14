<?php
	require("www2-funcs.php");
	login_init();
	assert_login();

	if(isset($_POST["filename"]))
		$filename = $_POST["filename"];
	else
		html_error_quit("找不到信件");
	if (strstr($filename, "..") || strstr($filename, "/"))
		html_error_quit("错误的参数");
	if(isset($_POST["title"]))
		$title = urldecode($_POST["title"]);
	else
		html_error_quit("错误的参数");
	if(isset($_POST["target"]))
	{
		$target = trim(ltrim($_POST["target"]));
		if (!$target)
			html_error_quit("请输入转入的讨论区");
	}
	else
		html_error_quit("错误的参数");
	$outgo = isset($_POST['outgo']) ? 1 : 0;
	
	
	$ret = bbs_docross("", 0, $target, $outgo, $filename, $title);
	switch ($ret) {
		case 0:
			html_success_quit("转贴成功！",
			array("<a href='bbsdoc.php?board=" . $target . "'>进入 " . $target . " 讨论区</a>"),
			"<a href=\"javascript:history.go(-1);\">返回</a>");
			break;
		case -1:
			html_error_quit("讨论区错误");
			break;
		case -2:
			html_error_quit("讨论区 ".$target. " 不存在");
			break;
		case -3:
			html_error_quit("不能转入只读讨论区");
			break;
		case -4:
			html_error_quit("您尚无 ".$target." 讨论区的发文权限");
			break;
		case -5:
			html_error_quit("您被封禁了 ".$target." 讨论区的发文权限");
			break;
		case -6:
			html_error_quit("转入文章错误");
			break;
		case -7:
			html_error_quit("该文已被转载过一次");
			break;
		case -8:
			html_error_quit("不能将文章转载到本版");
			break;
		case -9:
			html_error_quit($target." 讨论区不能上传附件");
			break;
		default:
	}
	html_error_quit("系统错误，请联系管理员");	
?>
