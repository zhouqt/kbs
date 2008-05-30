<?php
if (!defined('_BBS_WWW2_FUNCS_PHP_'))
{
define('_BBS_WWW2_FUNCS_PHP_', 1);

if (!isset($topdir))
	$topdir=".";

// NOTE: If you want to statically link kbs_bbs phpbbslib into php,
//       you *MUST* set enable_dl variable to Off in php.ini file.
if (!defined("BUILD_PHP_EXTENSION") || BUILD_PHP_EXTENSION==0)
	@dl("libphpbbslib.so");

if (!bbs_ext_initialized())
	bbs_init_ext();
chdir(BBS_HOME);

require("site.php");

function getmicrotime(){ 
   list($usec, $sec) = explode(" ",microtime()); 
   return ((float)$usec + (float)$sec); 
} 
if (defined("RUNNINGTIME")) $StartTime=getmicrotime();

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
	"ANNOUNCE" => -1,
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
 * @param $caller
 *         0 - bbsdoc
 *         1 - bbscon
 * @return 0 - 不允许的模式
 *         1 - 允许模式，索引是排序的
 *         2 - 允许模式，索引是不排序的
 * @author atppp
 */
function bbs_is_permit_mode($ftype, $caller) {
	global $dir_modes;
	switch($ftype) {
		case $dir_modes["ZHIDING"]:
			return ($caller == 0) ? 0 : 1; /* 置顶可以假设是排序的 */
		case $dir_modes["NORMAL"]:
			return 1;
		case $dir_modes["ORIGIN"]:
			return ($caller == 0) ? 1 : 0;
		case $dir_modes["DIGEST"]:
		case $dir_modes["MARK"]: /* 暂时当作不可排序 ... */
		case $dir_modes["DELETED"]:
			return 2;
		default: return 0;
	}
}

$dir_name = array(
	-2 => "(查询模式)",
	-1 => "(精华区)",
	0 => "",
	1 => "(文摘区)",
	3 => "(保留区)",
	4 => "(回收站)",
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



if (!defined ('FAVORITE_NAME'))
	define ('FAVORITE_NAME', '百宝箱');

$loginok=0;

function delete_all_cookie() {
	setcookie("UTMPKEY","",time()-3600,"/");
	setcookie("UTMPNUM","",time()-3600,"/");
	setcookie("UTMPUSERID","",time()-3600,"/");
	setcookie("WWWPARAMS","",time()-3600,"/");
	setcookie("MANAGEBIDS","",time()-3600,"/");
	header("Set-KBSRC: /");
}

function set_fromhost()
{
	global $proxyIPs;
	global $fullfromhost;
	global $fromhost;
	
	if (defined("CHECK_X_FORWARDED_FOR")) {
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
	} else {
		@$fullfromhost=$_SERVER["REMOTE_ADDR"];
		$fromhost=$fullfromhost;
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

define("ENCODESTRING","0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");
function decodesessionchar($ch)
{
	return @strpos(ENCODESTRING,$ch);
}

/*如果 $no_auto_guest_login 设置为 TRUE，一定注意这之后的脚本及其调用的 C 函数可以处理 currentuser=NULL 的情况！*/
function login_init($sid=FALSE,$no_auto_guest_login=FALSE)
{
	global $currentuinfo;
	global $loginok;
	global $currentuser_num;
	global $currentuinfo_num;
	global $currentuser;
	global $utmpnum;
	global $setboard;
	global $fromhost;
	global $fullfromhost;
	$currentuinfo_tmp = array();
	
	$compat_telnet=0;
	$sessionid = "";
	if ($sid) {
		if (is_string($sid)) {
			$sessionid = $sid;
		} else {
			@$sessionid = $_GET["sid"];
			if (!$sessionid) @$sessionid = $_POST["sid"];
			if (!$sessionid) @$sessionid = $_COOKIE["sid"];
			settype($sessionid, "string");
		}
	}
	if ($sessionid && strlen($sessionid)==9) {
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
	
	if ($utmpkey) {
		if (($ret=bbs_setonlineuser($userid,intval($utmpnum),intval($utmpkey),$currentuinfo_tmp,$compat_telnet))==0) {
			$loginok=1;
			$currentuinfo_num=bbs_getcurrentuinfo();
			$currentuser_num=bbs_getcurrentuser($currentuser);
		}else
			$utmpkey="";
	}
	
	// add by stiger, 如果登录失败就继续用guest登录
	if (!$sessionid && !$utmpkey && !$no_auto_guest_login) {
		set_fromhost();
		$error = bbs_wwwlogin(0, $fromhost, $fullfromhost);
		if($error == 2 || $error == 0){
			$data = array();
			$num = bbs_getcurrentuinfo($data);
			setcookie("UTMPKEY",$data["utmpkey"],0,"/");
			setcookie("UTMPNUM",$num,0,"/");
			setcookie("UTMPUSERID",$data["userid"],0,"/");
			header("Set-KBSRC: /");
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
	if ($loginok!=1  && !$no_auto_guest_login) {
		delete_all_cookie();
		cache_header("nocache");
?>
<html>
	<head><meta http-equiv="Content-Type" content="text/html; charset=gb2312" /></head>
	<body>您还没有登录，或者你发呆时间过长被服务器清除。 请重新<a href="index.html" target="_top">登录</a>。</body>
</html>
<?php
		exit;
	}
	
	if (($loginok==1)&&(isset($setboard)&&($setboard==1))) bbs_set_onboard(0,0);
	return $sessionid;
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
		if (($pos = strpos($oldmodified, ';')) !== false)
			$oldmodified = substr($oldmodified, 0, $pos);
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

function update_cache_header($updatetime = 10) {
	return cache_process("public", 60 * $updatetime, time(), 60 * $updatetime);
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

function page_header($title, $flag = "", $otherheaders = false) {
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
<?php
 if (defined("www2dev")) echo "<script>var gwww2dev=true;</script>";
 if (defined("ENABLE_JSMATH")) echo "<script> var gjsmath=true;</script>";
?> 
<script src="static/www2-main.js" type="text/javascript"></script>
<script>writeCss();</script>
<?php if ($otherheaders) echo $otherheaders; ?>
</head>
<?php
	if ($flag === FALSE) return;
	if (isset($currentuser["userid"]) && $currentuser["userid"] != "guest" && bbs_checkwebmsg()) {
?>
<script type="text/javascript">alertmsg();</script>
<?php
	}
?>
<body>
<div class="nav smaller"><a href="<?php echo MAINPAGE_FILE; ?>"><?php echo BBS_FULL_NAME; ?></a><span id="idExp"></span><?php if ($flag) echo " → " . $flag; ?> → <?php echo $title; ?></div>
<?php
}

/* 特别注意：POST 递交生成的页面，不应该出现 展开完整界面 的链接，所以调用本函数必须用 FALSE 参数 */
function page_footer($checkframe = TRUE) {
	global $StartTime;
	if (defined("RUNNINGTIME")) {
		$endtime = getmicrotime();
		echo "<span style='font-size:12px'><center>Powered by KBS BBS 2.0 (<a href='http://dev.kcn.cn/' target='_blank'>http://dev.kcn.cn</a>)<br/>页面执行时间：".sprintf(number_format(($endtime-$StartTime)*1000,3))."毫秒</center></span>";
	}

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
<script type="text/javascript"><!--
window.alert(<?php echo "\"$msg\""; ?>);
history.go(-1);
//-->
</script>
</body></html>
<?php
	exit;
}

$htmlErrorNotify = false;
function html_error_quit($err_msg)
{
	global $htmlErrorNotify;
	page_header("发生错误");
?>
<table class="error">
<tr><th>发生错误</th></tr>
<tr><td><?php echo $err_msg; ?></td></tr>
</table>
[<a href="javascript:history.go(-1)">快速返回</a>]<br /><br />
<?php
	if ($htmlErrorNotify !== false) {
?>
<script>notifyError('<?php echo $htmlErrorNotify; ?>','<?php echo addSlashes($err_msg); ?>');</script>
<?php
	}
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
	echo "<br /><br />";
	page_footer(false);
	exit;
}

function assert_login() {
	global $currentuser;
	if(!strcmp($currentuser["userid"],"guest")) {
		$str = "您还没有登录，或者长时间没有动作，请您重新登录。<br /><br />";
		$str .= "<form action=\"bbslogin.php?mainurl=".urlencode($_SERVER["REQUEST_URI"])."\" method=\"post\" target=\"_parent\">";
		$str .= "用户名：<input type=\"text\" name=\"id\" size=\"15\" /><br />";
		$str .= "密　码：<input type=\"password\" name=\"passwd\" maxlength=\"39\" size=\"15\" /><br /><br />";
		$str .= "<input type=\"submit\" value=\"登录\" />&nbsp;<input type=\"button\" value=\"注册\" onclick=\"javascript:top.location.href='bbsreg0.html';\" />";
		html_error_quit($str);
	}
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
	for ($i = 0; $i < BBS_SECNUM; $i++)
	{
		if (strstr(constant("BBS_SECCODE".$i), $secnum) !== false)
			return $i;
	}
	return -1;
}

function bbs_get_super_fav ($title, $url='', $type=0) {
	if (!$url) $url = $_SERVER['REQUEST_URI'];
	return '<a href="bbssfav.php?act=choose&title='.rawurlencode ($title).'&url='.rawurlencode ($url).'&type='.intval ($type).'">'.FAVORITE_NAME.'</a>';
}
function bbs_add_super_fav ($title, $url='', $type=0) {
	echo bbs_get_super_fav($title, $url, $type);
}


function htmljsformat($str,$multi=false) {
	$str = str_replace(' ','&nbsp;',htmlspecialchars($str,ENT_QUOTES));
	if ($multi) {
		$str = str_replace(array("\r\n","\n","\r")," <br/> ",$str);
	}
	return addslashes($str);
}

function bbs_get_quote($filename)
{
	$str = "";
	if(file_exists($filename))
	{
	    $fp = fopen($filename, "r");
        if ($fp) {
		    $lines = 0;
		    $quser = "未知";
            $buf = fgets($fp,256);       /* 取出第一行中 被引用文章的 作者信息 */
			$end = strrpos($buf,")");
			$start = strpos($buf,":");
			if($start != FALSE && $end != FALSE)
			    $quser=substr($buf,$start+2,$end-$start-1);

            $str .= "\n【 在 " . $quser . " 的大作中提到: 】\n";
            for ($i = 0; $i < 3; $i++) {
                if (($buf = fgets($fp,500)) == FALSE)
                    break;
            }
            while (1) {
                if (($buf = fgets($fp,500)) == FALSE)
                    break;
                if (strncmp($buf, "【", 2) == 0)
                    continue;
                if (strncmp($buf, ": ", 2) == 0)
                    continue;
                if (strncmp($buf, "--\n", 3) == 0)
                    break;
                if (strncmp($buf, "\n", 1) == 0)
                    continue;
                if (++$lines > BBS_QUOTED_LINES) {
                    $str .= ": ...................\n";
                    break;
                }
                //if (stristr($buf, "</textarea>") == FALSE)  //filter </textarea> tag in the text
                    $str .= ": ". htmlspecialchars($buf);
            }
            fclose($fp);
        }
    }
    return $str;
}

} // !define ('_BBS_WWW2_FUNCS_PHP_')
?>
