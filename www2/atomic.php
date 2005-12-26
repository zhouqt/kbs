<?php
define('UTF8', TRUE);
define('ARTCNT', 20);

require("www2-funcs.php");
require("www2-board.php");
login_init();

if (UTF8) {
	iconv_set_encoding("internal_encoding", "gb18030");
	iconv_set_encoding("output_encoding", "UTF-8");
	ob_start("ob_iconv_handler");
}

$atomic_header_shown = false;
$atomic_board = false;
$atomic_brdarr = array();
$atomic_brdnum = false;


$act = @$_GET["act"];
switch($act) {
	case "post":
		atomic_post();
		break;
	case "article":
		atomic_article();
		break;
	case "board":
		atomic_board();
		break;
	case "logout":
		bbs_wwwlogoff();
		delete_all_cookie();
		$currentuser["userid"] = "guest"; //hack
	default:
		atomic_mainpage();
		break;
}

function atomic_header() {
	global $atomic_header_shown, $cachemode;
	if ($atomic_header_shown) return;
	if ($cachemode=="") {
		cache_header("nocache");
	}
	$atomic_header_shown = true;
	echo '<html><head><meta http-equiv="content-type" content="text/html; charset=' . (UTF8?"UTF-8":"gb2312") . '"></head><body>';
}

function atomic_footer() {
	echo "</body></html>";
}

function atomic_error($msg) {
	global $atomic_header_shown;
	if (!$atomic_header_shown) atomic_header();
	echo $msg . " [<a href='atomic.php'>回首页</a>]";
	atomic_footer();
}

function atomic_get_input($str) {
	if (UTF8) return (iconv("UTF-8", "gb18030", $str));
	else return $str;
}

function atomic_show_boardjump() {
		echo <<<END
<form action="" method="get"><input type="hidden" name="act" value="board"/>
去讨论区: <input type="text" name="board" /> <input type="submit" value="Go"/>
</form>
END;
}


function atomic_get_board($checkpost = false) {
	global $currentuser, $atomic_board, $atomic_brdarr, $atomic_brdnum;
	if (isset($_GET["board"]))
		$atomic_board = $_GET["board"];
	else{
		atomic_error("错误的讨论区");
	}
	$brdarr = array();
	$atomic_brdnum = bbs_getboard($atomic_board, $brdarr);
	$atomic_brdarr = $brdarr;
	if ($atomic_brdnum == 0){
		atomic_error("错误的讨论区");
	}
	$atomic_board = $atomic_brdarr["NAME"];
	$usernum = $currentuser["index"];
	if (bbs_checkreadperm($usernum, $atomic_brdnum) == 0){
		atomic_error("错误的讨论区");
	}
	if ($atomic_brdarr["FLAG"]&BBS_BOARD_GROUP) {
		atomic_error("不支持版面组");
	}
	bbs_set_onboard($atomic_brdnum,1);
	
	if ($checkpost) {
		if(bbs_checkpostperm($usernum, $atomic_brdnum) == 0) {
			atomic_error("错误的讨论区或者您无权在此讨论区发表文章");
		}
		if (bbs_is_readonly_board($atomic_brdarr)) {
			atomic_error("不能在只读讨论区发表文章");
		}
	}
}

function atomic_board() {
	global $currentuser, $atomic_board, $atomic_brdarr, $atomic_brdnum, $dir_modes;
	atomic_get_board();
	$ftype = $dir_modes["NORMAL"];
	$isnormalboard = bbs_normalboard($atomic_board);
	if ($isnormalboard && isset($_GET["page"])) {
		$dotdirname = bbs_get_board_index($atomic_board, $ftype);
		if (cache_header("public",@filemtime($dotdirname),10)) return;
	}
	atomic_header();
	atomic_show_boardjump();
	
	$total = bbs_countarticles($atomic_brdnum, $ftype);
	if ($total <= 0) {
		atomic_error("本讨论区目前没有文章");
	}

	$page = isset($_GET["page"]) ? @intval($_GET["page"]) : 0;
	if (isset($_GET["start"])) {
		$page = (@intval($_GET["start"]) + ARTCNT - 1) / ARTCNT;
	}
	settype($page, "integer");
	$start = ($page > 0) ? ($page - 1) * ARTCNT + 1 : 0;
	if ($start == 0 || $start > ($total - ARTCNT + 1))
	{
		if ($total <= ARTCNT)
		{
			$start = 1;
			$page = 1;
		}
		else
		{
			$start = ($total - ARTCNT + 1);
			$page = ($start + ARTCNT - 1) / ARTCNT + 1;
		}
	}
	else
		$page = ($start + ARTCNT - 1) / ARTCNT;
	$articles = bbs_getarticles($atomic_board, $start, ARTCNT, $ftype);
	if ($articles == FALSE)
		atomic_error("读取文章列表失败");
	
	$html = '<form action="?" method="get"><input type="hidden" name="act" value="board"/>';
	$html .= '<input type="hidden" name="board" value="'.$atomic_board.'"/>';
	$html .= '<a href="?act=post&board='.$atomic_board.'">发表</a> ';
	if ($page > 1) {
		$html .= '<a href="?act=board&board='.$atomic_board.'&page=1">第一页</a> ';
		$html .= '<a href="?act=board&board='.$atomic_board.'&page='.($page - 1).'">上一页</a> ';
	} else {
		$html .= '第一页 上一页 ';
	}
	if ($start <= $total - 20) {
		$html .= '<a href="?act=board&board='.$atomic_board.'&page='.($page + 1).'">下一页</a> ';
		$html .= '<a href="?act=board&board='.$atomic_board.'">最后一页</a> ';
	} else {
		$html .= '下一页 最后一页 ';
	}
	$html .= '<input type="submit" value="跳转到"/> 第 <input type="text" name="start" size="3" /> 篇</form>';


	$html .= "<pre>  编号   刊 登 者      日  期   文章标题\n";
	$i = 0;
	foreach ($articles as $article)	{
		$title = $article["TITLE"];
		if (strncmp($title, "Re: ", 4) != 0)
			$title = "● " . $title;

		$flags = $article["FLAGS"];

		if (!strncmp($flags,"D",1)||!strncmp($flags,"d",1)) {
			$html .= " [提示]  ";
		} else {
			$html .= sprintf("%5d ", ($start+$i));
			if ($flags[1] == 'y') {
				$html .= $flags[0];
			} elseif ($flags[0] == 'N' || $flags[0] == '*'){
				$html .= " "; //$flags[0];  //不要未读标记 windinsn
			} else{
				$html .= $flags[0];
			}
			$html .= " ";
		}
		$html .= sprintf("%-12.12s ", $article["OWNER"]);
		$html .= strftime("%b %e  ", $article["POSTTIME"]);
		$html .= "<a href='?act=article&board=".$atomic_board."&id=".$article["ID"]."'>".htmlspecialchars($title)." </a>\n";
		$i++;
	}
	$html .= "</pre>";
	echo $html;
	atomic_footer();
}


function atomic_article() {
	global $currentuser, $atomic_board, $atomic_brdarr, $atomic_brdnum, $dir_modes;
	atomic_get_board();
	$ftype = $dir_modes["NORMAL"];
	$id = @intval($_GET["id"]);

	$url = "?act=article&board=" . $atomic_board . "&id=";
	@$ptr=$_GET["p"];
	// 同主题的指示在这里处理
	if ($ptr == "tn" || $ptr == "tp") {
		$articles = bbs_get_threads_from_id($atomic_brdnum, $id, $ftype,($ptr == "tp")?-1:1);
		if ($articles == FALSE)
			$redirt_id = $id;
		else
			$redirt_id = $articles[0]["ID"];
		header("Location: atomic.php" . $url . $redirt_id);
		exit;
	}

	$total = bbs_countarticles($atomic_brdnum, $ftype);
	if ($total <= 0) {
		atomic_error("错误的文章号,原文可能已经被删除");
	}
	$articles = array ();
	$num = bbs_get_records_from_id($atomic_board, $id, $ftype, $articles);
	if ($num <= 0) atomic_error("错误的文章号,原文可能已经被删除");

	if ($ptr == 'p' && $articles[0]["ID"] != 0) {
		header("Location: atomic.php" . $url . $articles[0]["ID"]);
		exit;
	}
	if ($ptr == 'n' && $articles[2]["ID"] != 0) {
		header("Location: atomic.php" . $url . $articles[2]["ID"]);
		exit;
	}

	$article = $articles[1];
	$filename = bbs_get_board_filename($atomic_board, $article["FILENAME"]);
	$isnormalboard = bbs_normalboard($atomic_board);
	if ($isnormalboard) {
		if (cache_header("public",@filemtime($filename),300)) return;
	}
	if ($currentuser["userid"] != "guest") bbs_brcaddread($atomic_board, $article["ID"]);
	atomic_header();
	$html = '<p><a href="?act=post&board='.$atomic_board.'">发表</a> <a href="?act=post&board='.$atomic_board.'&reid='.$id.'">回复</a> ';
	$url .= $article["ID"];
	$html .= '<a href="' . $url . '&p=p">上一篇</a> ';
	$html .= '<a href="' . $url . '&p=n">下一篇</a> ';
	$html .= '<a href="' . $url . '&p=tp">主题上篇</a> ';
	$html .= '<a href="' . $url . '&p=tn">主题下篇</a> ';
	$html .= '</p>';
	echo $html;
	echo bbs2_readfile_text($filename, 0, 0);
	atomic_footer();
}


function atomic_post() {
	global $currentuser, $atomic_board, $atomic_brdarr, $atomic_brdnum, $dir_modes;
	atomic_get_board(TRUE);

	if (isset($_GET["reid"]))
	{
		$reid = $_GET["reid"];
		if(bbs_is_noreply_board($atomic_brdarr))
			atomic_error("本版只可发表文章,不可回复文章!");
	}
	else {
		$reid = 0;
	}
	settype($reid, "integer");
	$articles = array();
	if ($reid > 0)
	{
		$num = bbs_get_records_from_id($atomic_board, $reid, $dir_modes["NORMAL"], $articles);
		if ($num == 0)
		{
			atomic_error("错误的 Re 文编号");
		}
		if ($articles[1]["FLAGS"][2] == 'y')
			atomic_error("该文不可回复!");
	}
	if (isset($_GET["post"])) {
		if (!isset($_POST["title"])) atomic_error("没有指定文章标题!");
		if (!isset($_POST["text"])) atomic_error("没有指定文章内容!");
		$title = atomic_get_input(trim($_POST["title"]));
		$text = atomic_get_input($_POST["text"]);
		if (isset($_GET["reid"])) $reID = $_GET["reid"];
		else $reID = 0;
		$outgo = bbs_is_outgo_board($atomic_brdarr) ? 1 : 0;
		$anony = 0;
		$ret = bbs_postarticle($atomic_board, $title, $text, $currentuser["signature"], $reID, $outgo, $anony, 0, 0);
		switch ($ret) {
		case -1:
			atomic_error("错误的讨论区名称!");
			break;
		case -2: 
			atomic_error("本版为二级目录版!");
			break;
		case -3: 
			atomic_error("标题为空!");
			break;
		case -4: 
			atomic_error("此讨论区是唯读的, 或是您尚无权限在此发表文章!");
			break;		
		case -5:	
			atomic_error("很抱歉, 你被版务人员停止了本版的post权利!");
			break;	
		case -6:
			atomic_error("两次发文间隔过密,请休息几秒再试!");	
			break;
		case -7: 
			atomic_error("无法读取索引文件! 请通知站务人员, 谢谢! ");
			break;
		case -8:
			atomic_error("本文不可回复!");
			break;
		case -9:
			atomic_error("系统内部错误, 请迅速通知站务人员, 谢谢!");
			break;
		}
		atomic_header();
		$url = "?act=board&board=" . $atomic_board;
		echo "发文成功！本页面将在3秒后自动返回<a href='$url'>版面文章列表</a><meta http-equiv='refresh' content='3; url=" . $url . "'/>";
		atomic_footer();
		return;
	}
	if ($reid) {
		if(!strncmp($articles[1]["TITLE"],"Re: ",4))$nowtitle = $articles[1]["TITLE"];
		else $nowtitle = "Re: " . $articles[1]["TITLE"];
	} else $nowtitle = "";
	atomic_header();
	$html = "<p><a href='?act=board&board=" . $atomic_board . "'>" . $atomic_board . " 版</a>发表文章</p>";
	$html .= "<form action='?act=post&board=" . $atomic_board . "&reid=" . $reid . "&post=1' method='post'>";
	$html .= '标题: <input type="text" name="title" size="40" maxlength="100" value="' . ($nowtitle?htmlspecialchars($nowtitle,ENT_QUOTES)." ":"") . '"/><br/>';
	$html .= '<textarea name="text" rows="20" cols="80" wrap="physical">'."\n";
	if($reid > 0){
		$filename = $articles[1]["FILENAME"];
		$filename = "boards/" . $atomic_board . "/" . $filename;
		$fp = @fopen($filename, "r");
		if ($fp) {
			$lines = 0;
			$buf = fgets($fp,256);       /* 取出第一行中 被引用文章的 作者信息 */
			$end = strrpos($buf,")");
			$start = strpos($buf,":");
			if($start != FALSE && $end != FALSE)
				$quser=substr($buf,$start+2,$end-$start-1);

			$html .= "\n【 在 " . $quser . " 的大作中提到: 】\n";
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
					$html .= ": ...................\n";
					break;
				}
				$html .= ": ". htmlspecialchars($buf);
			}
			$html .= "\n\n";
			fclose($fp);
		}
	}
	$html .= '</textarea><br/><input type="submit" value="发表" /></form>';
	$html .= "</form>";
	echo $html;
	atomic_footer();
}


function atomic_mainpage() {
	global $currentuser;
	atomic_header();
	if ( strcmp($currentuser["userid"], "guest") ) {
		echo "欢迎 " . $currentuser["userid"] . ". <a href='?act=logout'>注销</a><br/>";
	} else {
		echo <<<END
<form action="bbslogin.php?mainurl=atomic.php" method="post">
用户名: <input type="text" name="id" /> 密码: <input type="password" name="passwd" maxlength="39" />
<input type="submit" value="登录"/>
</form>
END;
	}
	atomic_show_boardjump();
	atomic_footer();
}

?>