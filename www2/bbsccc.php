<?php
require('www2-funcs.php');
login_init();
bbs_session_modify_user_mode(BBS_MODE_READING);
assert_login();

if (isset($_GET['board']))
	$board = $_GET['board'];
else
	html_error_quit('错误的讨论区');

$brdarr = array();
$bid = bbs_getboard($board,$brdarr);
if (!$bid)
	html_error_quit('错误的讨论区');
$board = $brdarr['NAME'];
$brd_encode = urlencode($board);

if (isset($_GET['id']))
	$id = intval($_GET['id']);
else
	html_error_quit('错误的文章ID');


if (!bbs_normalboard($board))
	if (bbs_checkreadperm($currentuser["index"], $bid) == 0)
		html_error_quit("错误的讨论区");

$ftype = $dir_modes["NORMAL"];
$articles = array ();
$num = bbs_get_records_from_id($board, $id, $ftype, $articles);
if ($num == 0)
	html_error_quit("错误的文章号,原文可能已经被删除");
$id = $articles[1]["ID"];
bbs_board_nav_header($brdarr, "文章转贴");

if (isset($_GET['do'])) {
	$target = trim(ltrim($_POST['target']));
	if (!$target)
		html_error_quit("请输入转入的讨论区");
	$outgo = isset($_POST['outgo'])?1:0;
	switch (bbs_docross($board,$id,$target,$outgo)) {
		case 0:
			html_success_quit("转贴成功！",
			array("<a href='bbsdoc.php?board=" . $target . "'>进入 " . $target . " 讨论区</a>",
			"<a href='bbsdoc.php?board=" . $brd_encode . "'>返回 " . $brdarr['DESC'] . "</a>",
			"<a href='bbscon.php?bid=" . $bid . "&id=" . $id . "'>返回《" . htmlspecialchars($articles[1]["TITLE"]) . "》</a>"));
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
		case -11:
			$prompt = "转贴成功！<br/><br/>但是很抱歉，本文可能含有不当内容，需经审核方可发表。<br/><br/>" .
            	      "根据《帐号管理办法》，被系统过滤的文章视同公开发表。请耐心等待<br/>" .
                	  "站务人员的审核，不要多次尝试发表此文章。<br/><br/>" .
            		  "如有疑问，请致信 SYSOP 咨询。";
			html_success_quit($prompt,
				array("<a href='bbsdoc.php?board=" . $target . "'>进入 " . $target . " 讨论区</a>",
				"<a href='bbsdoc.php?board=" . $brd_encode . "'>返回 " . $brdarr['DESC'] . "</a>"));
			break;
		case -21:
			html_error_quit("您的积分不符合 ".$target." 讨论区的设定, 暂时无法发表文章...");
			break;
		default:
	}
	html_error_quit("系统错误，请联系管理员");
}
?>
<form action="<?php echo $_SERVER['PHP_SELF']; ?>?do&board=<?php echo $brd_encode; ?>&id=<?php echo $id; ?>" method="post" class="medium"/>
	<fieldset>
		<legend>转贴文章：<a href="bbscon.php?bid=<?php echo $bid; ?>&id=<?php echo $id; ?>"><?php echo htmlspecialchars($articles[1]["TITLE"]); ?></a></legend>
		<div class="inputs">
			<label>请输入要转入的讨论区:</label>
			<input type="text" name="target" size="18" maxlength="20" id="sfocus"/>
			<input type="checkbox" name="outgo" checked />转信
		</div>
	</fieldset>
	<div class="oper"><input type="submit" value="转贴" /></div>
</form>
<?php
page_footer();
?>
