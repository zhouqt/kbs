<?php
/*
 * 精简版 web，atppp 制造。
 * Suppports UTF8. Absolutely NO javascript!
 * BUG: 没有完善处理版名含有特殊字符的情形
 */

/*
 * UTF8SP - TRUE/FALSE  (use UTF8 or GB18030)
 *        - NULL        (user can switch, default is GB18030)
 */
define('UTF8SP', NULL);
define('ARTCNT', 20);
define('MAXCHAR', 20000);
define('SUPPORT_UPLOAD', TRUE);

require("www2-funcs.php");
require("www2-board.php");
require("www2-bmp.php");
login_init();

if (is_null(UTF8SP)) {
	if (isset($_GET['utf8'])) {
		define('UTF8', (bool)$_GET['utf8']);
		setcookie("UTF8", (int)UTF8, 0, "/");
	} else if (isset($_COOKIE['UTF8'])){
		define('UTF8', (bool)$_COOKIE["UTF8"]);
	} else {
		define('UTF8', FALSE);
	}
} else {
	define('UTF8', UTF8SP);
}

if (UTF8) {
	iconv_set_encoding("internal_encoding", "gb18030");
	iconv_set_encoding("output_encoding", "UTF-8//IGNORE");
	ob_start("ob_iconv_handler");
}

$atomic_header_shown = false;
$atomic_board = false;
$atomic_brdarr = array();
$atomic_brdnum = false;
$atomic_ftype = 0;


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
	case "mail":
		atomic_mail();
		break;
	case "mailread":
		atomic_mailread();
		break;
	case "mailpost":
		atomic_mailpost();
		break;
	case "ann":
		atomic_ann();
		break;
	case "logout":
		bbs_wwwlogoff();
		delete_all_cookie();
		cache_header("nocache");
		header("Location: atomic.php");
		exit;
	default:
		atomic_mainpage();
		break;
}

function atomic_uploadable() {
	global $atomic_brdarr;
	if (!SUPPORT_UPLOAD) return FALSE;
	return bbs_is_attach_board($atomic_brdarr);
}

function atomic_header() {
	global $atomic_header_shown, $cachemode;
	if ($atomic_header_shown) return;
	if ($cachemode=="") {
		cache_header("nocache");
	}
	$atomic_header_shown = true;
	header("Content-Type: text/html; charset=".(UTF8?"UTF-8":"gb2312"));
	echo '<html><head><meta http-equiv="content-type" content="text/html; charset=' . (UTF8?"UTF-8":"gb2312") . '">'.
		 '<title>'.BBS_FULL_NAME.'</title><style>a{text-decoration:none;}</style></head><body>';
}

function atomic_footer() {
	echo "</body></html>";
}

function atomic_error($msg) {
	atomic_header();
	echo $msg . " <a href='?'>回首页</a>";
	atomic_footer();
	exit;
}

function atomic_get_input($str) {
	if (UTF8) return (iconv("UTF-8", "gb18030", $str));
	else return $str;
}

function atomic_show_boardjump() {
	global $atomic_board;
	if ($atomic_board) $bb = "当前讨论区: " . $atomic_board . ". ";
	else $bb = "";
		echo <<<END
<form action="" method="get"><input type="hidden" name="act" value="board"/>$bb
去讨论区: <input type="text" name="board" /> <input type="submit" value="Go"/> <a href='?'>回首页</a>
</form>
END;
}


function atomic_get_board($checkpost = false) {
	global $currentuser, $atomic_board, $atomic_brdarr, $atomic_brdnum, $atomic_ftype, $dir_modes;
	if (isset($_GET["board"]))
		$atomic_board = $_GET["board"];
	else{
		atomic_error("错误的讨论区");
	}
	$brdarr = array();
	$atomic_brdnum = bbs_getboard($atomic_board, $brdarr);
	$atomic_brdarr = $brdarr;
	if ($atomic_brdnum == 0){
		$boards = array();
		if (bbs_searchboard($atomic_board,0,$boards)) {
			if (sizeof($boards)==1) {
				cache_header("nocache");
				header("Location: atomic.php?act=board&board=" . urlencode($boards[0]['NAME']));
				exit;
			}
			if (sizeof($boards) > 1) {
				atomic_header();
				$html = "多个匹配的讨论区: ";
				foreach ($boards as $board) {
					if (!$board['NAME']) continue;
					$html .= '<a href="?act=board&board=' . $board['NAME'] . '">' . $board['NAME'] . '</a> ';
				}
				echo $html;
				atomic_footer();
				exit;
			}
		}
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
	if (isset($_GET["ftype"])) {
		$atomic_ftype = @intval($_GET["ftype"]);
		switch($atomic_ftype) {
			case $dir_modes["DIGEST"]:
			case $dir_modes["MARK"]:
			case $dir_modes["NORMAL"]:
				break;
			default:
				atomic_error("错误的模式");
		}
	} else {
		$atomic_ftype = $dir_modes["NORMAL"];
	}
}

function atomic_ann() {
	global $currentuser;
	$file = false;
	$path = @trim($_GET['path']);
	if (isset($_GET['file'])) {
		$path = trim($_GET['file']);
		if($path[0]=='/')
			$path = "0Announce".$path;
		else
			$path = "0Announce/".$path;
		$file = $path;
		$modfile = $file;
	} else {
		if($path[0]=='/')
			$path1 = "0Announce".$path;
		else
			$path1 = "0Announce/".$path;
		$modfile = $path1."/.Names";
	}
	if (strstr($path, '.Names') || strstr($path, '..') || strstr($path, 'SYSHome')) atomic_error('不存在该目录');

	$boardName = '';
	$articles = array();
	$path_tmp = '';
	if ($file === false) {
		$ret = bbs_read_ann_dir($path,$boardName,$path_tmp,$articles);
		switch ($ret) {
			case -1:
				atomic_error('精华区目录不存在');
			case -2:
				atomic_error('无法加载目录文件');
			case -3:
				break;
			case -9:
				atomic_error('系统错误');
			default;
		}
		$path = $path_tmp;
	} else {
		if( bbs_ann_traverse_check($path, $currentuser["userid"]) < 0 ) {
			atomic_error("错误的目录");
		}
	}
	$parent = '';
	$up_dirs = array();
	$up_cnt = bbs_ann_updirs($path,$boardName,$up_dirs);
	$cacheit = true;
	if ($up_cnt >= 2)
		$parent = $up_dirs[$up_cnt - 2];
	if ($boardName) {
		$brdArr = array();
		$boardID = bbs_getboard($boardName,$brdArr);
		$boardArr = $brdArr;
		if ($boardID) {
			$boardName = $boardArr['NAME'];
			$usernum = $currentuser['index'];
			if (bbs_checkreadperm($usernum, $boardID) == 0) {
				foundErr('不存在该目录');
			}
			bbs_set_onboard($boardID,1);
			if (!bbs_normalboard($boardName)) $cacheit = false;
		}
		else {
			$boardName = '';
		}
	}
	else {
		$boardID = 0;
	}
	
	if ($cacheit) {
		if (cache_header('public',@filemtime($modfile),300))
			return;
	}
	atomic_header(); $html = "<p>";
	if ($boardID) {
		$html .= "<a href='?act=board&board=".$boardName."'>回 ".$boardName." 版面</a> ";
	}
	if ($parent) {
		$html .= "<a href='?act=ann&path=".$parent."'>回上级目录</a> ";
	}
	$html .= "<a href='?'>回首页</a></p>";
	if ($file !== false) {
		echo $html;
		echo bbs2_readfile_text($file, MAXCHAR, 2);
	} else {
		$html .= "<pre> 编号 [类别] 标    题                               整  理       编辑日期\n";
		if (count($articles) >= 0) {
			$i = 1;
			foreach ($articles as $article) {
				switch($article['FLAG']) {
					case 0:
						continue 2;
					case 1:
						$alt = '目录';
						$url = '?act=ann&path='.urlencode($article['PATH']);
						break;
					case 2:
					case 3:
					default:
						$alt = '文件';
						$url = '?act=ann&file='.urlencode($article['PATH']);
				}
				$html .= sprintf("%5d ", $i) . "[" . $alt . "]";
				$html .= ($article['FLAG']==3)?"@":" ";
				$title = sprintf("%-37.37s",trim($article['TITLE']));
				$html .= '<a href="'.$url.'">'.htmlspecialchars($title).' </a>';
				$bm = explode(' ',trim($article['BM']));
				$html .= sprintf(" %-12.12s ", $bm[0]);
				$html .= date('Y-m-d',$article['TIME'])."\n";
				$i++;
			}
		}
		$html .= "</pre>";
		echo $html;
	}
	atomic_footer();
}

function atomic_board() {
	global $currentuser, $atomic_board, $atomic_brdarr, $atomic_brdnum, $dir_modes, $atomic_ftype;
	atomic_get_board();
	$isnormalboard = bbs_normalboard($atomic_board);
	if ($isnormalboard && (isset($_GET["page"])||$atomic_ftype) ) {
		$dotdirname = bbs_get_board_index($atomic_board, $atomic_ftype);
		if (cache_header("public",@filemtime($dotdirname),$atomic_ftype?300:10)) return;
	}
	atomic_header();
	atomic_show_boardjump();
	
	$total = bbs_countarticles($atomic_brdnum, $atomic_ftype);
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
	settype($page, "integer");
	$articles = bbs_getarticles($atomic_board, $start, ARTCNT, $atomic_ftype);
	if ($articles == FALSE)
		atomic_error("读取文章列表失败");
	
	$html = '<form action="?" method="get"><input type="hidden" name="act" value="board"/>';
	if ($atomic_ftype) {
		$html .= '<input type="hidden" name="ftype" value="' . $atomic_ftype . '"/>';
	}
	$html .= '<input type="hidden" name="board" value="'.$atomic_board.'"/>';
	$html .= '<a href="?act=post&board='.$atomic_board.'">发表</a> ';
	if (atomic_uploadable()) $html .= '<a href="?act=post&board='.$atomic_board.'&upload=1">带附件发表</a> ';
	$bl = '?act=board&board='.$atomic_board;
	if ($atomic_ftype) $bl .= '&ftype=' . $atomic_ftype;
	if ($page > 1) {
		$html .= '<a href="' . $bl .'&page=1">第一页</a> ';
		$html .= '<a href="' . $bl .'&page='.($page - 1).'">上页</a> ';
	} else {
		$html .= '第一页 上页 ';
	}
	if ($start <= $total - 20) {
		$html .= '<a href="' . $bl .'&page='.($page + 1).'">下页</a> ';
		$html .= '<a href="' . $bl .'">最后一页</a> ';
	} else {
		$html .= '下页 最后一页 ';
	}
	$html .= '<input type="submit" value="跳转到"/> 第 <input type="text" name="start" size="3" /> 篇 ';
	if ($atomic_ftype != $dir_modes["NORMAL"]) {
		$html .= "<a href='?act=board&board=".$atomic_board."'>版面</a> ";
	} else $html .= "<b>版面</b> ";
	if ($atomic_ftype != $dir_modes["DIGEST"]) {
		$html .= "<a href='?act=board&board=".$atomic_board."&ftype=".$dir_modes["DIGEST"]."'>文摘</a> ";
	} else $html .= "<b>文摘</b> ";
	if ($atomic_ftype != $dir_modes["MARK"]) {
		$html .= "<a href='?act=board&board=".$atomic_board."&ftype=".$dir_modes["MARK"]."'>保留</a> ";
	} else $html .= "<b>保留</b> ";
	$ann_path = bbs_getannpath($atomic_board);
	if ($ann_path != FALSE)	{
		if (!strncmp($ann_path,"0Announce/",10))
			$ann_path = substr($ann_path,9);
	}
	$html .= "<a href='?act=ann&path=".$ann_path."'>精华</a> ";
	$html .= "</form>";

	$html .= "<pre> 编号   刊 登 者     日  期  文章标题<br/>";
	$i = 0;
	foreach ($articles as $article)	{
		$title = $article["TITLE"];
		if (strncmp($title, "Re: ", 4) != 0)
			$title = "● " . $title;

		$flags = $article["FLAGS"];

		if (!strncmp($flags,"D",1)||!strncmp($flags,"d",1)) {
			$html .= " [提示] ";
		} else {
			$html .= sprintf("%5d ", ($start+$i));
			if ($flags[0] == 'N' || $flags[0] == '*'){
				$html .= " "; //$flags[0];  //不要未读标记 windinsn
			} else{
				$html .= $flags[0];
			}
			$html .= " ";
		}
		$html .= sprintf("%-12.12s ", $article["OWNER"]);
		$html .= strftime("%b %e ", $article["POSTTIME"]);
		$html .= $flags[3];
		$articleurl = "?act=article&board=".$atomic_board."&id=".$article["ID"];
		if ($atomic_ftype) {
			$articleurl .= "&ftype=" . $atomic_ftype . "&num=" . ($start+$i);
		}
		$html .= "<a href='".$articleurl."'>".htmlspecialchars($title)." </a><br/>";
		$i++;
	}
	$html .= "</pre>";

	echo $html;
	atomic_footer();
}


function atomic_article() {
	global $currentuser, $atomic_board, $atomic_brdarr, $atomic_brdnum, $dir_modes, $atomic_ftype;
	atomic_get_board();
	$id = @intval($_GET["id"]);
	if ($id <= 0) atomic_error("错误的文章号");

	$url = "?act=article&board=" . $atomic_board . "&id=";
	@$ptr=$_GET["p"];
	// 同主题的指示在这里处理
	if ($ptr == "tn" || $ptr == "tp") {
		$articles = bbs_get_threads_from_id($atomic_brdnum, $id, $dir_modes["NORMAL"],($ptr == "tp")?-1:1);
		if ($articles == FALSE)
			$redirt_id = $id;
		else
			$redirt_id = $articles[0]["ID"];
		header("Location: atomic.php" . $url . $redirt_id);
		exit;
	}

	$total = bbs_countarticles($atomic_brdnum, $atomic_ftype);
	if ($total <= 0) {
		atomic_error("错误的文章号,原文可能已经被删除");
	}

	if (!$atomic_ftype) {
		$articles = array ();
		$num = bbs_get_records_from_id($atomic_board, $id, $atomic_ftype, $articles);
		if ($num <= 0) atomic_error("错误的文章号,原文可能已经被删除");
		$article = $articles[1];
	} else {
		$num = @intval($_GET["num"]);
		if (($num <= 0) || ($num > $total)) atomic_error("错误的文章号,原文可能已经被删除");
		if (($articles = bbs_getarticles($atomic_board, $num, 1, $atomic_ftype)) === false) atomic_error("错误的文章号,原文可能已经被删除");
		if ($id != $articles[0]["ID"]) atomic_error("错误的文章号,原文可能已经被删除");
		$article = $articles[0];
	}
	$filename = bbs_get_board_filename($atomic_board, $article["FILENAME"]);
	$isnormalboard = bbs_normalboard($atomic_board);
	if ($isnormalboard) {
		if (cache_header("public",@filemtime($filename),300))
			return;
	}

	if (!$atomic_ftype) {
		$idx = ($ptr == 'p') ? 0 : (($ptr == 'n') ? 2 : -1);
		if (($idx != -1) && $articles[$idx]["ID"] != 0) {
			header("Location: atomic.php" . $url . $articles[$idx]["ID"]);
			exit;
		}
		bbs_brcaddread($atomic_board, $article["ID"]);
	}

	atomic_header();
	$html = '<p>';
	if (!$atomic_ftype) {
		$html .= '<a href="' . $url . $article["ID"] . '&p=tn">主题下篇</a> ';
		$html .= '<a href="' . $url . $article["GROUPID"] . '">楼主</a> ';
		$html .= '<a href="?act=post&board='.$atomic_board.'">发表</a> <a href="?act=post&board='.$atomic_board.'&reid='.$id.'">回复</a> ';
		if (atomic_uploadable()) $html .= '<a href="?act=post&board='.$atomic_board.'&reid='.$id.'&upload=1">带附件回复</a> ';
		$html .= '<a href="' . $url . $article["ID"] . '&p=p">上篇</a> ';
		$html .= '<a href="' . $url . $article["ID"] . '&p=n">下篇</a> ';
		$html .= '<a href="' . $url . $article["ID"] . '&p=tp">主题上篇</a> ';
		$html .= '<a href="' . $url . $article["REID"] . '">溯源</a> ';
	}
	$html .= '<a href="?act=board&board='.$atomic_board.'&page='.intval(($num + ARTCNT - 1) / ARTCNT).
			 ($atomic_ftype?"&ftype=".$atomic_ftype:"").'">回版面</a> ';
	$ourl = 'bbscon.php?bid=' . $atomic_brdnum . '&id=' . $article["ID"];
	if ($atomic_ftype) $ourl .= "&ftype=".$atomic_ftype."&num=".$num;
	$html .= '<a href="' . $ourl . '">原文</a> ';
	$html .= '</p>';
	echo $html;
	echo bbs2_readfile_text($filename, MAXCHAR, 2);
	if ($article["ATTACHPOS"]) echo '<p>本文含有附件: <a href="' . $ourl . '">原文链接</a></p>';
	atomic_footer();
}


function atomic_post() {
	global $currentuser, $atomic_board, $atomic_brdarr, $atomic_brdnum, $dir_modes, $utmpnum;
	atomic_get_board(TRUE);

	$reid = (isset($_GET["reid"])) ? @intval($_GET["reid"]) : 0;
	if ($reid > 0) {
		if(bbs_is_noreply_board($atomic_brdarr))
			atomic_error("本版只可发表文章,不可回复文章!");

		$articles = array();
		$num = bbs_get_records_from_id($atomic_board, $reid, $dir_modes["NORMAL"], $articles);
		if ($num == 0) atomic_error("错误的 Re 文编号");
		if ($articles[1]["FLAGS"][2] == 'y')
			atomic_error("该文不可回复!");
	}
	if (isset($_GET["post"])) {
		if (!isset($_POST["title"])) atomic_error("没有指定文章标题!");
		if (!isset($_POST["text"])) atomic_error("没有指定文章内容!");
		$title = atomic_get_input(trim($_POST["title"]));
		$text = atomic_get_input($_POST["text"]);
		if (isset($_GET["reid"])) $reID = @intval($_GET["reid"]);
		else $reID = 0;
		$outgo = bbs_is_outgo_board($atomic_brdarr) ? 1 : 0;
		$anony = 0;
		$attmsg = "";
		if (atomic_uploadable() && isset($_FILES['attachfile'])) {
			$attpost = $_FILES['attachfile'];
			@$errno = $attpost['error'];
			switch ($errno) {
			case UPLOAD_ERR_OK:
				$ofile = $attpost['tmp_name'];
				if (!file_exists($ofile)) {
					$attmsg = "文件传输出错！";
					break;
				}
				$oname = $attpost['name'];
				$htmlname = htmlspecialchars(my_basename($oname));
				if (!is_uploaded_file($ofile)) {
					die;
				}
				if (compress_bmp($ofile, $oname)) {
					$attmsg .= "过大 BMP 图片 " . $htmlname . " 被自动转换成 PNG 格式。<br/>";
				}
				$ret = bbs_upload_add_file($ofile, $oname);
				if ($ret) {
					$attmsg .= bbs_error_get_desc($ret);
				} else {
					$attmsg .= $htmlname . "上传成功！<br/>";
				}
				break;
			case UPLOAD_ERR_INI_SIZE:
			case UPLOAD_ERR_FORM_SIZE:
				$attmsg = "文件超过预定的大小" . sizestring(BBS_MAXATTACHMENTSIZE) . "字节";
				break;
			case UPLOAD_ERR_PARTIAL:
				$attmsg = "文件传输出错！";
				break;
			case UPLOAD_ERR_NO_FILE:
				$attmsg = "没有文件上传！";
				break;
			default:
				$attmsg = "未知错误";
			}
		}
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
			atomic_error("两次发文/信间隔过密,请休息几秒再试!");	
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
		case -21:
			atomic_error("您的积分不符合当前讨论区的设定, 暂时无法在当前讨论区发表文章...");
			break;
		}
		atomic_header();
		$url = "?act=board&board=" . $atomic_board;
		if (isset($attmsg)) echo $attmsg . "<br/>";
		if ($ret == -10) {
			echo "<p>很抱歉，本文可能含有不当内容，需经审核方可发表。<br/><br/>" .
				  "根据《帐号管理办法》，被系统过滤的文章视同公开发表。请耐心等待<br/>" .
				  "站务人员的审核，不要多次尝试发表此文章。<br/><br/>" .
				  "如有疑问，请致信 SYSOP 咨询。</p>";
			echo "返回<a href='$url'>版面文章列表</a>";
		} else {
			echo "发文成功！本页面将在3秒后自动返回<a href='$url'>版面文章列表</a><meta http-equiv='refresh' content='3; url=" . $url . "'/>";
		}
		atomic_footer();
		return;
	}
	if ($reid) {
		if(!strncmp($articles[1]["TITLE"],"Re: ",4))$nowtitle = $articles[1]["TITLE"];
		else $nowtitle = "Re: " . $articles[1]["TITLE"];
	} else $nowtitle = "";
	atomic_header();
	$html = "<p><a href='?act=board&board=" . $atomic_board . "'>" . $atomic_board . " 版</a>发表文章</p>";
	$html .= "<form action='?act=post&board=" . $atomic_board . "&reid=" . $reid . "&post=1' method='post'" .(isset($_GET['upload']) ? " enctype='multipart/form-data'>" : ">");
	$html .= '标题: <input type="text" name="title" size="40" maxlength="100" value="' . ($nowtitle?htmlspecialchars($nowtitle,ENT_QUOTES)." ":"") . '"/><br/>';
	$html .= '<textarea name="text" rows="20" cols="80" wrap="physical">';
	if($reid > 0){
		$filename = bbs_get_board_filename($atomic_board, $articles[1]["FILENAME"]);
		$q = @bbs_get_quote($filename);
		if ($q) $html .= "\n".$q;
	}
	$html .= '</textarea><br/>';
	if (isset($_GET['upload'])) $html .= '<input name="attachfile" type="file"/><br/>';
	$html .= '<input type="submit" value="发表" /></form>';
	echo $html;
	atomic_footer();
}

function atomic_mail_header() {
	global $currentuser;
	if ( ! strcmp($currentuser["userid"], "guest") ) {
		atomic_error("游客没有信箱");
	}
	atomic_header();
	$html = "<p>" . $currentuser["userid"] . " 的信箱 <a href='?'>回首页</a></p>";
	echo $html;
}

function atomic_mail() {
	global $currentuser;
	atomic_mail_header();
	$mail_fullpath = bbs_setmailfile($currentuser["userid"],".DIR");
	$mail_num = bbs_getmailnum2($mail_fullpath);
	if($mail_num <= 0 || $mail_num > 30000) atomic_error("读取邮件数据失败!");
	$start = (isset($_GET["start"])) ? @intval($_GET["start"]) : 999999;
	$num = ARTCNT;
	if ($start > $mail_num - ARTCNT + 1) $start = $mail_num - ARTCNT + 1;
	if ($start <= 0) {
		$start = 1;
		if ($num > $mail_num) $num = $mail_num;
	}
	$maildata = bbs_getmails($mail_fullpath, $start - 1, $num);
	if ($maildata == FALSE) atomic_error("读取邮件数据失败!");
	
	$html = '<form action="?" method="get"><input type="hidden" name="act" value="mail"/>';
	$html .= '<a href="?act=mailpost">写信</a> ';
	if ($start > 1) {
		$i = $start - ARTCNT;
		if ($i < 1) $i = 1;
		$html .= '<a href="?act=mail&start=1">第一页</a> ';
		$html .= '<a href="?act=mail&start='.$i.'">上一页</a> ';
	} else {
		$html .= '第一页 上一页 ';
	}
	if ($start <= $mail_num - ARTCNT) {
		$i = $start + ARTCNT;
		if ($i > $mail_num) $i = $mail_num;
		$html .= '<a href="?act=mail&start='.$i.'">下一页</a> ';
		$html .= '<a href="?act=mail">最后一页</a> ';
	} else {
		$html .= '下一页 最后一页 ';
	}
	$html .= '<input type="submit" value="跳转到"/> 第 <input type="text" name="start" size="3" /> 篇</form>';
	$html .= '<pre> 编号    发信者       日  期  标  题<br/>';
	for ($i = 0; $i < count($maildata); $i++) {
		$article = $maildata[$i];
		$title = $article["TITLE"];
		if (strncmp($title, "Re: ", 4) != 0)
			$title = "★ " . $title;

		$html .= sprintf("%5d ", ($start+$i));
		$html .= $maildata[$i]["FLAGS"];

		$html .= sprintf(" %-12.12s ", $article["OWNER"]);
		$html .= strftime("%b %e ", $article["POSTTIME"]);
		$html .= ($maildata[$i]["ATTACHPOS"]>0) ? "@" : " ";
		$html .= "<a href='?act=mailread&num=".($start+$i)."'>".htmlspecialchars($title)." </a> (".sizestring($maildata[$i]['EFFSIZE']).")<br/>";
	}
	$html .= "</pre>";
	echo $html;
	atomic_footer();
}

function atomic_mailread() {
	global $currentuser;
	atomic_mail_header();
	$mail_fullpath = bbs_setmailfile($currentuser["userid"],".DIR");
	$mail_num = bbs_getmailnum2($mail_fullpath);
	if($mail_num <= 0 || $mail_num > 30000) atomic_error("读取邮件数据失败!");
	if (isset($_GET["num"]))
		$num = @intval($_GET["num"]);
	else {
		atomic_error("错误的参数");
	}
	if ($num <= 0 || $num > $mail_num) atomic_error("错误的参数");
	$articles = array ();
	if( bbs_get_records_from_num($mail_fullpath, $num-1, $articles) ) {
		$filename = bbs_setmailfile($currentuser["userid"], $articles[0]["FILENAME"]);
	}else{
		atomic_error("错误的参数");
	}

	$html = "<p>";
	$html .= '<a href="?act=mailpost">写信</a> <a href="?act=mailpost&num='.$num.'">回信</a> ';
	if($num > 1){
		$html .= '<a href="?act=mailread&num=' . ($num-1) . '">上一篇</a> ';
	}
	$html .= '<a href="?act=mail&start=' . $num . '">收件箱</a> ';
	if($num < $mail_num){
		$html .= '<a href="?act=mailread&num=' . ($num+1) . '">下一篇</a> ';
	}
	$html .= "</p>";
	echo $html;
	echo bbs2_readfile_text($filename, 0, 2);
	bbs_setmailreaded($mail_fullpath,$num-1);
	atomic_footer();
}

function atomic_mailpost() {
	global $currentuser;
	atomic_mail_header();
	if (!bbs_can_send_mail()) atomic_error("您不能发送信件");

	$num = (isset($_GET["num"])) ? @intval($_GET["num"]) : 0;
	$mail_fullpath = bbs_setmailfile($currentuser["userid"],".DIR");
	if ($num > 0) {
		$articles = array ();
		if( bbs_get_records_from_num($mail_fullpath, $num-1, $articles) ) {
			$title = $articles[0]["TITLE"];
			$receiver = $articles[0]["OWNER"];
			$shortfilename = $articles[0]["FILENAME"];
			$filename = bbs_setmailfile($currentuser["userid"], $shortfilename);
		}else{
			atomic_error("错误的参数");
		}
		
	}
	if (isset($_GET["post"])) {
		$title = atomic_get_input(trim(@$_POST["title"]));
		if (!$title) $title = '无主题';
		$content = atomic_get_input(@$_POST["text"]);
		$sig = $currentuser["signature"];
		$backup = (bbs_is_save2sent() != 0);
		
		if ($num > 0) {
			$ret = bbs_postmail($mail_fullpath, $shortfilename, $num-1, $title, $content, $sig, $backup);
		} else {
			$incept = trim(@$_POST['userid']);
			if (!$incept)
				atomic_error("请输入收件人ID");
			$lookupuser = array();
			if (!bbs_getuser($incept,$lookupuser))
				atomic_error("错误的收件人ID");
			$incept = $lookupuser['userid'];
		
			if (!strcasecmp($incept,'guest')) atomic_error("不能发信给guest");
			
			$ret = bbs_postmail($incept,$title,$content,$sig,$backup);
		}

		if ($ret < 0)  {
			switch($ret) {
				case -1:
				case -2:
					atomic_error("无法创建文件");
					break;
				case -3:
					atomic_error($incept." 拒收您的邮件");
					break;
				case -4:
					atomic_error($incept." 的信箱已满");
					break;
				case -5:
					atomic_error("两次发文/信间隔过密,请休息几秒再试!");	
					break;
				case -6:
					atomic_error("添加邮件列表出错");
					break;
				case -7:
					atomic_error("邮件发送成功，但未能保存到发件箱");
					break;
				case -8:
					atomic_error("找不到所回复的原信。");
					break;
				case -100:
					atomic_error("错误的收件人ID");
					break;
				default:
					atomic_error("系统错误，请联系管理员");
			}
		}
		
		if ($num > 0) {
			$url = "?act=mailread&num=".$num;
			echo "发送成功！本页面将在3秒后自动返回<a href='$url'>原信件</a><meta http-equiv='refresh' content='3; url=" . $url . "'/>";
		} else {
			$url = "?act=mail";
			echo "发送成功！本页面将在3秒后自动返回<a href='$url'>信件列表</a><meta http-equiv='refresh' content='3; url=" . $url . "'/>";
		}
		atomic_footer();
		return;
	}
	
	if ($num > 0) {
		if(!strncmp($title,"Re: ",4)) $nowtitle = $title;
		else $nowtitle = "Re: " . $title;
	} else {
		$nowtitle = "";
	}
	$html = "<form action='?act=mailpost&num=".$num."&post=1' method='post'>";
	$html .= '标题: <input type="text" name="title" size="40" maxlength="100" value="' . ($nowtitle?htmlspecialchars($nowtitle,ENT_QUOTES)." ":"") . '"/><br/>';
	if ($num > 0) {
		$html .= "收件人: " . $receiver . "<br/>";
	} else {
		$html .= '收件人: <input type="text" name="userid"/><br/>';
	}
	$html .= '<textarea name="text" rows="20" cols="80" wrap="physical">';
	if ($num > 0) {
		$html .= "\n\n【 在 " . $receiver . " 的来信中提到: 】\n";
		$fp = fopen($filename, "r");
		if ($fp) {
			$lines = 0;
			for ($i = 0; $i < 4; $i++) {
				if (($buf = fgets($fp,500)) == FALSE)
					break;
			}
			while (1) {
				if (($buf = fgets($fp,500)) == FALSE)
					break;
				if (strncmp($buf, ": 【", 4) == 0)
					continue;
				if (strncmp($buf, ": : ", 4) == 0)
					continue;
				if (strncmp($buf, "--\n", 3) == 0)
					break;
				if (strncmp($buf,'\n',1) == 0)
					continue;
				if (++$lines > 10) {
					$html .= ": ...................\n";
					break;
				}
				$html .= ": ". htmlspecialchars($buf);
			}
			fclose($fp);
		}
	}
	$html .= '</textarea><br/><input type="submit" value="发送" /></form>';
	echo $html;
	atomic_footer();
}

function atomic_mainpage() {
	global $currentuser;
	atomic_header();
	if ( strcmp($currentuser["userid"], "guest") ) {
		$html = "<p>欢迎 " . $currentuser["userid"] . ". <a href='?act=logout'>注销</a></p>";
		$select = 0;
		if(bbs_load_favboard($select)!=-1) {
			$boards = bbs_fav_boards($select, 1);
			if ($boards) {
				$html .= "<p>顶层收藏夹: ";
				$brd_name = $boards["NAME"];
				$brd_flag= $boards["FLAG"];
				$brd_bid= $boards["BID"];
				$rows = sizeof($brd_name);
				for ($i = 0; $i < $rows; $i++) {
					if ($brd_bid[$i] == -1) continue;
					if ($brd_flag[$i] == -1 ) continue;
					if ($brd_flag[$i] & BBS_BOARD_GROUP) continue;
					$html .= '<a href="?act=board&board=' . $brd_name[$i] . '">' . $brd_name[$i] . '</a> ';
				}
				$html .= '</p>';
			}
		}
		$oldtotal = 0; $oldunread = 0;
		if (!bbs_getmailnum($currentuser["userid"],$total,$unread, $oldtotal, $oldunread)) {
			$unread = $total = 0;
		}
		$html .= "<p><a href='?act=mail'>信箱</a>: $total 封, 新信: $unread 封. <a href='?act=mailpost'>写信</a></p>";
		echo $html;
	} else {
		echo <<<END
<form action="bbslogin.php?mainurl=atomic.php" method="post">
用户名: <input type="text" name="id" /> 密码: <input type="password" name="passwd" maxlength="39" />
<input type="submit" value="登录"/>
</form>
END;
	}
	atomic_show_boardjump();
	if (is_null(UTF8SP)) {
		$url = $_SERVER['REQUEST_URI'];
		if (strstr($url, 'utf8=')) $url = substr($url, 0, strlen($url)-1);
		else if (!strstr($url, '?')) $url.= '?utf8=';
		echo "UTF8: <a href='" . $url . (UTF8 ? "0" : "1") . "'>" . (UTF8 ? "ON" : "OFF") . "</a>. ";
	} else {
		echo "UTF8: " . (UTF8 ? "ON" : "OFF") . ". ";
	}
	echo "文章显示长度限制: " . MAXCHAR . ".";
	atomic_footer();
}

?>
