<?php
if (!defined('_BBS_FUNCS_PHP_'))
{
define('_BBS_FUNCS_PHP_', 1);

$Version="Powered by wForum Version 0.9";
$Copyright=
"<a href=\"http://www.aka.cn/\" target=\"_blank\">阿卡信息技术(北京)有限公司</a> &amp; "
. "<a href=\"http://dev.kcn.cn/\" target=\"_blank\">KBS 开发组</a> 版权所有 2005 <br/>"
. "<a href=\"http://wforum.zixia.net/\" target=\"_blank\">"
. "<font face=\"Verdana, Arial, Helvetica, sans-serif\" size=\"1\" color=\"#6000CC\">"
. "<b>wForum</b></font></a> "
. "<a href=\"http://www.zixia.net/\" target=\"_blank\">"
. "<font face=\"Verdana, Arial, Helvetica, sans-serif\" size=\"1\" color=\"#6000CC\">"
. "<b>@zixia.net</b></font></a>";

if (!isset($needlogin)){ //本页面是否需要设置cookie等登录变量，默认需要
	$needlogin=1;
}

if ($needlogin && ((!isset($_COOKIE['iscookies'])) || ($_COOKIE['iscookies']=='')))
{
	setcookie('iscookies','0',time()+3650*24*3600);
?>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=gb2312">
<meta HTTP-EQUIV="REFRESH" CONTENT="3">
<META NAME="DESCRIPTION" CONTENT="wForum，基于高性能的KBS系统源码（一万五千人同时在线）的高性能论坛系统">
<META NAME="KEYWORDS" CONTENT="wForum,aka,阿卡信息技术(北京)有限公司,zixia,论坛">
</head>
<body>
正在登录论坛……<br><br>本系统要求使用COOKIES，假如您的浏览器禁用COOKIES，您将不能登录本系统……
<span style="display:none"><?php   echo $Copyright; ?></span>
</body>
</html>
<?php
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
if (BUILD_PHP_EXTENSION==0)
	@dl("libphpbbslib.so");

chdir(BBS_HOME);
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
global $loginok; //是否以正式用户登录了
global $guestloginok;
//global $currentuinfo;
global $currentuinfo_num;
//global $currentuser;
global $currentuuser_num;
global $cachemode;
$yank=1;

$loginok=0;
$guestloginok=0;

/* 本文件内部变量 */
$needloginok = false; //本页面是否必须正式用户才可以访问，
$showedbanner = false; //是否已经显示过 banner
$stats=''; //页面标题
$errMsg=''; //错误信息
$foundErr = false; //是否有错误
$sucmsg=''; //成功信息


if (!isset($nologin)) {
	$nologin=0;
}

if (!isset($setboard)){
	$setboard=0;
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
require("wForum.site.php");


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

/* used by cache_header() and update_cache_header()，不应该直接被调用 */
function cache_process($scope, $forcecachetime, $modifytime, $expiretime) {
	global $cachemode;
	//session_cache_limiter($scope);
	$cachemode=$scope;
	if ($scope=="nocache") {
		header("Expires: Thu, 19 Nov 1981 08:52:00 GMT");
		header("Cache-Control: no-store, no-cache, must-revalidate");
		header("Pragma: no-cache");
		return FALSE;
	}
	@$oldmodified=$_SERVER["HTTP_IF_MODIFIED_SINCE"];
	if ($oldmodified!="") {
		$oldtime = strtotime($oldmodified) + $forcecachetime;
	} else $oldtime=0;
	if ($oldtime >= $modifytime) {
		header("HTTP/1.1 304 Not Modified");
		header("Cache-Control: max-age=" . "$expiretime");
		return TRUE;
	}
	header("Last-Modified: " . gmdate("D, d M Y H:i:s", $modifytime) . " GMT");
	header("Expires: " . gmdate("D, d M Y H:i:s", time()+$expiretime) . " GMT");
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

function html_init($charset="",$otherheader="",$is_mathml=false)
{
	global $cachemode;
	global $HTMLTitle;
	global $HTMLCharset;
	global $DEFAULTStyle;
	global $stats;

	//session_start();
	if ($charset==""){
		$charset=$HTMLCharset;
	}
	$title = $HTMLTitle;
	if (isset($stats) ){
		$title = $title . ' -- ' . $stats;
	}
	if ($cachemode=="") {
		cache_header("nocache");
	}
	if ($is_mathml) {
		if (strpos($_SERVER["HTTP_USER_AGENT"], "MSIE") === false) {
			$eee = " encoding=\"$charset\"";
		} else {
			$eee = ""; //fuck, IE doesn't understand that.
			if (stristr($_SERVER["HTTP_USER_AGENT"], "MathPlayer") === false) {
				$is_mathml = false;
			}
		}
	}
	if ($is_mathml) {
		header("Content-Type: application/xhtml+xml");
?>
<?xml version="1.0"<?php echo $eee; ?>?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1 plus MathML 2.0//EN"
               "http://www.w3.org/TR/MathML2/dtd/xhtml-math11-f.dtd">
<html xmlns="http://www.w3.org/1999/xhtml"
xmlns:math="http://www.w3.org/1998/Math/MathML">
<?php
	} else {
?>
<?xml version="1.0" encoding="<?php echo $charset; ?>"?>
<!DOCTYPE html
	 PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<?php
	}
?>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=<?php echo $charset; ?>"/>
<title><?php echo $title; ?></title>
<link rel="stylesheet" type="text/css" href="css/ansi.css"/>
<link rel="stylesheet" type="text/css" href="css/common.css"/>
<script src="inc/browser.js"  language="javascript"></script>
<script src="inc/funcs.js"  language="javascript"></script>
<script type="text/javascript">
<!--
	writeStyleSheets();
//-->
</script>
<?php echo($otherheader); ?>
</head>
<?php
}


function showLogon($showBack = 0, $comeurl = "") {
	if ($comeurl == "") {
		if (isset($_SERVER['HTTP_REFERER'])) {
			$comeurl = $_SERVER['HTTP_REFERER'];
		}
	}
?>
	<table cellpadding="3" cellspacing="1" align="center" class="TableBorder1" style="width: 75%;">
	<form action="logon.php" method="post"> 
	<tr>
	<th valign="middle" colspan="2" align="center" height="25">请输入您的用户名、密码登录</th></tr>
	<tr>
	<td valign="middle" class="TableBody1">请输入您的用户名</td>
	<td valign="middle" class="TableBody1"><input name="id" id="idF" type="text" tabindex="1"/> &nbsp; <a href="register.php">没有注册？</a></td></tr>
	<tr>
	<td valign="middle" class="TableBody1">请输入您的密码</td>
	<td valign="middle" class="TableBody1"><input name="passwd" type="password" tabindex="2" /> &nbsp; <!--<a href="foundlostpass.php">忘记密码？</a>--></td></tr>
	<tr>
	<td class="TableBody1" valign="top" width="30%" ><b>Cookie 选项</b><br/> 请选择你的 Cookie 保存时间，下次访问可以方便输入。</td>
	<td valign="middle" class="TableBody1">
	<input type="radio" name="CookieDate" value="0" checked="checked" />不保存，关闭浏览器就失效<br/>
				<input type="radio" name="CookieDate" value="1" />保存一天<br/>
				<input type="radio" name="CookieDate" value="2" />保存一月<br/>
				<input type="radio" name="CookieDate" value="3" />保存一年<br/>                
	</td></tr>
	<input type="hidden" name="comeurl" value="<?php echo htmlspecialchars($comeurl); ?>">
	<tr>
	<td class="TableBody2" valign="middle" colspan="2" align="center"><input tabindex="3" type="submit" name="submit" value="登 录" />
<?php
	if ($showBack) {
?>
	&nbsp;&nbsp;<input type="button" name="back" value="返 回" onclick="location.href='<?php echo htmlspecialchars($comeurl, ENT_QUOTES); ?>'"/>
<?php
	}
?>
	</td></tr></form></table>
<script language="Javascript">
<!--
	registerFocusOnLoad("idF");
//-->
</script>
<?php
}

function requireLoginok($msg = false, $directexit = true) {
	global $loginok;
	global $needloginok;
	global $showedbanner;
	$needloginok = 1;
	if ($loginok == 1) return;
	foundErr(($msg === false) ? "本页需要您以正式用户身份登录之后才能访问！" : $msg, $directexit, false);
}

function setSucMsg($msg){
	global $sucmsg;
	$sucmsg.='<li>'.$msg.'</li>';
}
function setStat($stat){
	GLOBAL $stats;
	$stats=$stat;
}

function foundErr($msg, $directexit = true, $showmsg = true){ //ToDo: non-standard HTML - atppp
	global $errMsg;
	global $foundErr;
	global $showedbanner;
	$errMsg.='<li>'.$msg.'</li>';
	$foundErr=true;
	if ($directexit) {
		if (!$showedbanner) show_nav();
		show_footer($showmsg, true);
		exit;
	}
}

function isErrFounded(){
	GLOBAL $foundErr;
	return $foundErr;
}

function get_frame_referer() {
	if (!isset($_SERVER["HTTP_REFERER"]) || ( $_SERVER["HTTP_REFERER"]=="") ) {
		return "index.php";
	} else {
		$target = $_SERVER["HTTP_REFERER"];
		$l_target = strtolower($target);
		if ((strpos($l_target,'bbsleft.php')!==false)) return "index.php";
		if ((strpos($l_target,'frames.php')!==false)) return "index.php";
		return $target;
	}
}

function html_error_quit()
{
	global $errMsg;
	global $needloginok;
	global $loginok;
?>
<br/>
<table cellpadding="3" cellspacing="1" align="center" class="TableBorder1" style="width: 75%;">
<tr align="center">
<th height="25">论坛错误信息</th>
</tr>
<tr>
<td class="TableBody1">
<b>产生错误的可能原因：</b>
<ul>
<li>您是否仔细阅读了帮助文件，可能您还没有登录或者不具有使用当前功能的权限。</li>
<?php   echo $errMsg; ?>
</ul>
</td></tr>
<?php
	if (($needloginok!=0)&&($loginok!=1)) {
		echo "</table>";
  		showLogon(1);
  	} else {
?>
	<tr>
	<td class=TableBody2 valign=middle align=center><a href="<?php echo htmlspecialchars(get_frame_referer(), ENT_QUOTES); ?>"> <<返回上一页 </a></td></tr></table>
<?php
	}
} 

function html_success_quit($Desc='',$URL='')
{
  global $sucmsg;
?>
<br/>
<table cellpadding="3" cellspacing="1" align="center" class="TableBorder1" style="width: 75%;">
<tr align="center">
<th width="100%">论坛成功信息</th>
</tr>
<tr>
<td width="100%" class="TableBody1">
<b>操作成功：</b>
<ul>
<?php   echo $sucmsg; ?>
</ul>
</td></tr>
<tr align=center><td width="100%" class="TableBody2">
<?php
	if ($Desc=='') {
?>
<a href="<?php   echo htmlspecialchars(get_frame_referer(), ENT_QUOTES); ?>"> &lt;&lt; 返回上一页</a>
<?php
	} else {
?>
<a href="<?php   echo $URL; ?>"> &lt;&lt; <?php echo $Desc; ?></a>
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

function show_nav($boardName='',$is_mathml=false,$other_headers="")
{
	global $Banner;
	global $SiteName;
	global $SiteURL;
	global $loginok;
	global $currentuser;
	global $currentuinfo;
	global $showedbanner;
	global $needloginok;
	$showedbanner = true;

	html_init("",$other_headers,$is_mathml);
?>
<script language="javascript">
<!--
	var siteconf_SMS_SUPPORT = <?php echo SMS_SUPPORT ? 1 : 0; ?>;
	var siteconf_BOARDS_PER_ROW = <?php echo BOARDS_PER_ROW; ?>;
	var siteconf_SHOW_POST_UNREAD = <?php echo SHOW_POST_UNREAD  ? 1 : 0; ?>;
	var siteconf_THREADSPERPAGE = <?php echo THREADSPERPAGE; ?>;
	defineMenus();
//-->
</script>
<body style="margin: 1px;" onmouseover="HideMenu(event);">
<div id="menuDiv" class="navClass1"></div>
<table cellspacing="0" cellpadding="0" align="center" class="navClass2">
<tr><td width="100%">
<table width="100%" align="center" border="0" cellspacing="0" cellpadding="0">
<tr><td class="TopDarkNav" height="9"></td></tr>
<tr><td height="70" class="TopLighNav2">

<table border="0" width="100%" align="center">
<tr>
<td align="left" width="25%"><a href="index.php"><img border="0" src='<?php echo  $Banner; ?>'/></a></td>
<td align="center" width="65%">
<?php echo MAINTITLE; ?>
</td>
<td align="right" style="line-height: 15pt" width="10%">
<nobr><a href="http://www.aka.cn/">联系我们</a></nobr><br/>
<nobr><a href="index.php">论坛帮助</a></nobr>
</td></tr>
</table>

</td></tr>
<tr><td class="TopLighNav" height="9"></td></tr>
<?php
	if ($boardName !== false) {
?>
		<tr> 
		  <td class="TopLighNav1" height="22" valign="middle">&nbsp;&nbsp;
<?php   
	if ($loginok!=1)  {
?>
<a href="logon.php">登录</a> <img src="pic/navspacer.gif" align="absmiddle"/> <a href="register.php">注册</a>
<?php  
	}  else  {
		echo '欢迎您 <b>'.$currentuser['userid'].'</b> ';
		if ($currentuser["userlevel"] & BBS_PERM_CLOAK) {
?>
<img src="pic/navspacer.gif" align="absmiddle"/>
<a href="changecloak.php"><?php echo $currentuinfo["invisible"]?"现身":"隐身"; ?></a> 
<?php
		}
?>
<img src="pic/navspacer.gif" align="absmiddle"/> <a href="logon.php">重登录</a> 
<img src="pic/navspacer.gif" align="absmiddle"/> <a href="usermanagemenu.php" onmouseover="ShowMenu(manage,100,event)">用户功能菜单</a>
<img src="pic/navspacer.gif" align="absmiddle"/> <a href="#" onmouseover="ShowMenu(talk,100,event)">谈天说地菜单</a>
<?php
 }
 if (AUDIO_CHAT) {
?>
<img src="pic/navspacer.gif" align="absmiddle"/>  <a href="http://voicechat.zixia.net:10015/voicechat.htm?r=1" target="_blank">语音聊天室</a>
<?php
 }
?>
 <img src="pic/navspacer.gif" align="absmiddle"/>  <a title="搜索当前版面" href="query.php<?php echo $boardName==''?'':'?boardName='.$boardName; ?>" onmouseover="ShowMenu(query,100,event)">搜索</a> 
 <img src="pic/navspacer.gif" align="absmiddle"/>  <a href="#" onmouseover="ShowMenu(stylelist,100,event)">自选风格</a> 
<?php 
	if ($loginok) {
?>
<img src="pic/navspacer.gif" align="absmiddle"/> <a href="logout.php<?php if ($needloginok!=0) echo "?jumphome=1"; ?>">注销</a>
<?php   
	}
?>
			</td>
		</tr>
<?php
	}
?>
</table>
</td></tr>
</table>
<?php 
} 

function head_var($Title='', $URL='',$showWelcome=0)
{
  GLOBAL $SiteName;
  GLOBAL $stats;
  if ($URL=='') {
	  $URL=$_SERVER['PHP_SELF'];
  }
?>
<?php
  if ($showWelcome==1) {
?>
<br/>
<table cellspacing="1" cellpadding="3" align="center" border="0" width="97%">
<tr>
<td height="25">
&gt;&gt; 欢迎光临 <B><?php       echo $SiteName; ?></B>
</td></tr>
</table>
<?php
  } 
?>
<table cellspacing="1" cellpadding="3" align="center" class="TableBorder2">
<tr><td><nobr>
<img src="pic/forum_nav.gif" align="absmiddle"/> <a href="index.php"><?php   echo $SiteName; ?></a> → 
<?php 
	if ($Title!='') {
		echo  "<a href=".$URL.">".$Title."</a> → ";
	}
	echo $stats;
?>
</nobr></td>
<td align="right" id="serverTime"> </td>
</tr>
</table>
<?php
	if (SHOW_SERVER_TIME) {
?>
<script language="JavaScript">
<!--
	initTime(<?php echo time()+intval(date("Z")); ?>);
//-->
</script>
<?php
	}
?>
<br/>
<?php 
} 

function show_footer($showmsg = true, $showerr = true)
{
  global $Version;
  global $Copyright;
  global $StartTime;
  global $FooterBan;
  global $loginok;
  global $conn;

  if ($showerr) {
  	if (isErrFounded()) {
  		html_error_quit();
  	}
  }
  $endtime=getmicrotime();
?>
<p>
<table cellSpacing="0" cellPadding="0" border="0" align="center">
<tr>
	<td align="center">
	    <a href="http://wforum.aka.cn/" target="_blank"><img border="0" src="images/wforum.gif"/></a>
		<a href="http://dev.kcn.cn/" target="_blank"><img border="0" src="images/poweredby.gif"/></a>&nbsp;&nbsp;<br/>
		<nobr><?php   echo $Version; ?></nobr>
	</td>
	<td>
		<table cellSpacing="0" cellPadding="0" border="0" align="center">
		<tr>
			<td align="center" nowrap="nowrap">
				<?php   echo $Copyright; ?>
				, 页面执行时间：<?php  printf(number_format(($endtime-$StartTime)*1000,3)); ?>毫秒
			</td>
		</tr>
		</table>
	</td>
</tr>
<tr>
<td colspan="2"><?php if (isset($FooterBan)) echo $FooterBan ; ?></td>
</tr>
</table></p>
<?php
	if ($loginok==1 && $showmsg) {
		getMsg();
	}
?>
<br/>
<br/>
</body>
</html>
<?php
	if (isset($conn) && ($conn !== false)) CloseDatabase();
} 

function getMsg(){
?>
<div id="floater" style="position:absolute; width:502px; height:152px; z-index:2; left: 200px; top: 250px; visibility: hidden; background-color: transparent; layer-background-color: #FFFFFF; "> 
</div>
<iframe width="100%" height="0" frameborder="0" scrolling="no" src="getmsg.php" name="webmsg">
</iframe>
<script src="inc/floater.js"  language="javascript"></script>
<?php
}

function htmlformat($str,$multi=false) {
    $str = str_replace(' ','&nbsp;',htmlspecialchars($str, ENT_QUOTES));
    if ($multi)
        $str = nl2br($str);
    return $str;    
}

function jumpReferer($forcehome = false) {
	if ($forcehome) {
		$target = "index.php";
	} else {
		$target = get_frame_referer();
	}
	//header("Location: $target");
?>
<script language="JavaScript">
<!--
    if (top == self) {
        location.href = "<?php echo addslashes($target); ?>";
    } else {
        top.location.href = "frames.php?target=<?php echo urlencode($target); ?>";
    }
//-->
</script>
<?php
}


define("ENCODESTRING","0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");
function decodesessionchar($ch)
{
	return strpos(ENCODESTRING,$ch);
}


/* 登录相关代码 */
$loginok=0;

function setfromhost() {
    global $fullfromhost;
    global $fromhost;
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
}
setfromhost();

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
	@$utmpkey = $_COOKIE[COOKIE_PREFIX."UTMPKEY"];
	@$utmpnum = $_COOKIE[COOKIE_PREFIX."UTMPNUM"];
	@$userid = $_COOKIE[COOKIE_PREFIX."UTMPUSERID"];
	@$userpassword=$_COOKIE[COOKIE_PREFIX."PASSWORD"];
}
if ($userid=='') {
	$userid='guest';
}

$setonlined=0;
$setidcookie=true;
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
				if (($userid!='guest') && ($userpassword != '') && (bbs_checkpasswd($userid,base64_decode($userpassword),1)==0)){
					$ret=bbs_wwwlogin(1);
					if ( ($ret==2) || ($ret==0) ){
						$loginok=1;
						$setidcookie=false;
					}else if ($ret==5) {
						foundErr("请勿频繁登录！");
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
	if ($setidcookie) {
		setcookie(COOKIE_PREFIX."UTMPUSERID",$data["userid"],time()+360000,COOKIE_PATH,COOKIE_DOMAIN);
	}
	setcookie(COOKIE_PREFIX."UTMPKEY",$data["utmpkey"],time()+360000,COOKIE_PATH,COOKIE_DOMAIN);
	setcookie(COOKIE_PREFIX."UTMPNUM",$currentuinfo_num,time()+360000,COOKIE_PATH,COOKIE_DOMAIN);
	setcookie(COOKIE_PREFIX."LOGINTIME",$data["logintime"],time()+360000,COOKIE_PATH,COOKIE_DOMAIN);
}


if (($needlogin!=0)&&($loginok!=1)&& ($guestloginok!=1) ){
	foundErr("您尚未登录！");
}

if ( ($loginok==1) || ($guestloginok==1) ) {
	$yank = 0;
	if ($setboard==1) 
		bbs_set_onboard(0,0);
}


} // !define ('_BBS_FUNCS_PHP_')
?>
