<?php
	/**
	 * File included by all other php scripts.
	 * $Id$
	 */
if (!defined('_BBS_FUNCS_PHP_'))
{
define('_BBS_FUNCS_PHP_', 1);

if ( (!isset($_COOKIE['iscookies'])) || ($_COOKIE['iscookies']==''))
{
	setcookie('iscookies','0',time()+3650*24*3600);
	print '<META http-equiv=Content-Type content=text/html; charset=gb2312><meta HTTP-EQUIV=REFRESH CONTENT=3>正在登陆论坛……<br><br>本系统要求使用COOKIES，假如您的浏览器禁用COOKIES，您将不能登录本系统……';
	exit();
} 


function getmicrotime(){ 
   list($usec, $sec) = explode(" ",microtime()); 
   return ((float)$usec + (float)$sec); 
} 

global $StartTime;
$StartTime=getmicrotime();

// NOTE: If you want to statically link smth_bbs phpbbslib into php,
//       you *MUST* set enable_dl variable to Off in php.ini file.
if (ini_get("enable_dl") == 1)
	dl("libphpbbslib.so");


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
global $errMsg;
global $foundErr;
global $stats;
global $gusetloginok;
global $sucmsg;
$yank=1;
$sucmsg='';
$stats='';
$errMsg='';
$foundErr=false;
$loginok=0;
$guestloginok=0;

if (!isset($nologin)) {
	$nologin=0;
}

if (!isset($setboard)){
	$setboard=0;
}

if (!isset($needlogin)){
	$needlogin=1;
}

function setSucMsg($msg){
	global $sucmsg;
	$sucmsg.='<br><li>'.$msg;
}
function setStat($stat){
	GLOBAL $stats;
	$stats=$stat;
}

function foundErr($msg){
	global $errMsg;
	global $foundErr;
	$errMsg.='<br><li>'.$msg;
	$foundErr=true;
}

function isErrFounded(){
	GLOBAL $foundErr;
	return $foundErr;
}

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
	"ATTACH" => 0x200
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

bbs_setfromhost($fromhost,$fullfromhost);

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
	@$utmpkey = $_COOKIE["W_UTMPKEY"];
	@$utmpnum = $_COOKIE["W_UTMPNUM"];
	@$userid = $_COOKIE["W_UTMPUSERID"];
	@$userpassword=$_COOKIE["W_PASSWORD"];
}
if ($userid=='') {
	$userid='guest';
}

$setonlined=0;
if ($nologin==0) {

	// add by stiger, login as "guest" default.....
	if ( ($userid=='guest') && ($utmpkey == "")&&($needlogin!=0)){ 
		$error = bbs_wwwlogin(0);
		if($error == 2 || $error == 0){
			$guestloginok=1;
		}
	} else {
		if ( ($utmpkey!="") || ($userid!='guest')) {
			$ret=bbs_setonlineuser($userid,intval($utmpnum),intval($utmpkey),$currentuinfo,$compat_telnet);
			if (($ret)==0) {
				if ($userid!="guest") {
					$loginok=1;
				} else {
					$guestloginok=1;
				}
				$currentuinfo_num=bbs_getcurrentuinfo();
				$currentuser_num=bbs_getcurrentuser($currentuser);
				$setonlined=1;
			} else {
				if (($userid!='guest') && (bbs_checkpasswd($userid,$userpassword)==0)){
					$ret=bbs_wwwlogin(1);
					if ( ($ret==2) || ($ret==0) ){
						if ($userid!="guest") {
							$loginok=1;
						} else {
							$guestloginok=1;
						}

					}else if ($ret==5) {
						foundErr("请勿频繁登陆！");
					}
				} else {
					$error = bbs_wwwlogin(0);
					if($error == 2 || $error == 0){
						$guestloginok=1;
					}
				}
			}
		}
	}
}

if  ( ($loginok || $guestloginok ) && ($setonlined==0) ){
	$data=array();
	$currentuinfo_num=bbs_getcurrentuinfo($data);
	bbs_setonlineuser($userid,$currentuinfo_num,$data["utmpkey"],$currentuinfo,$compat_telnet);
	$currentuser_num=bbs_getcurrentuser($currentuser);
	$path='';
	setcookie("W_UTMPUSERID",$data["userid"],time()+360000,"",$path);
	setcookie("W_UTMPKEY",$data["utmpkey"],time()+360000,$path);
	setcookie("W_UTMPNUM",$currentuinfo_num,time()+360000,$path);
	setcookie("W_LOGINTIME",$data["logintime"],time()+360000,$path);
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

function html_init($charset="",$title="",$otherheader="")
{
	global $_COOKIE;
	global $cachemode;
	global $HTMLTitle;
	global $HTMLCharset;
	global $DEFAULTStyle;
	global $stats;

	session_start();
	if ($charset==""){
		$charset=$HTMLCharset;
	}
	if ($title=="") {
		$title=$HTMLTitle;
	}
	if (isset($stats) ){
		$title = $title . ' -- ' . $stats;
	}
	if ($cachemode=="") {
		cache_header("no-cache");
		Header("Cache-Control: no-cache");
    }
	@$css_style = $_COOKIE["style"];
	if ($css_style==''){
		$css_style=$DEFAULTStyle;
	}
?>
<?xml version="1.0" encoding="<?php echo $charset; ?>"?>
<!DOCTYPE html
     PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=<?php echo $charset; ?>"/>
<title><?php echo $title; ?></title>
<link rel="stylesheet" type="text/css" href="css/ansi.css"/>
<link rel="stylesheet" type="text/css" href="css/<?php echo $css_style; ?>.css"/>
<script src="inc/funcs.js"  language="javascript"></script>
<?php echo($otherheader); ?>
</head>
<?php
}




function html_error_quit()
{
	global $errMsg;
	global $needlogin;
	global $loginok;
?>
<br>
<table cellpadding=3 cellspacing=1 align=center class=TableBorder1 width="75%">
<tr align=center>
<th width="100%" height=25 colspan=2>论坛错误信息
</td>
</tr>
<tr>
<td width="100%" class=TableBody1 colspan=2>
<b>产生错误的可能原因：</b><br><br>
<li>您是否仔细阅读了<a href="boardhelp.php">帮助文件</a>，可能您还没有登陆或者不具有使用当前功能的权限。
<?php   echo $errMsg; ?>
</td></tr>
<?php   if (($needlogin!=0)&&($loginok!=1))
  {
?>
<form action="logon.php" method=post>
<input type="hidden" name="action" value="doLogon">
    <tr>
    <th valign=middle colspan=2 align=center height=25>请输入您的用户名、密码登陆</td></tr>
    <tr>
    <td valign=middle class=TableBody1>请输入您的用户名</td>
    <td valign=middle class=TableBody1><INPUT name=id type=text> &nbsp; <a href=reg.php>没有注册？</a></td></tr>
    <tr>
    <td valign=middle class=TableBody1>请输入您的密码</font></td>
    <td valign=middle class=TableBody1><INPUT name=password type=password> &nbsp; <a href=lostpass.php>忘记密码？</a></td></tr>
    <tr>
    <td class=TableBody1 valign=top width=30% ><b>Cookie 选项</b><BR> 请选择你的 Cookie 保存时间，下次访问可以方便输入。</td>
    <td valign=middle class=TableBody1>                <input type=radio name=CookieDate value=0 checked>不保存，关闭浏览器就失效<br>
                <input type=radio name=CookieDate value=1>保存一天<br>
                <input type=radio name=CookieDate value=2>保存一月<br>
                <input type=radio name=CookieDate value=3>保存一年<br>                </td></tr>
	<input type=hidden name=comeurl value="<?php     echo $_SERVER['HTTP_REFERER']; ?>">
    <tr>
    <td class=TableBody2 valign=middle colspan=2 align=center><input type=submit name=submit value="登 陆">&nbsp;&nbsp;<input type=button name="back" value="返 回" onclick="location.href='<?php  echo $_SERVER['HTTP_REFERER']; ?>'"></td></tr>
</form>
<?php   }
    else
  {
?>
    <tr>
    <td class=TableBody2 valign=middle colspan=2 align=center><a href="<?php echo $_SERVER['HTTP_REFERER']; ?>"> <<返回上一页 </a></td></tr>
<?php   } ?>
</table>
<?php 
} 

function html_success_quit($Desc='',$URL='')
{
  global $sucmsg;
?>
<br>
<table cellpadding=3 cellspacing=1 align=center class=TableBorder1 width="75%">
<tr align=center>
<th width="100%">论坛成功信息
</td>
</tr>
<tr>
<td width="100%" class=TableBody1>
<b>操作成功：</b><br><br>
<?php   echo $sucmsg; ?>
</td></tr>
<tr align=center><td width="100%" class=TableBody2>
<?php
	if ($Desc=='') {
?>
<a href="<?php   echo $_SERVER['HTTP_REFERER']; ?>"> << 返回上一页</a>
<?php
	} else {
?>
<a href="<?php   echo $URL; ?>"> << <?php echo $Desc; ?></a>
<?php
	}
?>
</td></tr>
</table>
<?php 
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

function show_nav($boardName='')
{
	global $Banner;
	global $BannerURL;
	global $SiteName;
	global $SiteURL;
	global $StartTime;
	global $loginok;
	global $currentuser;

  html_init();
  if ($loginok==1) {
?>

<?php 
  }
?>
<body topmargin=0 leftmargin=0 onmouseover="HideMenu();">
<div id=menuDiv class="navClass1"></div>
<table cellspacing=0 cellpadding=0 align=center class="navClass2">
<tr><td width=100% >
<table width=100% align=center border=0 cellspacing=0 cellpadding=0>
<tr><td class=TopDarkNav height=9></td></tr>
<tr><td height=70 class=TopLighNav2>

<TABLE border=0 width="100%" align=center>
<TR>
<TD align=left width="25%"><a href="<?php  echo  $BannerURL; ?>" target="_blank"><img border=0 src='<?php echo  $Banner; ?>'></a></TD>
<TD Align=center width="65%">
<?php echo MAINTITLE; ?>
</td>
<td align=right style="line-height: 15pt" width="10%">
<a href=#><span style="CURSOR: hand" onClick="window.external.AddFavorite('<?php   echo $SiteURL; ?>', '<?php   echo $SiteName; ?>');">加入收藏</span></a>
<br><a href="">联系我们</a>
<br><a href="">论坛帮助</a>
</td>
</td></tr>
</table>

</td></tr>
<tr><td class=TopLighNav height=9></td></tr>
        <tr> 
          <td class=TopLighNav1 height=22  valign="middle">&nbsp;&nbsp;
<?php   
	if ($loginok!=1)  {
?>
<a href="logon.php">登陆</a> <img src=pic/navspacer.gif align=absmiddle> <a href="register.php">注册</a>
<?php  
	}  else  {
		echo '欢迎您 <b>'.$currentuser['userid'].'</b> ';
?>
<img src=pic/navspacer.gif align=absmiddle>
<a href="logon.php">重登陆</a> 
<img src=pic/navspacer.gif align=absmiddle>  <a href="#" onMouseOver='ShowMenu(manage,100)'>用户功能菜单</a>
<?php
 }
?>
<img src=pic/navspacer.gif align=absmiddle>  <a href="http://voicechat.zixia.net:10015/voicechat.htm?r=1" target=_blank>语音聊天室</a>
 <img src=pic/navspacer.gif align=absmiddle>  <a href="query.php<?php echo $boardName==''?'':'?boardName='.$boardName; ?>">搜索</a> 
 <img src=pic/navspacer.gif align=absmiddle>  <a href="#" onMouseOver='ShowMenu(stylelist,100)'>自选风格</a> 
 <?php    if ($loginok)
  {
?> <img src=pic/navspacer.gif align=absmiddle> <a href="logout.php">退出</a><?php   
} ?>
			</td>
        </tr>
</table>
</td></tr>
</table>
<?php 
	if ($loginok==1) {
		getMsg();
	}
} 

function head_var($Title='', $URL='',$showWelcome=0)
{
  GLOBAL $SiteName;
  GLOBAL $SiteURL;
  GLOBAL $stats;
  if ($URL=='') {
	  $URL=$_SERVER['PHP_SELF'];
  }
?>
<?php
  if ($showWelcome==1) {
?>
<br>
<table cellspacing=1 cellpadding=3 align=center border=0 width="97%">
<tr>
<td height=25>
>> 欢迎光临 <B><?php       echo $SiteName; ?></B>
</td></tr>
</table>
<?php
  } 
?>
<table cellspacing=1 cellpadding=3 align=center class=TableBorder2>
<tr><td height=25 valign=middle>
<img src="pic/forum_nav.gif" align=absmiddle> <a href="<?php echo $SiteURL; ?>"><?php   echo $SiteName; ?></a> → 
<?php 
	if ($Title!='') {
		echo  "<a href=".$URL.">".$Title."</a> → ";
	}
	echo $stats;
?>
<a name=top></a>
</td></td>
</table>
<br>
<?php 
} 

function show_footer()
{
  global $Version;
  global $Copyright;
  global $StartTime;

  $endtime=getmicrotime();
?>
<p>
<TABLE cellSpacing=0 cellPadding=0 border=0 align=center>
<tr>
	<td align=center>
		<a href="http://wforum.aka.cn/" target="_blank"><img border="0" src="/images/wforum.gif"></a><br>
		<nobr><?php   echo $Version; ?></nobr>
	</td>
	<td>
		<TABLE cellSpacing=0 cellPadding=0 border=0 align=center>
		<tr>
			<td align=center>
				<?php   echo $Forum_ads[1]; ?>
			</td>
		</tr>
		<tr>
		</tr>
		<tr>
			<td align=center nowrap>
				<?php   echo $Copyright; ?>
 				, 页面执行时间：<?php  printf(number_format(($endtime-$StartTime)*1000,3)); ?>毫秒
			</td>
		</tr>
		</table>
	</td>
</tr>
</TABLE>
<br>
<br>
<br>
</body>
</html>
<?php 

} 

function getMsg(){

?>

<div id="floater" style="position:absolute; width:502px; height:152px; z-index:2; left: 200px; top: 250px; visibility: hidden; background-color: transparent; layer-background-color: #FFFFFF; "> 
</div>
<iframe width="100%" height="0" frameborder="0" scrolling=no src="getmsg.php" name="webmsg">
</iframe>
<script src="inc/floater.js"  language="javascript"></script>
<?php
}

if (($needlogin!=0)&&($loginok!=1)&& ($guestloginok!=1) ){
	show_nav();
	foundErr("您尚未登陆！");
	html_error_quit();
	show_footer();
	exit(0);
	return;
}

if ( ($loginok==1) || ($guestloginok==1) ) {
	$yank=bbs_is_yank() ? 0 : 1;
	if ($setboard==1) 
		bbs_set_onboard(0,0);
}

} // !define ('_BBS_FUNCS_PHP_')
?>
