<?php
if (!bbs_ext_initialized()){
  bbs_init_ext();}

global $SQUID_ACCL;
global $BBS_PERM_POSTMASK;
global $BBS_PERM_NOZAP;
global $BBS_HOME;
global $BBS_FULL_NAME;
global $BBS_WAP_NAME;
define("BBS_WAP_NAME","水木清华");
global $urlmap;
global $utmpkey;
global $utmpnum;
global $userid;
global $sessionid;
global $havetoget;
global $backmenu;

//optional begin
//$fromhost=$_SERVER["REMOTE_ADDR"];
global $fromhost;
global $fullfromhost;
global $loginok;
global $currentuinfo;
global $currentuinfo_num;
global $currentuser;
global $currentuser_num;
global $cachemode;
$cachemode="";
$currentuinfo=array ();
$currentuser=array ();
$backmenu=1;
//optional end

$urlmap = array(
		"index" => "/wap/index.php",
		"login" => "/wap/login.php",
		"logout" => "/wap/logout.php",
		"menu" => "/wap/menu.php",
		"top10" => "/wap/t10.php",
		"sendmsg" => "/wap/smg.php",
		"readmail" => "/wap/rml.php"
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

//copy from funcs.php begin

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
bbs_setfromhost('M.'.trim($fromhost),'M.'.trim($fullfromhost));

$havetoget=1;
@$sessionid = $_GET["sid"];
if($sessionid == '')
{
  @$sessionid = $_COOKIE["SID"];
  $havetoget=0;
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

//copy end
define("ENCODESTRING", "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");

function decodesession($str)
{
  $value = 0;
  $uparray =  preg_split('//', strtoupper($str), -1, PREG_SPLIT_NO_EMPTY);
  $sidlen = count($uparray);

  for ($idx = 0; $idx < $sidlen; ++$idx)
    {
      $value = $value*36+strpos(ENCODESTRING, $uparray[$idx]);
    }
  return $value;
}

function encodesession($num, $len)
{
  $idx = 0;
  $numleft = $num;
  $retstr = "";
  while ($idx < $len)
    {
      $retstr = substr(ENCODESTRING, $numleft%36, 1).$retstr;
      $numleft = (int)($numleft/36);
      ++$idx;
    }
  return $retstr;
}

//TODO: detect code type of $strorig
function smarticonv($strorig)
{
  return iconv("UTF-8","GBK",$strorig);
}

function urlstr($url, $args=array())
{
  global $urlmap;
  global $sessionid;
  global $havetoget;
  $retstr = $urlmap[$url];
  if(count ($args) == 0)
    {
      return $retstr.($havetoget?'?sid='.$sessionid:'');
    }
  else
    {
      reset ($args);
      list ($key, $val) = each ($args);
      $retstr = $retstr."?".$key."=".$val;
      while (list ($key, $val) = each ($args))
	{
	  $retstr = $retstr."&amp;".$key."=".$val;
	}
      return $retstr.($havetoget?'&amp;sid='.$sessionid:'');
    }
}

function authpanic($message)
{
  global $backmenu;
  $backmenu = 0;
  waphead(0);
  echo "<card id=\"error\" title=\"".BBS_WAP_NAME."\">";
  echo "<p align=\"center\">".BBS_FULL_NAME."</p>";
  echo "<p>$message</p>";
  echo "<p align=\"center\">";
  echo "<a href=\"".urlstr('index')."\">重新登陆</a>";
  echo "</p>";
  echo "</card></wml>";
}

function loginok()
{
  global $sessionid;
  global $utmpkey;
  global $utmpnum;
  global $userid;
  global $currentuinfo;
  global $currentuinfo_num;
  global $currentuser;
  global $currentuser_num;
  global $havetoget;


  if (strlen($sessionid) != 9)
    {
      if (empty($sessionid))
	{
	  $error = bbs_wwwlogin(0);
	  if($error == 2 || $error == 0)
	    {
	      $data = array();
	      $num = bbs_getcurrentuinfo($data);
	      @$utmpkey = $data["utmpkey"];
	      @$utmpnum = $num;
	      @$userid = $data["userid"];
	      if (($ret=bbs_setonlineuser($userid,intval($utmpnum),intval($utmpkey),$currentuinfo,1))==0)
		{
		  $currentuinfo_num=bbs_getcurrentuinfo();
		  $currentuser_num=bbs_getcurrentuser($currentuser);
		}
	      return 1;
	    }
	  else
	    {
	      authpanic("guest登陆失败");
	      return 0;
	    }
	}
      else
	{
	  authpanic('错误的登陆信息');
	  return 0;
	}
    }
  else
    {
      $data = array();
      $numstr = $sessionid[0].$sessionid[1].$sessionid[2];
      $keystr = $sessionid[3].$sessionid[4].$sessionid[5].$sessionid[6].$sessionid[7].$sessionid[8];
      $utmpnum = decodesession($numstr);
      $utmpkey = decodesession($keystr);

      bbs_getonlineuser($utmpnum, $data);
      if($data["utmpkey"] != $utmpkey)
	{
	  authpanic('矛盾的登陆信息');
	  return 0;
	}
      else
	{
	  @$userid = $data['userid'];
	  if (($ret=bbs_setonlineuser($userid,intval($utmpnum),intval($utmpkey),$currentuinfo,$havetoget))==0) {
	    $currentuinfo_num=bbs_getcurrentuinfo();
	    $currentuser_num=bbs_getcurrentuser($currentuser);
	  }
	  return 1;
	}
    }
}

function checkmm()
{
  global $utmpkey;
  global $utmpnum;
  global $userid;

  $ret = bbs_getwebmsg($srcid,$msgbuf,$srcutmpnum,$sndtime);
  $ret2 = bbs_getmailnum($userid,$total,$unread, $oldtotal, $oldunread);
  
  if ($ret || ($ret2 && $unread))
    {
      echo "<card id=\"mm\" title=\"".BBS_WAP_NAME."\">";
      if ($ret)
	{
	  echo "<p align=\"left\">$srcid(".strftime("%H:%M", $sndtime)."):<br/>";
	  echo htmlspecialchars($msgbuf)."<br/>";
	  echo "<input emptyok=\"true\" type=\"text\" name=\"msg\" size=\"20\"/>";
	  echo "<br/>";
	  echo "<a href=\"".urlstr('sendmsg',array('m'=>'$(msg)','to'=>$srcid,'pid'=>decodesession($srcutmpnum,3)))."\">回复</a>";
	  echo "</p>";
	}
      if ($unread)
	{
	  echo "<p align=\"center\"><a href=\"".urlstr('readmail')."\"> ";
	  echo "你有新信!";
	  echo "</a></p>";
	}
      echo "<p>";
      echo "<do type=\"other\" label=\"继续\">";
      echo "<go href=\"#main\"></go>";
      echo "</do>";
      echo "</p>";
      echo "</card>";
      return 1;
    }
  else
    {
      return 0;
    }
}

function waphead($secs)
{
  global $backmenu;
  if ($secs == 0) $content = "no-cache";
  else $content = "max-age=$secs";
  header("Content-type:text/vnd.wap.wml;charset=gb2312");
  echo "<?xml version=\"1.0\" encoding=\"gb2312\"?>\n";
  echo"<!DOCTYPE wml PUBLIC \"-//WAPFORUM//DTD WML 1.1//EN\" \"http://www.wapforum.org/DTD/wml_1.1.xml\">\n";
  echo "<wml xml:lang=\"zh-cn\">\n";
  echo "<head>\n";
  echo "<meta http-equiv=\"Cache-Control\" content=\"$content\" forua=\"true\"/>\n";
  echo "</head>\n";
  
  //-----------------------------
  //add "return to menu " button
  if ($backmenu)
    {
      echo "<template>";
      echo "<do type=\"prev\" label=\"主菜单\">";
      echo "<go href=\"".urlstr('menu')."\"/>";
      echo "</do>";
      echo "</template>";
    }

}

?>
