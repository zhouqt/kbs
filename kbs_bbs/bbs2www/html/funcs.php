<?php
	/**
	 * File included by all other php scripts.
	 * $Id$
	 */
if (!defined('_BBS_FUNCS_PHP_'))
{
define('_BBS_FUNCS_PHP_', 1);

if (!isset($topdir))
    $topdir=".";

// NOTE: If you want to statically link smth_bbs phpbbslib into php,
//       you *MUST* set enable_dl variable to Off in php.ini file.
if (BUILD_PHP_EXTENSION==0)
	@dl("$topdir/../libexec/bbs/libphpbbslib.so");

if (!bbs_ext_initialized())
	bbs_init_ext();
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
	"ZHIDING" => 9
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
function decodesessionchar($ch)
{
	return strpos(ENCODESTRING,$ch);
}

$loginok=0;

@$fullfromhost=$_SERVER["HTTP_X_FORWARDED_FOR"];
  if ($fullfromhost=="") {
      @$fullfromhost=$_SERVER["REMOTE_ADDR"];
      $fromhost=$fullfromhost;
  }
  else {
	$str = strrchr($fullfromhost, ",");
	if ($str!=FALSE)
		$fromhost=substr($str,1);
        else
		$fromhost=$fullfromhost;
  }

//sometimes,fromhost has strang space
bbs_setfromhost(trim($fromhost),trim($fullfromhost));

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
// add by stiger, login as "guest" default.....
if (($utmpkey == "")&&(!isset($needlogin) || ($needlogin!=0))){
	$error = bbs_wwwlogin(0);
	if($error == 2 || $error == 0){
		$data = array();
		$num = bbs_getcurrentuinfo($data);
        setcookie("UTMPKEY",$data["utmpkey"],time()+360000,"");
        setcookie("UTMPNUM",$num,time()+360000,"");
        setcookie("UTMPUSERID",$data["userid"],time()+360000,"");
        setcookie("LOGINTIME",$data["logintime"],time()+360000,"");
		@$utmpkey = $data["utmpkey"];
		@$utmpnum = $num;
		@$userid = $data["userid"];
  		$compat_telnet=1;
	}
}
//add end

if ($utmpkey!="") {
  if (($ret=bbs_setonlineuser($userid,intval($utmpnum),intval($utmpkey),$currentuinfo,$compat_telnet))==0) {
    $loginok=1;
    $currentuinfo_num=bbs_getcurrentuinfo();
    $currentuser_num=bbs_getcurrentuser($currentuser);
  }
}

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

function error_alert($msg)
{
?>
<SCRIPT language="javascript">
window.alert(<?php echo "\"$msg\""; ?>);
history.go(-1);
</SCRIPT>
<?php
}

function error_nologin()
{
        setcookie("UTMPKEY","",time() - 3600,"");
        setcookie("UTMPNUM","",time() - 3600,"");
        setcookie("UTMPUSERID","",time() - 3600,"");
        setcookie("LOGINTIME","",time() - 3600,"");
?>
<SCRIPT language="javascript">
window.location="/nologin.html";
</SCRIPT>
<?php
}

function cache_header($scope,$modifytime=0,$expiretime=300)
{
	global $cachemode;
	session_cache_limiter($scope);
	$cachemode=$scope;
	if ($scope=="no-cache")
		return FALSE;
	@$oldmodified=$_SERVER["HTTP_IF_MODIFIED_SINCE"];
	if ($oldmodified!="") {
                $oldtime=strtotime($oldmodified);
	} else $oldtime=0;
	if ($oldtime>=$modifytime) {
		header("HTTP/1.1 304 Not Modified");
	        header("Cache-Control: max-age=" . "$expiretime");
		return TRUE;
	}
	header("Last-Modified: " . gmdate("D, d M Y H:i:s", $modifytime) . "GMT");
	header("Expires: " . gmdate("D, d M Y H:i:s", $modifytime+$expiretime) . "GMT");
	header("Cache-Control: max-age=" . "$expiretime");
	return FALSE;
}

function html_init($charset,$title="",$otherheader="")
{
	global $_COOKIE;
	global $cachemode;
	if ($cachemode=="") {
		cache_header("no-cache");
		Header("Cache-Control: no-cache");
    }
	@$css_style = $_COOKIE["STYLE"];
	settype($css_style, "integer");
?>
<?xml version="1.0" encoding="<?php echo $charset; ?>"?>
<!DOCTYPE html
     PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=<?php echo $charset; ?>"/>
<?php
        if ( func_num_args() > 1) {
?>
<title><?php echo $title; ?></title>
<?php
        }
	switch ($css_style)
	{
	case 0:
?>
<link rel="stylesheet" type="text/css" href="/bbs-bf.css"/>
<?php
		break;
	case 1:
	default:
?>
<link rel="stylesheet" type="text/css" href="/bbs.css"/>
<?php
	}
?>
<link rel="stylesheet" type="text/css" href="/ansi.css"/>
<?php echo($otherheader); ?>
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
        setcookie("UTMPKEY","",time() - 3600,"");
        setcookie("UTMPNUM","",time() - 3600,"");
        setcookie("UTMPUSERID","",time() - 3600,"");
        setcookie("LOGINTIME","",time() - 3600,"");
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
´íÎó! <?php echo $err_msg; ?>! <br><br>
<a href="javascript:history.go(-1)">¿ìËÙ·µ»Ø</a>
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
	global $section_nums;
	$arrlen = sizeof($section_nums);
	for ($i = 0; $i < $arrlen; $i++)
	{
		if (strcmp($section_nums[$i], $secnum) == 0)
			return $i;
	}
	return -1;
}

function ansi_getfontcode($fgcolor,$bgcolor,$defaultfg,$defaultbg,$highlight,$blink,$underlink, &$head,&$tail)
{
    $modify="";
    if ($fgcolor==-1) 
      $modify=sprintf(" color=%s",$defaultfg);
    else
    if ($fgcolor==-2)
      $modify=sprintf(" color=%s",$defaultbg);
    else
    if ($highlight)
       $fgcolor+=8;
    if ($fgcolor<0) $fgcolor=0;
    if ($bgcolor==-1)
      $modify .= sprintf(" style='background-color:%s'",$defaultbg);
    else
    if ($bgcolor==-2)
      $modify .= sprintf(" style='background-color:%s'",$defaultfg);
    if ($bgcolor<0) $bgcolor=0;
    $head = sprintf("<font class=f%d%02d%s>",$bgcolor,$fgcolor,$modify);
    if ($underlink) {
       $head .= "<u>";
       $tail = "</u>";
    }
    $tail .= "</font>";
}

function ansi_convert( $buf , $defaultfg, $defaultbg)
{
    $keyword = preg_split("/\x1b\[([^a-zA-Z]*)([a-zA-Z])/",$buf,-1,PREG_SPLIT_DELIM_CAPTURE);
    $fgcolor=-1;
    $bgcolor=-1;
    $blink=false;
    $underlink=false;
    $highlight=false;
    for ($i=1;$i<count($keyword);$i+=3) {
        if ($keyword[$i+2]=="")
            continue;
        if ($keyword[$i+1]=='m') {
            $head="";
            $tail="";
            if ($keyword[$i]=="") {
            		// *[;m
                $fgcolor=-1;
                $bgcolor=-1;
                $blink=false;
                $underlink=false;
                $highlight=false;
            } else {
            	$good=true;
            	$colorcodes=split(';',$keyword[$i]);
                foreach ( $colorcodes as $code ) {
            	    if (preg_match("/[\D]/",$code)) {
            	    	$good=false;
            	        break;
            	    }
                    if ($code=="") 
                        $value=0;
            	    else
            	        $value=intval($code);
                    if ($value<=8 && $value>=1) {
                    	switch ($value) {
                    	case 0:
                            $fgcolor=-1;
                            $bgcolor=-1;
                            $blink=false;
                            $underlink=false;
                            break;
                    	case 1:
                    	    $highlight=1;
                    	    break;
                    	case 4:
                    	    $underlink=1;
                    	    break;
                    	case 5:
                    	    $blink=1;
                    	case 7:
                    	    $savebg=$bgcolor;
                    	    if ($fgcolor==-1)
                    	        $bgcolor=-2;
                    	    else
                    	        $bgcolor=$fgcolor;
                    	    if ($bgcolor==-1)
                    	        $fgcolor=-2;
                    	    else
                    	        $fgcolor=$savebg;
                        }
                    } else
                    if ($value<=37 && $value>=30)
                        $fgcolor=$value-30;
                    else
                    if ($value<=47 && $value>=40)
                        $bgcolor=$value-40;
                    else {
                    	// unsupport code
            	    	$good=false;
                        break;
                    }
                }
                if ($good)
                    ansi_getfontcode($fgcolor,$bgcolor,$defaultfg,$defaultbg,$highlight,$blink,$underlink, $head,$tail);
            }
            $final .= $head . htmlspecialchars($keyword[$i+2]) . $tail;
        } else $final .= htmlspecialchars($keyword[$i+2]);
    }
    return $final;
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

if ((!isset($needlogin)||($needlogin!=0))&&($loginok!=1)&&($_SERVER["PHP_SELF"]!="/bbslogin.php")) {
	error_nologin();
	return;
}

if (($loginok==1)&&(isset($setboard)&&($setboard==1))) bbs_set_onboard(0,0);
} // !define ('_BBS_FUNCS_PHP_')
?>
