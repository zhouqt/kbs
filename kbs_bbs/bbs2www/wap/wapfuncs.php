<?php

if (BUILD_PHP_EXTENSION==0)
@dl("libphpbbslib.so");

if (!bbs_ext_initialized()){
  bbs_init_ext();}

global $SQUID_ACCL;
global $BBS_PERM_POSTMASK;
global $BBS_PERM_NOZAP;
global $BBS_HOME;
global $BBS_FULL_NAME;
global $BBS_WAP_NAME;
define("BBS_WAP_NAME","Ë®Ä¾Çå»ª");
global $urlmap;
global $utmpkey;
global $utmpnum;
global $userid;
global $sessionid;
global $havetoget;

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
//optional end

$urlmap = array(
		"index" => array("/wap/index.php"),
		"login" => array("/wap/login.php"),
		"logout" => array("/wap/logout.php"),

		"menu" => array("/wap/menu.php"),
		"sysinfo" => array("/wap/sys.php"),
		"top10" => array("/wap/t10.php"),
		"sendmsg" => array("/wap/smg.php"),
	     
		"onlinefriend" => array("/wap/ofd.php"),
		"showuser" => array("/wap/sur.php"),

		"favboard" => array("/wap/fav.php",
				    "select",
				    "up",
				    "page"),
		"searchboard" => array("/wap/seb.php",
				       "page"),
		"showboard" => array("/wap/sbd.php",
				     "board",
				     "page",
				     "mode",
				     "secpage",),
		"searchpost" => array("/wap/sep.php"),//todo
		/* semi-seperated */
		"readpost" => array("/wap/rpt.php",
				    "board",
				    "id",
				    "way",
				    "page"),
		"writepost" => array("/wap/wpt.php",
				     "board",
				     "reid"),
		//TODO
		"mailbox" => array("/wap/mbx.php"),
		"listmail" => array("/wap/lml.php",
				    "mbox",
				    "page"),
		"readmail" => array("/wap/rml.php",
				    "mbox",
				    "mailid",
				    "page"),
		"writemail" => array("/wap/wml.php"),
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

//if client doesn't support cookie, havatoget will be set.
//otherwise havetoget will be set zero.
$havetoget=1;
@$sessionid = $_GET["sid"];
if($sessionid == '')
{
  @$sessionid = $_COOKIE["S"];
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

function bbs_check_board_flag($board,$flag)
{
  if ($board["FLAG"] & $flag)
    return TRUE;
  else
    return FALSE;
}

function bbs_is_readonly_board($board)
{
  global $BOARD_FLAGS;
  return bbs_check_board_flag($board, $BOARD_FLAGS["READONLY"]);
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

function encodesession($num, $len=0)
{
  $idx = 0;
  $numleft = $num;
  $retstr = "";
  if ($len != 0)
    {
      while ($idx < $len)
	{
	  $retstr = substr(ENCODESTRING, $numleft%36, 1).$retstr;
	  $numleft = (int)($numleft/36);
	  ++$idx;
	}
    }
  else
    {
      while ($numleft != 0)
	{
	  $retstr = substr(ENCODESTRING, $numleft%36, 1).$retstr;
	  $numleft = (int)($numleft/36);
	  ++$idx;      
	}
    }

  return $retstr;
}

function encarg($url,$argv)
{
  global $urlmap;
  $total = count ( $urlmap["$url"] ) - 1;
  $bitmap = 0;
  $args = "";
  while ($total > 0)
    {
      $bitmap=($bitmap<<1);
      $value = intval($argv[$urlmap[$url][$total]]);
      if ($value != 0)
	{
	  $args = $args.(empty($args)?"":"-").encodesession($value);
	  $bitmap++;
	}
      $total--;
    }
  return encodesession($bitmap,1).$args;
}

function decarg($url,$args)
{
  global $urlmap;
  $bitmap = decodesession(substr($args,0,1));
  $argv = explode("-",substr($args,1));
  $pos = 1;
  $rpos = count($argv) -1;
  $ret = array();
  while ($bitmap > 0)
    {
      if (($bitmap%2) == 1)
	{
	  $ret[$urlmap[$url][$pos]] = decodesession($argv[$rpos]);
	  $rpos--;
	}
      $bitmap=($bitmap>>1);
      $pos++;
    }
  return $ret;
}

//TODO: detect code type of $strorig
function smarticonv($strorig)
{
    return iconv("UTF-8","GBK",$strorig);
  //  return $strorig;
}

function urlstr($url, $args=array(), $extarg="")
{
  global $urlmap;
  global $sessionid;
  global $havetoget;
  $retstr = $urlmap[$url][0];
  if(count ($args) == 0)
    {
      return $retstr.(empty($extarg) && empty($args) ? "":"?")
	.(empty($extarg) ? "" : "x=".$extarg)
	.($havetoget ? "&amp;sid=".$sessionid : "");
    }
  else
    {
      reset ($args);
      list ($key, $val) = each ($args);
      $retstr = $retstr."?".(empty($extarg)?"":"x=".$extarg."&amp;")
	.$key."=".$val;
      while (list ($key, $val) = each ($args))
	{
	  $retstr = $retstr."&amp;".$key."=".$val;
	}
      return $retstr.($havetoget?"&amp;sid=".$sessionid:"");
    }
}

function showlink($href,$hint)
{
  echo "<a href=\"$href\">$hint</a>";
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
  showlink(urlstr('index'),"ÖØÐÂµÇÂ½");
  echo "</p>";
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
	      authpanic("guestµÇÂ½Ê§°Ü");
	      return 0;
	    }
	}
      else
	{
	  authpanic('´íÎóµÄµÇÂ½ÐÅÏ¢');
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
	  authpanic('Ã¬¶ÜµÄµÇÂ½ÐÅÏ¢');
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
	  echo "<p>$srcid(".strftime("%H:%M", $sndtime)."):<br/>";
	  echo htmlspecialchars($msgbuf)."<br/>";
	  echo "<input name=\"m\" size=\"20\"/><br/>";
	  showlink(urlstr('sendmsg',array('m'=>'$(m)','t'=>$srcid,'i'=>decodesession($srcutmpnum,3))),"»Ø¸´");
	  echo "</p>";
	}
      if ($unread)
	{
	  echo "<p>";
	  showlink(urlstr('listmail'),"ÄãÓÐÐÂÐÅ!");
	  echo "</p>";
	}
      echo "<p>";
      showlink("#main","¼ÌÐø");
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
  
}

function stripansi($origstr)
{
  return  preg_replace("/\[[0-9\;]*[a-zA-Z@]/","",$origstr);
}

function ch_substr($string, $start=0, $length=0) {
  if ($start > 0) {
    $bound_test = substr($string, 0, $start);
    if (preg_match("/[x7F-xFF]$/", $bound_test)) {
      $start++;
    }
  }
  if ($length > 0) {
    $out = substr($string, $start, $length);
    if (preg_match("/[x7F-xFF]$/", $out)) {
      $out = substr($string, $start, ++$length);
    }
    return $out;
  } else {
    return substr($string, $start);
  }
}

function loadmboxes($ownid)
{
      $mail_boxes["path"] = array(".DIR",".SENT",".DELETED");
      $mail_boxes["title"] = array("ÊÕ¼þÏä","·¢¼þÏä","À¬»øÏä");
      $mail_boxes["nums"] = array(bbs_getmailnum2(bbs_setmailfile($ownid,".DIR")),bbs_getmailnum2(bbs_setmailfile($ownid,".SENT")),bbs_getmailnum2(bbs_setmailfile($ownid,".DELETED")));
      //custom mailboxs
      $mail_cusbox = bbs_loadmaillist($ownid);
      $totle_mails = $mail_boxnums[0]+$mail_boxnums[1]+$mail_boxnums[2];
      $i = 2;
      if ($mail_cusbox != -1){
	foreach ($mail_cusbox as $mailbox){
	  $i++;
	  $mail_boxes["path"][$i] = $mailbox["pathname"];
	  $mail_boxes["title"][$i] = $mailbox["boxname"];
	  $mail_boxes["nums"][$i] = bbs_getmailnum2(bbs_setmailfile($ownid,$mailbox["pathname"]));
	}
      }
      return $mail_boxes;
}
?>
