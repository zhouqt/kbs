<?php
require("wapfuncs.php");
$havetoget = 0;

//***********************************

@$id = $_GET["u"];
@$passwd = $_GET["p"];
@$kkm = $_GET["k"];
//setcookie("PWD",$passwd,time()+360000,urlstr('index'));
//setcookie("UID",$id,time()+360000,urlstr('index'));
setcookie("P",$passwd,time()+360000,"/");
setcookie("U",$id,time()+360000,"/");

$data = array ();
$error = -1;
$loginok = 6;

if ($id=="")
{
  $id = "guest";
}

if (($id!="guest")&&bbs_checkpasswd($id,$passwd)!=0)
{
  $loginok=6;
}
else {
  $kick=0;
  if ($kkm != "")
    $kick=1;
  $error=bbs_wwwlogin($kick);
  if (($error!=0)&&($error!=2)) {
    $loginok=6;
    if ($error==-1)
      $loginok=4;
  }
  else {
    $loginok=0;
    $utmpnum = bbs_getcurrentuinfo($data);
    @$utmpkey = $data["utmpkey"];
    @$userid = $data["userid"];
    $sessionid = encodesession($utmpnum,3).encodesession($utmpkey,6);
    setcookie("S", $sessionid, time()+360000, "/");
    if ($userid == 'guest') setcookie("S", '', time()-360, "/");

  }
}

if ($loginok != 0)
{
  if ($loginok != 1)
    {
    if ($loginok == 6)
      {
	authpanic("密码错误");
      }
    else
      /*$loginok = 4*/
      {
	waphead(0);
	echo "<card id=\"main\" title=\"".BBS_WAP_NAME."\">";
	echo "<p>登陆次数太多</p>";
	echo "<p>";
	showlink(urlstr('login',array('u'=>$id,'p'=>$passwd,'k'=>1)),"踢出其他登陆");
	echo "</p>";
      }
    }
}
else 
{
  /*****************Notice************************
     if ( cannot set cookie )
     {
       $havatoget = ($userid == 'guest')?0:1;
     }
  ************************************************/

  waphead(0);
  echo "<card id=\"main\" title=\"".BBS_WAP_NAME."\">";
  echo "<onevent type=\"ontimer\">";
  echo "<go href=\"".urlstr('menu')."\"/>";
  echo "</onevent><timer value=\"1\"/>";
}
?>
</card>
</wml>
