<?php
require("funcs.php");
if( !defined("HAVE_BRDENV") )
	exit();

if (isset($_GET["board"]))
	$board = $_GET["board"];
else
{
        html_init("gb2312","","",1);
	html_error_quit("错误的讨论区");
	exit();
}

$brdarr = array();
$brdnum = bbs_getboard($board, $brdarr);
if ($brdnum == 0)
{
        html_init("gb2312","","",1);
	html_error_quit("错误的讨论区");
	exit();
}
$board = $brdarr["NAME"];
$usernum = $currentuser["index"];
if (bbs_checkreadperm($usernum, $brdnum) == 0)
{
        html_init("gb2312","","",1);
	html_error_quit("错误的讨论区");
	exit();
}

if( !bbs_board_have_envelop($board) )
{
	html_init("gb2312","","",1);
	html_error_quit("尚无版面导读");
	exit();
}

$brd_encode = urlencode( $board );
html_init("gb2312","","",1);
?>
<frameset rows="14,*" frameborder="NO" border="0" framespacing="0">
  <frame src="bbsenv-top.php?board=<?php echo $brd_encode; ?>" name="beName" frameborder="no" scrolling="NO" noresize marginwidth="0" marginheight="0" id="beName">
  <frame src="<?php echo "/boards/".$brd_encode."/".$defultenvfile; ?>" name="bePage" frameborder="no" noresize marginwidth="0" id="bePage">
<?php
/****
  <frame src="bbsenv-bottom.php?board=<?php echo $brd_encode; ?>" name="beInfor" frameborder="no" scrolling="NO" noresize marginwidth="0" marginheight="0" id="beInfor">
****/
?>
</frameset>
<noframes><body>
</body></noframes>
</html>