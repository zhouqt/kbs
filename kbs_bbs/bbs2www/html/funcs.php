<?php
	/**
	 * File included by all other php scripts.
	 * $Id$
	 */
dl("../libexec/bbs/libphpbbslib.so");
global $SQUID_ACCL;
global $BBS_PERM_POSTMASK;
global $BBS_PERM_NOZAP;
global $BBS_HOME;
global $BBS_FULL_NAME;
//$fromhost=$_SERVER["REMOTE_ADDR"];
global $fromhost;
global $fullfromhost;
global $loginok;
//global $currentuinfo;
global $currentuinfo_num;
//global $currentuser;
global $currentuuser_num;
$currentuinfo=array ();
$currentuser=array ();
$section_nums = array("0", "1", "3", "4", "5", "6", "7", "8", "9");
$section_names = array(
	array("BBS 系统", "[站内]"),
	array("清华大学", "[本校]"),
	array("电脑技术", "[电脑/系统]"),
	array("休闲娱乐", "[休闲/音乐]"),
	array("文化人文", "[文化/人文]"),
	array("社会信息", "[社会/信息]"),
	array("学术科学", "[学科/语言]"),
	array("体育健身", "[运动/健身]"),
	array("知性感性", "[谈天/感性]")
);
$dir_modes = array(
	"NORMAL" => 0,
	"DIGEST" => 1,
	"THREAD" => 2,
	"MARK" => 3,
	"DELETED" => 4,
	"JUNK" => 5,
	"ORIGIN" => 6,
	"AUTHOR" => 7,
	"TITLE" => 8
);

$loginok=0;
header("Cache-Control: no-cache");

  $fullfromhost=$_SERVER["HTTP_X_FORWARDED_FOR"];
  if ($fullfromhost=="") {
      $fullfromhost=$_SERVER["REMOTE_ADDR"];
      $fromhost=$fullfromhost;
  }
  else {
	$str = strrchr($fullfromhost, ",");
	if ($str!=FALSE)
		$fromhost=substr($str,1);
        else
		$fromhost=$fullfromhost;
  }

bbs_setfromhost($fromhost,$fullfromhost);

$utmpkey = $_COOKIE["UTMPKEY"];
$utmpnum = $_COOKIE["UTMPNUM"];
$userid = $_COOKIE["UTMPUSERID"];
if ($utmpkey!="") {
  if (bbs_setonlineuser($userid,$utmpnum,$utmpkey,$currentuinfo)==0) {
    $loginok=1;
    $currentuinfo_num=bbs_getcurrentuinfo();
    $currentuser_num=bbs_getcurrentuser($currentuser);
  }
}

function valid_filename($fn)
{
	if ((strstr($fn,"..")!=FALSE)||(strstr($fn,"/")))
		return 0;
	return 1;
}

function getboardfilename($boardname,$filename)
{
	return "boards/" . $boardname . "/" . $filename;
}

function error_alert($msg)
{
?>
<SCRIPT language="javascript">
window.alert(<? echo "\"$msg\""; ?>);
history.go(-1);
</SCRIPT>
<?php
}

function error_nologin()
{
?>
<SCRIPT language="javascript">
window.location="/nologin.html";
</SCRIPT>
<?php
}

function html_init($charset)
{
	$css_style = $_COOKIE["STYLE"];
?>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=<?php echo $charset; ?>">
<?php
	switch ($css_style)
	{
	case 1:
?>
<link rel="stylesheet" type="text/css" href="/bbs-bf.css">
<?php
		break;
	case 0:
	default:
?>
<link rel="stylesheet" type="text/css" href="/bbs.css">
<?php
	}
?>
</head>
<?php
}

function html_normal_quit()
{
?>
</body>
</html>
<?php
	exit;
}

function html_nologin()
{
?>
<html>
<head></head>
<body>
<script language="Javascript">
top.window.location='/nologin.html';
</script>
</body>
</html>
<?php
}

function html_error_quit($err_msg)
{
?>
<body>
错误! <?php echo $err_msg; ?>! <br><br>
<a href="javascript:history.go(-1)">快速返回</a>
</body>
</html>
<?php
	exit;
}

function get_bbsfile($relative_name)
{
	global $BBS_HOME;
	return $BBS_HOME . $relative_name;
}

function get_secname_index($secnum)
{
	global $section_nums;
	$arrlen = sizeof($section_nums);
	for ($i = 0; $i < $arrlen; $i++)
	{
		if (strcmp($section_nums[$i], $secnum) == 0)
			return $i;
	}
	return -1;
}

if (($loginok!=1)&&($_SERVER["PHP_SELF"]!="/bbslogin.php")) {
	error_nologin();
	return;
}

?>
