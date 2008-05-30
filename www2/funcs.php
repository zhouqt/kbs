<?php
if (!defined('_BBS_FUNCS_PHP_'))
{
define('_BBS_FUNCS_PHP_', 1);

if (!isset($topdir))
	$topdir=".";

// NOTE: If you want to statically link kbs_bbs phpbbslib into php,
//       you *MUST* set enable_dl variable to Off in php.ini file.
if (!defined("BUILD_PHP_EXTENSION") || BUILD_PHP_EXTENSION==0)
	@dl("libphpbbslib.so");

if (!bbs_ext_initialized())
	bbs_init_ext();
chdir(BBS_HOME);

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
global $cachemode;
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

$bbsman_modes = array(
	"DEL"   => 1,
	"MARK"  => 2,
	"DIGEST"=> 3,
	"NOREPLY" => 4,
	"ZHIDING" => 5
	);
/**
 * Constants of board flags, packed in an array.
 */
$BOARD_FLAGS = array(
	"VOTE" => 0x01,
	"NOZAP" => 0x02,
	"READONLY" => 0x04,
	"JUNK" => 0x08,
	"ANONY" => 0x10,
	"OUTGO" => 0x20,
	"CLUBREAD" => 0x40,
	"CLUBWRITE" => 0x80,
	"CLUBHIDE" => 0x100,
	"ATTACH" => 0x200,
	"NOREPLY" => 0x2000
	);
$filename_trans = array(" " => "_", 
	";" => "_", 
	"|" => "_",
	"&" => "_",
	">" => "_",
	"<" => "_",
	"*" => "_",
	"\"" => "_",
	"'" => "_"
	);
require("site.php");

define("ENCODESTRING","0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");
define("ACTIVATIONLEN",15); //激活码长度
define("WWW_DEFAULT_PARAMS","0"); //默认的WWW参数
if (!defined ('FAVORITE_NAME'))
	define ('FAVORITE_NAME', '我的百宝箱');

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
	
	// add by stiger, 如果登陆失败就继续用guest登陆
	if ($utmpkey == "") {
		$error = bbs_wwwlogin(0);
		if($error == 2 || $error == 0){
			$data = array();
			$num = bbs_getcurrentuinfo($data);
			setcookie("UTMPKEY",$data["utmpkey"],0,"/");
			setcookie("UTMPNUM",$num,0,"/");
			setcookie("UTMPUSERID",$data["userid"],0,"/");
			setcookie("LOGINTIME",$data["logintime"],0,"/");
			setcookie("WWWPARAMS",WWW_DEFAULT_PARAMS,0,"/"); 
			@$utmpkey = $data["utmpkey"];
			@$utmpnum = $num;
			@$userid = $data["userid"];
			$compat_telnet=1;
		}
		//guest 登陆成功，设置一下
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
	if (($loginok!=1)&&($_SERVER["PHP_SELF"]!="/bbslogin.php")) {
		error_nologin();
		return;
	}
	
	if (($loginok==1)&&(isset($setboard)&&($setboard==1))) bbs_set_onboard(0,0);
	//add end
}


/* 
** BBS Board Envelop Code Start
** add by windinsn Mar 13 , 2004 
*/
$defultenvfile = "index.html";
function board_envelop_dir($board)
{
	if( !valid_filename($board) )
		return FALSE;
	return $_SERVER["DOCUMENT_ROOT"] . "/boards/" . $board;
}

function board_envelop_file($board , $filename )
{
	if( !valid_filename($board) || !valid_filename($filename))
		return FALSE;
	return  board_envelop_dir($board) . "/" . $filename;
}

function bbs_board_have_envelop($board)
{
	global $defultenvfile;
	$envfile = board_envelop_file( $board , $defultenvfile );
	if( !$envfile) 
		return FALSE;
	if( file_exists( $envfile ) )
		return TRUE;
	else
		return FALSE;	
}
/* BBS Board Envelop Code End */

function valid_filename($fn)
{
	if ((strstr($fn,"..")!=FALSE)||(strstr($fn,"/")!=FALSE))
		return 0;
	if ( (strstr($fn,"&")!=FALSE)||(strstr($fn,";")!=FALSE)
	   ||(strstr($fn,"|")!=FALSE)||(strstr($fn,"*")!=FALSE)
	   ||(strstr($fn,"<")!=FALSE)||(strstr($fn,">")!=FALSE))
		return 0;
	return 1;
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

function error_alert($msg)
{
?>
<SCRIPT type="text/javascript">
window.alert(<?php echo "\"$msg\""; ?>);
history.go(-1);
</SCRIPT>
<?php
}

function error_nologin()
{
	setcookie("UTMPKEY","",time() - 3600,"/");
	setcookie("UTMPNUM","",time() - 3600,"/");
	setcookie("UTMPUSERID","",time() - 3600,"/");
	setcookie("LOGINTIME","",time() - 3600,"/");
	setcookie("WWWPARAMS","",time() - 3600,"/");
?>
<SCRIPT type="text/javascript">
window.location="/nologin.html";
</SCRIPT>
<?php
}

/* used by cache_header() and update_cache_header()，不应该直接被调用 */
function cache_process($scope, $forcecachetime, $modifytime, $expiretime) {
	global $cachemode;
	//session_cache_limiter($scope); //这个函数只能接受 "nocache" 参数，不能接受 "no-cache" 参数
	$cachemode=$scope;
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
	if ($oldtime >= $modifytime) {
		header("HTTP/1.1 304 Not Modified");
		header("Cache-Control: max-age=" . "$expiretime");
		return TRUE;
	}
	header("Last-Modified: " . gmt_date_format($modifytime));
	header("Expires: " . gmt_date_format(time()+$expiretime));
	header("Cache-Control: max-age=" . "$expiretime");
	return FALSE;
}

function cache_header($scope,$modifytime=0,$expiretime=300)
{
	return cache_process($scope, 0, $modifytime, $expiretime);
}

function update_cache_header($updatetime = 10,$expiretime = 300)
{
	return cache_process("public", 60 * $updatetime, time(), $expiretime);
}


function html_init($charset,$title="",$otherheader="",$new_style=0)
{
	global $cachemode;
	global $currentuser;
	if ($cachemode=="") {
		cache_header("nocache");
	}
	echo '<?xml version="1.0" encoding="'.$charset.'"?>'."\n";
?>
<!DOCTYPE html
	 PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=<?php echo $charset; ?>"/>
<script src="bbs.js"></script>
<?php
		if ( func_num_args() > 1) {
?>
<title><?php echo $title; ?></title>
<?php
		}
	if($new_style==9 ){
?>
<link rel="stylesheet" type="text/css" href="bbsleft.css"/>
<?php		
	}
	else{
?>
<link rel="stylesheet" type="text/css" href="default.css"/>
<?php			
	}
	echo($otherheader); 
?>
</head>
<?php
	if($_SERVER["PHP_SELF"] != "/bbsgetmsg.php" 
			&& isset($currentuser["userid"]) && ($currentuser["userid"] != "guest")
			&& bbs_checkwebmsg())
	{
?>
<script type="text/javascript">
if (top.fmsg && !top.fmsg.form0)
	top.fmsg.location.reload();
</script>
<?php
	}
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
	setcookie("UTMPKEY","",time() - 3600,"/");
	setcookie("UTMPNUM","",time() - 3600,"/");
	setcookie("UTMPUSERID","",time() - 3600,"/");
	setcookie("LOGINTIME","",time() - 3600,"/");
	setcookie("WWWPARAMS","",time() - 3600,"/");
?>
<html>
<head></head>
<body>
<script type="text/javascript">
top.window.location='/nologin.html';
</script>
</body>
</html>
<?php
exit();
}

function html_error_quit($err_msg)
{
?>
<link rel="stylesheet" type="text/css" href="/default.css"/>
<body>
<br /><br /><br /><br /><br /><br /><br /><br /><br /><br />
<center>
<table cellspacing="0" cellpadding="10" border="0" class="t1">
<tr><td class="t3">
<font style="color: #FF0000"><b>发生错误</b></font>
</td></tr>
<tr><td class="t4">
<?php echo $err_msg; ?>
</td></tr>
</table><br /><br />
[<a href="javascript:history.go(-1)">快速返回</a>]
</center>
</body>
</html>
<?php
	exit;
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

function get_bbsfile($relative_name)
{
	return BBS_HOME . $relative_name;
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

function bbs_is_owner($article, $user)
{
	if ($article["OWNER"] == $user["userid"])
		return 1;
	else
		return 0;
}

function bbs_can_delete_article($board, $article, $user)
{
	if (bbs_is_bm($board["NUM"], $user["index"]) 
			|| bbs_is_owner($article, $user))
		return 1;
	else
		return 0;
}

function bbs_can_edit_article($board, $article, $user)
{
	if (bbs_is_bm($board["NUM"], $user["index"]) 
			|| bbs_is_owner($article, $user))
		return 1;
	else
		return 0;
}

function check_php_setting($name, $value) {
  if (ini_get($name) != $value) {
	print "<p>Note that the value of PHP's configuration option <code><b>$name</b></code> is incorrect.  It should be set to '$value' for Drupal to work properly.</p>";
  }
}

function bbs_login_form()
{
?>
<form name="loginform" target="_top" action="/bbslogin.php?mainurl=<?php echo urlencode($_SERVER["REQUEST_URI"]); ?>" method="post">
<B><FONT SIZE="2" color="#333333">用户名:</FONT></B>
<INPUT TYPE=text STYLE="width:80px;height:16px;font-size: 12px;color: #555555;border-color: #000000;border-style: solid;border-width: 1px;background-color: #feffff;" LENGTH="10" onMouseOver="this.focus()" onFocus="this.select()" name="id" >
&nbsp;&nbsp;
<B><FONT SIZE="2" color="#333333">密 码:</FONT></B>
<INPUT TYPE=password  STYLE="width:80px;height:16px;font-size: 12px;color: #555555;border-color: #000000;border-style: solid;border-width: 1px;background-color: #feffff;" LENGTH="10" name="passwd" maxlength="39">
&nbsp;&nbsp;&nbsp;&nbsp;
<input type="submit" value="登录" style="width:40px;height:16px;font-size: 12px;color: #555555;border-color: #000000;border-style: solid;border-width: 1px;background-color: #fffff8;" name="submit1">
&nbsp;
<input type="button" name="guest" value="注册" style="width:40px;height:16px;font-size: 12px;color: #555555;border-color: #000000;border-style: solid;border-width: 1px;background-color: #fffff8;" onclick="location.href='/bbsreg0.html'")>
</form>
<?php	
}

function powered_by_smth()
{
?>
<STYLE TYPE="text/css">
<!--
.PoweredBySmth {font-size: 12px; font-family: "Verdana", "Arial", "Helvetica", "sans-serif";color: #000000; font-weight: bold;text-align:center;}
.PoweredBySmth A {color: #CC0000; text-decoration: none;}
.PoweredBySmth A:hover {text-decoration: underline;}
-->
</STYLE>
<TABLE CLASS="PoweredBySmth" align="center">
<TR><TD>
Powered By <A HREF="http://dev.kcn.cn/" TARGET="_blank">KBS_BBS</A>
</TD></TR><TR><TD>
<A HREF="http://dev.kcn.cn/" TARGET="_blank"><IMG SRC="/images/poweredby.gif" BORDER="0" ALT="Powered By KBS_BBS" /></A>
</TD></TR>
</TABLE>
<?php    
}

function bbs_add_super_fav ($title, $url='', $type=0) {
	global $currentuser;
	if (!$url) $url = $_SERVER['REQUEST_URI'];
	echo '<a href="/bbssfav.php?act=choose&title='.rawurlencode ($title).'&url='.rawurlencode ($url).'&type='.intval ($type).'">'.FAVORITE_NAME.'</a>';
}

} // !define ('_BBS_FUNCS_PHP_')
?>
