<?php
require("wapfuncs.php");
if(loginok())
{
  if ($userid == "guest")
    {
      authpanic("过客没有信箱");
    }
  else
    {
      $argv = decarg("readmail",$_GET['x']);
      $mboxnum = intval($argv["mbox"]);
      $mailid = intval($argv["mailid"]);
      $page = intval($argv["page"]);
      $mailboxes = loadmboxes($userid);
      waphead(0);
      echo "<card id=\"main\" title=\"".BBS_WAP_NAME."\">";
      echo "<p>";
      if ($mboxnum >= count($mailboxes["path"])) {
        echo "错误的邮箱号<br/>";
      }
      else {
	$dir = "mail/".strtoupper($userid{0})."/".$userid."/".$mailboxes["path"][$mboxnum];
	$articles = array ();
        if( bbs_get_records_from_num($dir, $mailid, $articles) ) {
	  $file = $articles[0]["FILENAME"];
	  $mailfrom = $articles[0]["OWNER"];
	  $filename = "mail/".strtoupper($userid{0})."/".$userid."/".$file ;
	  if(! file_exists($filename)){
	    echo "信件不存在...<br/>";
	  }
	  else{
	    wapshowmail($filename, $page, $mboxnum, $mailid, $mailfrom);
	    bbs_setmailreaded($dir, $mailid);
	  }
        }else{
	  echo "错误的参数<br/>";
        }

      }
      showlink(urlstr("menu"),"主菜单");
      echo "</p>";
    }
}
?>
</card>
</wml>

<?php
function wapshowmail($fname, $page, $mbox, $maild, $target)
{
  $pagesize = 800;
  $fhandle = @fopen($fname, "rb");
  $fdata = "";
  $fbuf = "";
  while (!feof($fhandle)) $fdata .= fread($fhandle, 1024);
  fclose($fhandle);

  $fbuf1 = preg_split('/\x0/', $fdata, -1, PREG_SPLIT_NO_EMPTY);
  $fbuf2 = stripansi($fbuf1[0]);
  $fbuf1 = preg_split('/\n/', $fbuf2, 5, PREG_SPLIT_NO_EMPTY);
  $fbody = "";
  if(preg_match('/寄信人: (.*)/', $fbuf1[0], $farray)){
    $fwho = $farray[1];
  }
  else {
    $fwho = "";
    $fbody = $fbuf1[0];
  }
  if(preg_match('/标(\s)*题: (.*)/', $fbuf1[1], $farray)){
    $ftitle = $farray[2];
  }
  else {
    $ftitle = "";
    $fbody = $fbody.$fbuf1[1];
  }

  if(preg_match('/发信站: (.*)\((.*)\)(.*)/', $fbuf1[2], $farray)){
    $ftime = $farray[2];
  }
  else {
    $fsite = "";
    $fbody = $fbody.$fbuf1[2];
  }
  if(preg_match('/来(\s)*源: (.*)/', $fbuf1[3], $farray)){
    $fwhere = $farray[2];
  }
  else {
    $where = "";
    $fbody = $fbody.$fbuf1[3];
  }

  if(strncmp($ftitle, "Re: ", 4) != 0)
    $ftitle = "★".$ftitle;
  $ftime = preg_replace("/[\t\ ]+/", '.', $ftime);
  $fbody = $fwho."[".$ftime."][".$fwhere."]\n".$ftitle."\n".$fbody."\n".$fbuf1[4];
  $startnum = $page * $pagesize;
  $endnum = min($startnum+$pagesize,strlen($fbody));
  if($startnum < strlen($fbody)){
    $fbuf1 = ch_substr($fbody, $startnum, $pagesize);
    $fbuf2 = preg_replace("/[\t\ ]*/m", "", $fbuf1);
    $fbuf = preg_replace("/(\n)+/s","\n",$fbuf2);
    echo preg_replace("/\n/","<br/>",htmlspecialchars($fbuf));
    echo "<br/>";
    if($page > 0)
      showlink(urlstr("readmail",array(),encarg("readmail",array("mbox"=>$mbox, "mailid"=>$mailid, "page"=>($page -1)))),"上一页");
    if($endnum < strlen($fbody)) {
      showlink(urlstr("readmail",array(),encarg("readmail",array("mbox"=>$mbox, "mailid"=>$mailid, "page"=>($page+1)))),"下一页");
    }
    else {
      showlink(urlstr("writemail",array("to"=>$target)),"回复");
    }
    echo "<br/>";
  }
  else echo "错误的页号<br/>";
}
?>