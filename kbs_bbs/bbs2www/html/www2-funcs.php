<?php
if (!defined('_BBS_WWW2_FUNCS_PHP_'))
{
define('_BBS_WWW2_FUNCS_PHP_', 1);

if (!isset($topdir))
	$topdir=".";

// NOTE: If you want to statically link smth_bbs phpbbslib into php,
//       you *MUST* set enable_dl variable to Off in php.ini file.
if (BUILD_PHP_EXTENSION==0)
	@dl("$topdir/../libexec/bbs/libphpbbslib.so");

chdir(BBS_HOME);
if (!bbs_ext_initialized())
	bbs_init_ext();

global $fromhost;
global $fullfromhost;
global $loginok;
//global $currentuinfo;
global $currentuinfo_num;
//global $currentuser;
global $currentuuser_num;
global $cachemode;
$headershown = false;
$cachemode="";
$currentuinfo=array ();
$currentuser=array ();



$dir_modes = array(
	"NORMAL" => 0,
	"DIGEST" => 1,
	"THREAD" => 2,
	"MARK" => 3,
	"DELETED" => 4,
	"JUNK" => 5,
	"ORIGIN" => 6,
	"AUTHOR" => 7,
	"TITLE" => 8,
	"SUPERFILTER" => 9,
	"WEB_THREAD" => 10,
	"ZHIDING" => 11
);
/**
 * Checking whether the dir mode is permitted or not.
 * 
 * @param $ftype the mode
 * @return 0     the mode is not allowed
 *         1     the mode is allowed, and the index is sorted
 *         2     the mode is allowed, and the index is not sorted
 * @author atppp
 */
function bbs_is_permit_mode($ftype) {
	global $dir_modes;
	switch($ftype) {
		case $dir_modes["ZHIDING"]:
			//zhiding mode is not sorted, but the only place we care is
			//in bbscon.php, and there we can say it's sorted without problem
		case $dir_modes["NORMAL"]:
		case $dir_modes["ORIGIN"]:
		/* case $dir_modes["SUPERFILTER"]: */ return 1; break;
		case $dir_modes["DIGEST"]:
		/* case $dir_modes["MARK"]: */  return 2; break;
		default: return 0; break;
	}
}

$dir_name = array(
	0 => "",
	1 => "(文摘区)",
	3 => "(保留区)",
	6 => "(主题模式)",
	11 => ""
);
$dir_index = array(
	0 => ".DIR",
	1 => ".DIGEST",
	3 => ".MARK",
	4 => ".DELETED",
	5 => ".JUNK",
	6 => ".ORIGIN",
	11 => ".DINGDIR"
);
function bbs_get_board_index($board, $ftype) {
	global $currentuser;
	global $dir_index;
	global $dir_modes;
	if ($ftype == $dir_modes["SUPERFILTER"]) {
		$userid = $currentuser["userid"];
		if ($userid == "guest") die; /* TODO */
		return ".Search." . $userid;
	} else {
		return bbs_get_board_filename($board, $dir_index[$ftype]);
	}
}



require("site.php");

define("WWW_DEFAULT_PARAMS", 0);
define("ACTIVATIONLEN",15); //激活码长度
if (!defined ('FAVORITE_NAME'))
	define ('FAVORITE_NAME', '百宝箱');

function decodesessionchar($ch)
{
	return strpos(ENCODESTRING,$ch);
}

$loginok=0;

function set_fromhost()
{
	global $proxyIPs;
	global $fullfromhost;
	global $fromhost;
	
	@$fullfromhost=$_SERVER["HTTP_X_FORWARDED_FOR"];
	if ($fullfromhost=="") {
		@$fullfromhost=$_SERVER["REMOTE_ADDR"];
		$fromhost=$fullfromhost;
	}
	else {
		$ips = explode(",", $fullfromhost);
		$c = count($ips);
		if ($c > 1) {
			$fromhost = trim($ips[$c - 1]);
			if (isset($proxyIPs) && in_array($fromhost, $proxyIPs)) {
				$fromhost = $ips[$c - 2];
			}
		} else $fromhost = $fullfromhost;
	}
	if ($fromhost=="")  {
		$fromhost="127.0.0.1"; 
		$fullfromhost="127.0.0.1"; 
	}
	if (defined("IPV6_LEGACY_IPV4_DISPLAY")) { 
		if ((strchr($fromhost, '.'))&&($p=strrchr($fromhost, ':')))
			$fromhost=substr($p, 1);
	}
	//sometimes,fromhost has strang space
	bbs_setfromhost(trim($fromhost),trim($fullfromhost));
}

function login_init()
{
	global $currentuinfo;
	global $loginok;
	global $currentuser_num;
	global $currentuinfo_num;
	global $currentuser;
	global $utmpnum;
	global $setboard;
	global $fromhost;
	$currentuinfo_tmp = array();
	
	set_fromhost();
	
	$compat_telnet=0;
	@$sessionid = $_GET["sid"];
	
	//TODO: add the check of telnet compat
	if (($sessionid!='')&&($_SERVER['PHP_SELF']=='/bbscon.php')) {
		$utmpnum=decodesessionchar($sessionid[0])+decodesessionchar($sessionid[1])*36+decodesessionchar($sessionid[2])*36*36;
		$utmpkey=decodesessionchar($sessionid[3])+decodesessionchar($sessionid[4])*36+decodesessionchar($sessionid[5])*36*36
			+decodesessionchar($sessionid[6])*36*36*36+decodesessionchar($sessionid[7])*36*36*36*36+decodesessionchar($sessionid[8])*36*36*36*36*36;
		$userid='';
		$compat_telnet=1;
	} else {
		@$utmpkey = $_COOKIE["UTMPKEY"];
		@$utmpnum = $_COOKIE["UTMPNUM"];
		@$userid = $_COOKIE["UTMPUSERID"];
	}
	
	if ($utmpkey!="") {
		if (($ret=bbs_setonlineuser($userid,intval($utmpnum),intval($utmpkey),$currentuinfo_tmp,$compat_telnet))==0) {
			$loginok=1;
			$currentuinfo_num=bbs_getcurrentuinfo();
			$currentuser_num=bbs_getcurrentuser($currentuser);
		}else
			$utmpkey="";
	}
	
	// add by stiger, 如果登录失败就继续用guest登录
	if ($utmpkey == "") {
		$error = bbs_wwwlogin(0);
		if($error == 2 || $error == 0){
			$data = array();
			$num = bbs_getcurrentuinfo($data);
			setcookie("UTMPKEY",$data["utmpkey"],time()+360000,"/");
			setcookie("UTMPNUM",$num,time()+360000,"/");
			setcookie("UTMPUSERID",$data["userid"],time()+360000,"/");
			setcookie("WWWPARAMS",WWW_DEFAULT_PARAMS,time()+360000,"/"); 
			@$utmpkey = $data["utmpkey"];
			@$utmpnum = $num;
			@$userid = $data["userid"];
			$compat_telnet=1;
		}
		//guest 登录成功，设置一下
		if ($utmpkey!="") {
			if (($ret=bbs_setonlineuser($userid,intval($utmpnum),intval($utmpkey),$currentuinfo_tmp,$compat_telnet))==0) {
				$loginok=1;
				$currentuinfo_num=bbs_getcurrentuinfo();
				$currentuser_num=bbs_getcurrentuser($currentuser);
			}
		}
	}
	
	$currentuinfo = $currentuinfo_tmp;
	
	settype($utmpnum,"integer");
	if ($loginok!=1) {
		setcookie("UTMPKEY","",time() - 3600,"/");
		setcookie("UTMPNUM","",time() - 3600,"/");
		setcookie("UTMPUSERID","",time() - 3600,"/");
		setcookie("WWWPARAMS","",time() - 3600,"/");
?>
<html>
	<head><meta http-equiv="Content-Type" content="text/html; charset=gb2312" /></head>
	<body>您还没有登录，或者你发呆时间过长被服务器清除。 请重新<a href="index.html" target="_top">登录</a>。</body>
</html>
<?php
		exit;
	}
	
	if (($loginok==1)&&(isset($setboard)&&($setboard==1))) bbs_set_onboard(0,0);
	//add end
}




function bbs_get_board_filename($boardname,$filename)
{
	return "boards/" . $boardname . "/" . $filename;
}

function bbs_get_vote_filename($boardname, $filename)
{
	return "vote/" . $boardname . "/" . $filename;
}

function gmt_date_format($t) {
	return gmdate("D, d M Y H:i:s", $t) . " GMT";
}



/* used by cache_header() and update_cache_header()，不应该直接被调用 */
function cache_process($scope, $forcecachetime, $modifytime, $expiretime) {
	global $cachemode;
	//session_cache_limiter($scope); //这个函数只能接受 "nocache" 参数，不能接受 "no-cache" 参数
	$cachemode=$scope;
	if ($modifytime > 0) {
        header("Last-Modified: " . gmt_date_format($modifytime));
    }
	if ($scope=="nocache" || $scope=="no-cache") { //兼容 "no-cache" 的写法，不推荐
		header("Expires: Thu, 19 Nov 1981 08:52:00 GMT");
		header("Cache-Control: no-store, no-cache, must-revalidate");
		header("Pragma: no-cache");
		return FALSE;
	}
	@$oldmodified=$_SERVER["HTTP_IF_MODIFIED_SINCE"];
	if ($oldmodified!="") {
		$oldtime = strtotime($oldmodified) + $forcecachetime;
	} else $oldtime=0;
	if ($oldtime >= $modifytime && $modifytime > 0) {
		header("HTTP/1.1 304 Not Modified");
		header("Cache-Control: max-age=" . "$expiretime");
		return TRUE;
	}
	header("Expires: " . gmt_date_format(time()+$expiretime));
	header("Cache-Control: max-age=" . "$expiretime");
	return FALSE;
}

function cache_header($scope,$modifytime=0,$expiretime=300) {
	settype($modifytime, "integer");
	return cache_process($scope, 0, $modifytime, $expiretime);
}

function update_cache_header($updatetime = 10,$expiretime = 300) {
	return cache_process("public", 60 * $updatetime, time(), $expiretime);
}




function toolbox_header($title) {
	global $currentuser;
	page_header($title, $currentuser["userid"] . " 的工具箱");
}

function mailbox_header($title) {
	global $currentuser;
	page_header($title, $currentuser["userid"] . " 的信箱");
}

function bbs_board_nav_header($brdarr, $title) {
	page_header($title, "<a href=\"bbsdoc.php?board=" . $brdarr["NAME"] . "\">" . htmlspecialchars($brdarr["DESC"]) . "</a>");
}

function page_header($title, $flag = "") {
	/*
	 * $flag: FALSE  - no header
	 *        string - parent
	 */
	global $cachemode;
	global $currentuser;
	global $headershown;
	if ($headershown) return;
	$headershown = true;
	if ($cachemode=="") {
		cache_header("nocache");
	}
?>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=gb2312"/>
<title><?php echo $title; ?></title>
<script src="www2-main.js" type="text/javascript"></script>
<link rel="stylesheet" type="text/css" href="www2-default.css" />
</head>
<?php
	if ($flag === FALSE) return;
	if($currentuser["userid"] != "guest" && bbs_checkwebmsg()) {
?>
<script type="text/javascript">alertmsg();</script>
<?php
	}
?>
<body>
<div id="divNav"><a href="<?php echo MAINPAGE_FILE; ?>"><?php echo BBS_FULL_NAME; ?></a><?php if ($flag) echo " → " . $flag; ?> → <?php echo $title; ?></div>
<?php
}

/* 特别注意：POST 递交生成的页面，不应该出现 展开完整界面 的链接，所以调用本函数必须用 FALSE 参数 */
function page_footer($checkframe = TRUE) {
	if ($checkframe) {
?>
<script>checkFrame();</script>
<?php
	}
?>
</body></html>
<?php
}

function error_alert($msg)
{
	page_header("发生错误");
?>
<script type="text/javascript">
window.alert(<?php echo "\"$msg\""; ?>);
history.go(-1);
</script>
</body></html>
<?php
	exit;
}

function html_error_quit($err_msg)
{
	page_header("发生错误");
?>
发生错误: <?php echo $err_msg; ?><br />
[<a href="javascript:history.go(-1)">快速返回</a>]
<?php
	page_footer(false);
	exit;
}

function html_success_quit($msg, $operations = FALSE)
{
	page_header("操作成功");
	echo "操作成功: " . $msg;
	if ($operations === FALSE) {
		echo "<br /><br />[<a href=\"javascript:history.go(-2);\">快速返回</a>]";
	} else {
?>
<div class="medium"><ul><b>您可以选择以下操作：</b>
<?php
		foreach($operations as $o) echo "<li>" . $o . "</li>";
?>
</ul>
<?php
	}
	page_footer(false);
	exit;
}

function assert_login() {
	global $currentuser;
	if(!strcmp($currentuser["userid"],"guest"))
		html_error_quit("请先登录");
}

function sizestring($size)
{
	if ($size<1024)
	  return "$size";
	$fsize=((double)$size)/1024;
	if ($fsize<1024) {
	  return sprintf("%01.2fk","$fsize");
	}
	$fsize=((double)$fsize)/1024;
	if ($fsize<1024) {
	  return sprintf("%01.2fM","$fsize");
	}
	$fsize=((double)$fsize)/1024;
	if ($fsize<1024) {
	  return sprintf("%01.2fG","$fsize");
	}
	$fsize=((double)$fsize)/1024;
	if ($fsize<1024) {
	  return sprintf("%01.2fT","$fsize");
	}
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

function bbs_add_super_fav ($title, $url='', $type=0) {
	global $currentuser;
	if (!$url) $url = $_SERVER['REQUEST_URI'];
	echo '<a href="bbssfav.php?act=choose&title='.rawurlencode ($title).'&url='.rawurlencode ($url).'&type='.intval ($type).'">'.FAVORITE_NAME.'</a>';
}

} // !define ('_BBS_WWW2_FUNCS_PHP_')
?>
