<?php
/*
 * edit article's title
 * @author: windinsn apr 28,2004
 */
require("funcs.php");
login_init();

html_init("gb2312","","",1);
if ($loginok != 1)
	html_nologin();

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
bbs_set_onboard($brcnum,1);
$usernum = $currentuser["index"];
if (bbs_checkreadperm($usernum, $brdnum) == 0)
	html_error_quit("错误的讨论区");

if(bbs_checkpostperm($usernum, $brdnum) == 0) 
{
	if (!strcmp($currentuser["userid"],"guest"))
		html_error_quit("请先注册帐号");
	else 
		html_error_quit("错误的讨论区或者您无权在此讨论区发表文章");
}

if (!isset($_GET["mode"]))
	$mode = $dir_modes["NORMAL"];
else
{
	if($_GET["mode"] != $dir_modes["NORMAL"] && $_GET["mode"] != $dir_modes["ZHIDING"] && $_GET["mode"] != $dir_modes["DIGEST"])
		html_error_quit("错误的阅读模式");
	$mode = $_GET["mode"];
}

$articles = array ();
$num = bbs_get_records_from_id($board, $id, $mode, $articles);
if($num==0)
	html_error_quit("错误的文章号,原文可能已经被删除");

if($_POST["title"])
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
?>
<br /><br /><br />
<table cellpadding=3 cellspacing=1 align=center class=TableBorder1>
<tr align=center><th width="100%">标题修改成功！</td>
</tr><tr><td width="100%" class=TableBody1>
本页面将在3秒后自动返回版面文章列表<meta HTTP-EQUIV=REFRESH CONTENT='3; URL=bbsdoc.php?board=<?php echo $brd_encode; ?>' >，<b>您可以选择以下操作：</b><br><ul>
<li><a href="<?php echo MAINPAGE_FILE; ?>">返回首页</a></li>
<li><a href="/bbsdoc.php?board=<?php   echo $brd_encode; ?>">返回<?php   echo $brdarr['DESC']; ?></a></li>
<li><a href="/bbscon.php?board=<?php   echo $brd_encode; ?>&id=<?php echo $id; ?>">返回《<?php  echo $_POST["title"]; ?>》</a></li>
</ul></td></tr></table>
<?php		
	}
?>

<?php
}
else
{
?>	
<br /><br /><br /><br /><center>
<form action="<?php echo $_SERVER["PHP_SELF"]; ?>?board=<?php echo $brd_encode; ?>&id=<?php echo $id; ?>&mode=<?php echo $mode; ?>" method="post" />
新文章标题：
<input type="text" name="title" id="title" size="40" value="<?php echo htmlspecialchars($articles[1]["TITLE"]); ?>" />
<input type="submit" value="修改" />
</form>	</center>
<?php	
}
html_normal_quit();
?>