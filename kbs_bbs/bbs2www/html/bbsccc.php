<?php
/**
 * 转贴
 * windinsn
 */
require('funcs.php');
require('board.inc.php');
login_init();
html_init("gb2312","","",1);

if ($loginok != 1)
    html_nologin();

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
bbs_board_header($brdarr)
?>
<center><br/>
<?php
if (isset($_GET['do'])) {
    $target = trim(ltrim($_POST['target']));
    if (!$target)
        html_error_quit("请输入转入的讨论区");
    $outgo = isset($_POST['outgo'])?1:0;
    switch (bbs_docross($board,$id,$target,$outgo)) {
        case 0:
?>
<b>转贴成功!</b><br /><br /><br />
[<a href="/bbsdoc.php?board=<?php echo $target; ?>">进入 <?php echo $target; ?> 讨论区</a>]
[<a href="/bbscon.php?board=<?php echo $brd_encode; ?>&id=<?php echo $id; ?>&ftype=<?php echo $ftype; ?>"><?php echo htmlspecialchars($articles[1]["TITLE"]); ?>
</b></a>]
<?php
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
              html_error_quit($target." 讨论区不能粘贴附件");
              break;
        default:
              html_error_quit("系统错误，请联系管理员");
    }
}
else {
?>
<p>转载文章：<b>
<a href="/bbscon.php?board=<?php echo $brd_encode; ?>&id=<?php echo $id; ?>&ftype=<?php echo $ftype; ?>"><?php echo htmlspecialchars($articles[1]["TITLE"]); ?>
</b></a>
</p><br />
<form action="<?php echo $_SERVER['PHP_SELF']; ?>?do&board=<?php echo $brd_encode; ?>&id=<?php echo $id; ?>" method="post" />
请输入要转入的讨论区
<input type="text" name="target" size="18" maxlength="20" />
<input type="checkbox" name="outgo" checked />转信
<input type="submit" value="转贴" />
</form>
<?php
}
?>
<br /><br />
<br /></center>
<?php
bbs_board_foot($brdarr,"CROSS");
html_normal_quit();
?>