<?php
require("www2-funcs.php");
login_init();
bbs_session_modify_user_mode(BBS_MODE_EDIT);
assert_login();

if (isset($_GET["board"]))
	$board = $_GET["board"];
else
	html_error_quit("错误的讨论区");

$id = intval($_GET["id"]);
if(!$id)
	html_error_quit("错误的文章");

// 检查用户能否阅读该版
$brdarr = array();
$brdnum = bbs_getboard($board, $brdarr);
if ($brdnum == 0)
	html_error_quit("错误的讨论区");

$board=$brdarr["NAME"];
$brd_encode = urlencode($board);
bbs_set_onboard($brdnum,1);
$usernum = $currentuser["index"];
if (bbs_checkreadperm($usernum, $brdnum) == 0)
	html_error_quit("错误的讨论区");

if(bbs_checkpostperm($usernum, $brdnum) == 0) 
	html_error_quit("错误的讨论区或者您无权在此讨论区发表文章");

$mode = $dir_modes["NORMAL"]; /* TODO: support for other modes? */

$articles = array ();
$num = bbs_get_records_from_id($board, $id, $mode, $articles);
if($num==0)
	html_error_quit("错误的文章号,原文可能已经被删除");

bbs_board_nav_header($brdarr, "修改文章标题");

if(isset($_POST["title"]))
{
	$ret = bbs_edittitle($board,$id,$_POST["title"],$mode);
	if($ret != 0)
	{
		switch($ret)
		{
			case -1:
				html_error_quit("错误的讨论区");
				break;
			case -2:
				html_error_quit("对不起，该讨论区不能修改标题");
				break;
			case -3:
				html_error_quit("对不起，该讨论区为只读讨论区");
				break;
			case -4:
				html_error_quit("错误的文章号");
				break;
			case -5:
				html_error_quit("对不起，您已被停止在".$board."版的发文权限");
				break;
			case -6:
				html_error_quit("对不起，您无权修改本文");
				break;
			case -7:
				html_error_quit("标题含有不雅用字");
				break;
			case -8:
				html_error_quit("对不起，当前模式无法修改标题");
				break;
			case -9:
				html_error_quit("标题过长");
				break;
			default:
				html_error_quit("系统错误，请联系管理员");
		}
	}
	else
	{
		html_success_quit("标题修改成功！<br/>" . 
			"本页面将在3秒后自动返回版面文章列表<meta http-equiv=refresh content='3; url=bbsdoc.php?board=" . $brd_encode . "'/>",
			array("<a href='" . MAINPAGE_FILE . "'>返回首页</a>", 
			"<a href='bbsdoc.php?board=" . $brd_encode . "'>返回 " . $brdarr['DESC'] . "</a>",
			"<a href='bbscon.php?board=" . $brd_encode . "&id=" . $id . "'>返回《" . $_POST["title"] . "》</a>"));
	}
}
else
{
?>	
<form action="<?php echo $_SERVER["PHP_SELF"]; ?>?board=<?php echo $brd_encode; ?>&id=<?php echo $id; ?>" method="post" class="large"/>
	<fieldset><legend>修改文章标题</legend>
		<div class="inputs">
			<label>新文章标题:</label>
			<input type="text" name="title" id="sfocus" size="40" value="<?php echo htmlspecialchars($articles[1]["TITLE"]); ?>" />
		</div>
	</fieldset>
	<div class="oper"><input type="submit" value="修改" /></div>
</form>
<?php	
}
page_footer();
?>
