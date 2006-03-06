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
bbs_board_nav_header($brdarr, "文章推荐");
$confirmed = isset($_GET['do']) ? 1 : 0;
	switch (bbs_docommend($board,$id,$confirmed)) {
		case 0:
			if ($confirmed) {
				html_success_quit("推荐成功！",
				array("<a href='bbsdoc.php?board=" . $brd_encode . "'>返回 " . $brdarr['DESC'] . "</a>",
				"<a href='bbscon.php?bid=" . $bid . "&id=" . $id . "'>返回《" . htmlspecialchars($articles[1]["TITLE"]) . "》</a>"));
			} else {
				// show recommend form below
			}
			break;
		case -1:
			html_error_quit("对不起，您没有这篇文章的推荐权限");
			break;
		case -2:
			html_error_quit("讨论区错误");
			break;
		case -3:
			html_error_quit("错误的文章号,原文可能已经被删除");
			break;
		case -4:
			html_error_quit("本文章已经推荐过，感谢您的热心推荐");
			break;
		case -5:
			html_error_quit("对不起，请勿推荐内部版面文章");
			break;
		case -6:
			html_error_quit("对不起，您被停止了推荐的权力");
			break;
		case -7:
			html_error_quit("推荐系统错误");
			break;
		default:
			html_error_quit("系统错误，请联系管理员");
	}
?>
<form action="<?php echo $_SERVER['PHP_SELF']; ?>?do&board=<?php echo $brd_encode; ?>&id=<?php echo $id; ?>" method="post" class="medium"/>
	<fieldset>
		<legend>推荐文章：<?php echo $articles[1]["OWNER"]; ?> 的 <a href="bbscon.php?bid=<?php echo $bid; ?>&id=<?php echo $id; ?>"><?php echo htmlspecialchars($articles[1]["TITLE"]); ?></a></legend>
		<div class="oper">
			<input type="submit" value="确认" />&nbsp;&nbsp;<input type="button" value="取消" onclick="history.go(-1)" />
		</div>
	</fieldset>
</form>
<?php
page_footer();
?>
