<?php
require("funcs.php");
login_init();

if( !defined("HAVE_BRDENV") )
	exit();

if (isset($_GET["board"]))
	$board = $_GET["board"];
else{
        html_init("gb2312","","",1);
	html_error_quit("错误的讨论区");
	exit();
}

$brdarr = array();
$brdnum = bbs_getboard($board, $brdarr);
if ($brdnum == 0){
        html_init("gb2312","","",1);
	html_error_quit("错误的讨论区");
	exit();
}
$usernum = $currentuser["index"];
if (bbs_checkreadperm($usernum, $brdnum) == 0){
        html_init("gb2312","","",1);
	html_error_quit("错误的讨论区");
	exit();
}

$brd_encode = urlencode($brdarr["NAME"]);
html_init("gb2312","","",1);
?>
<body topmargin="0" leftmargin="0">
<p class="b2">
<a href="<?php echo MAINPAGE_FILE; ?>" class="b2" target="f3"><font class="b2"><?php echo BBS_FULL_NAME; ?></font></a>
-
<?php
$sec_index = get_secname_index($brdarr["SECNUM"]);
if ($sec_index >= 0)
{	
?>
<a href="/bbsboa.php?group=<?php echo $sec_index; ?>" class="b2" target="f3"><font class="b2"><?php echo $section_names[$sec_index][0]; ?></font></a>
<?php
}
?>
-
<a href="/bbsdoc.php?board=<?php echo $brd_encode; ?>" class="b2" target="f3"><?php echo $brdarr["NAME"]; ?>版</a>
(
<a href="/bbsnot.php?board=<?php echo $brd_encode; ?>" class="b2" target="f3"><font class="b2">进版画面</font></a>
|
<a href="/bbsfav.php?bname=<?php echo $brdarr["NAME"]; ?>&select=0" class="b2" target="f3"><font class="b2">添加到收藏夹</font></a>
)
<a href="/bbsdoc.php?board=<?php echo $brd_encode; ?>&env" target="f3"><font color=red><strong>进入版面</strong></font></a>
</p>
<?php
html_normal_quit();
?>
