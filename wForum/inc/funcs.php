<?php
	/**
	 * File included by all other php scripts.
	 * $Id$
	 */
if (!defined('_BBS_FUNCS_PHP_'))
{
define('_BBS_FUNCS_PHP_', 1);

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

function html_init($charset="",$title="",$otherheader="")
{
	global $_COOKIE;
	global $cachemode;
	global $HTMLTitle;
	global $HTMLCharset;
	global $DEFAULTStyle;
	global $stats;

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
	@$css_style = $_COOKIE["STYLE"];
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
<link rel="stylesheet" type="text/css" href="css/<?php echo $css_style; ?>"/>
<link rel="stylesheet" type="text/css" href="css/ansi.css"/>
<script src="/inc/funcs.js"  language="javascript"></script>
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

function show_nav()
{
	global $Banner;
	global $BannerURL;
	global $SiteName;
	global $SiteURL;
	global $StartTime;


  html_init();
?>
<script>
var stylelist = '<a style=font-size:9pt;line-height:12pt; href=\"cookies.asp?action=stylemod&skinid=0&boardid=0\">恢复默认设置</a><br><a style=font-size:9pt;line-height:12pt; href=\"cookies.asp?action=stylemod&skinid=1&boardid=0\">默认模板</a><br><a style=font-size:9pt;line-height:12pt; href=\"cookies.asp?action=stylemod&skinid=25&boardid=0\">水晶紫色</a><br><a style=font-size:9pt;line-height:12pt; href=\"cookies.asp?action=stylemod&skinid=26&boardid=0\">ｅ点小镇</a><br><a style=font-size:9pt;line-height:12pt; href=\"cookies.asp?action=stylemod&skinid=27&boardid=0\">心情灰色</a><br><a style=font-size:9pt;line-height:12pt; href=\"cookies.asp?action=stylemod&skinid=28&boardid=0\">秋意盎然</a><br><a style=font-size:9pt;line-height:12pt; href=\"cookies.asp?action=stylemod&skinid=29&boardid=0\">蓝色庄重</a><br><a style=font-size:9pt;line-height:12pt; href=\"cookies.asp?action=stylemod&skinid=32&boardid=0\">绿色淡雅</a><br><a style=font-size:9pt;line-height:12pt; href=\"cookies.asp?action=stylemod&skinid=34&boardid=0\">蓝雅绿</a><br><a style=font-size:9pt;line-height:12pt; href=\"cookies.asp?action=stylemod&skinid=35&boardid=0\">紫色淡雅</a><br><a style=font-size:9pt;line-height:12pt; href=\"cookies.asp?action=stylemod&skinid=36&boardid=0\">淡紫色</a><br><a style=font-size:9pt;line-height:12pt; href=\"cookies.asp?action=stylemod&skinid=37&boardid=0\">橘子红了</a><br><a style=font-size:9pt;line-height:12pt; href=\"cookies.asp?action=stylemod&skinid=38&boardid=0\">红红夜思</a><br><a style=font-size:9pt;line-height:12pt; href=\"cookies.asp?action=stylemod&skinid=40&boardid=0\">粉色回忆</a><br><a style=font-size:9pt;line-height:12pt; href=\"cookies.asp?action=stylemod&skinid=41&boardid=0\">青青河草</a><br><a style=font-size:9pt;line-height:12pt; href=\"cookies.asp?action=stylemod&skinid=42&boardid=0\">浓浓绿意</a><br><a style=font-size:9pt;line-height:12pt; href=\"cookies.asp?action=stylemod&skinid=44&boardid=0\">棕红预览</a><br><a style=font-size:9pt;line-height:12pt; href=\"cookies.asp?action=stylemod&skinid=45&boardid=0\">淡咖啡</a><br><a style=font-size:9pt;line-height:12pt; href=\"cookies.asp?action=stylemod&skinid=46&boardid=0\">碧海晴天</a><br><a style=font-size:9pt;line-height:12pt; href=\"cookies.asp?action=stylemod&skinid=47&boardid=0\">蓝色水晶</a><br><a style=font-size:9pt;line-height:12pt; href=\"cookies.asp?action=stylemod&skinid=48&boardid=0\">雪花飘飘</a><br>';
</script>
<body topmargin=0 leftmargin=0 onmouseover="HideMenu();>
<div id=menuDiv class="navclass1"></div>
<table cellspacing=0 cellpadding=0 align=center class="navclass2">
<tr><td width=100% >
<table width=100% align=center border=0 cellspacing=0 cellpadding=0>
<tr><td class=TopDarkNav height=9></td></tr>
<tr><td height=70 class=TopLighNav2>

<TABLE border=0 width="100%" align=center>
<TR>
<TD align=left width="25%"><a href="<?php  echo  $BannerURL; ?>" target="_blank"><img border=0 src='<?php echo  $Banner; ?>'></a></TD>
<TD Align=center width="65%">
<object classid="clsid:D27CDB6E-AE6D-11cf-96B8-444553540000" codebase="http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=5,0,0,0" width="468" height="60"><param name=movie value="http://www.dvbbs.net/skin/default/dvbanner.swf"><param name=quality value=high><param name=menu value=false><embed src="http://www.dvbbs.net/skin/default/dvbanner.swf" quality=high pluginspage="http://www.macromedia.com/shockwave/download/index.cgi?P1_Prod_Version=ShockwaveFlash" type="application/x-shockwave-flash" width="468" height="60"></embed></object>
</td>
<td align=right style="line-height: 15pt" width="10%">
<a href=#><span style="CURSOR: hand" onClick="window.external.AddFavorite('<?php   echo $SiteURL; ?>', '<?php   echo $SiteName; ?>')">加入收藏</span></a>
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
	if (!$Founduser)  {
?>
<a href="login.php">登陆</a> <img src=pic/navspacer.gif align=absmiddle> <a href="reg.php">注册</a>
<?php  
	}  else  {
?>
 <img src=pic/navspacer.gif align=absmiddle> <a href="login.php">重登陆</a> 
<?php
 }
?>
 <img src=pic/navspacer.gif align=absmiddle>  <a href="#" onMouseOver='ShowMenu(stylelist,100)'>自选风格</a> 
 <?php    if ($Founduser)
  {
?> <img src=<?php     echo $Forum_info[7]; ?>navspacer.gif align=absmiddle> <a href="logout.php">退出</a><?php   
} ?>
			</td>
        </tr>
</table>
</td></tr>
</table>
<br>
<br>
<?php 

} 

function head_var($IsBoard,$idepth,$GetTitle,$GetUrl)
{
  extract($GLOBALS);


?>
<table cellspacing=1 cellpadding=3 align=center border=0 width="97%">
<tr>
<?php   if (!$Founduser)
  {
?>
<td height=25>
<BR>
>> <?php     if ($FoundBoard)
    {
?><?php       echo $BoardReadme; ?><?php     }
      else
    {
?>欢迎光临 <B><?php       echo $Forum_info[0]; ?></B><?php     } ?>
<?php   }
    else
  {
?>
<td width=65% >
</td><td width=35% align=right>
<?php     if (intval(newincept())>intval(0))
    {
?>
<bgsound src="<?php       echo $Forum_info[7].$Forum_statePic[8]; ?>" border=0>
<?php       if (intval($Forum_setting[10])==1)
      {
?>
<script language=JavaScript>openScript('messanger.php?action=read&id=<?php         echo inceptid(1); ?>&sender=<?php         echo inceptid(2); ?>',500,400)</script>
<?php       } ?>
<img src=<?php       echo $Forum_info[7].$Forum_boardpic[9]; ?>> <a href="usersms.php?action=inbox">我的收件箱</a> (<a href="javascript:openScript('messanger.php?action=read&id=<?php       echo inceptid(1); ?>&sender=<?php       echo inceptid(2); ?>',500,400)"><font color="<?php       echo $Forum_body[8]; ?>"><?php       echo newincept(); ?> 新</font></a>)
<?php     }
      else
    {
?>
<img src=<?php       echo $Forum_info[7].$Forum_boardpic[8]; ?>> <a href="usersms.php?action=inbox">我的收件箱</a> (<font color=gray>0 新</font>)
<?php     } ?>
<?php   } ?>
</td></tr>
</table>
<table cellspacing=1 cellpadding=3 align=center class=tableBorder2>
<tr><td height=25 valign=middle>
<img src="<?php   echo $Forum_info[7].$Forum_pic[12]; ?>" align=absmiddle> <a href=index.php><?php   echo $Forum_info[0]; ?></a> → 
<?php 
  if ($IsBoard==1)
  {

    if ($BoardParentID>0)
    {

      for ($i=0; $i<=$idepth-1; $i=$i+1)
      {

        print "<a href=list.php?boardid=".$FBoardID[$i].">".$FBoardName[$i]."</a> →  ";
        if ($i>9)
        {
          break;
        } 


      } 

    } 

    if ($_REQUEST['CatLog']=="NN")
    {

            $_COOKIE['BoardList'.$Boardid."BoardID"]="NNotShow";
    } 

    print "<a href=list.php?boardid=".$Boardid.">".$boardtype."</a> →  ".HTMLEncode($Stats);
    if ($_COOKIE['BoardList'.$Boardid."BoardID"]=="NNotShow")
    {
      print "&nbsp;<a href=\"?BoardID=".$Boardid."&cBoardID=".$Boardid."&Catlog=Y\" title=\"展开论坛列表\">[展开]</a>";
    } 

  }
    else
  if ($IsBoard==2)
  {

    print HTMLEncode($Stats);
  }
    else
  {

    print "<a href=".$GetUrl.">".$GetTitle."</a> → ".HTMLEncode($Stats);
  } 

?>
<a name=top></a>
</td></td>
</table>
<br>
<?php 
  return $function_ret;
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
<tr><td align=center>
<?php   echo $Forum_ads[1]; ?>
</td></tr>
<tr><td align=center nowrap>
<?php   echo $Version; ?><br>
<?php   echo $Copyright; ?>
 , 页面执行时间：<?php  printf(number_format(($endtime-$StartTime)*1000,3)); ?>毫秒
</td></tr>
</table>
</body>
</html>
<?php 

} 

if ((!isset($needlogin)||($needlogin!=0))&&($loginok!=1)&&($_SERVER["PHP_SELF"]!="/bbslogin.php")) {
	error_nologin();
	return;
}

if (($loginok==1)&&(isset($setboard)&&($setboard==1))) bbs_set_onboard(0,0);


} // !define ('_BBS_FUNCS_PHP_')
?>
