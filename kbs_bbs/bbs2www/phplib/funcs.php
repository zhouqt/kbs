<?
dl("../libexec/bbs/libphpbbslib.so");
global $SQUID_ACCL;
global $BBS_HOME;
//$fromhost=$_SERVER["REMOTE_ADDR"];
global $fromhost;
global $fullfromhost;
global $loginok;
global $currentuinfo;
global $currentuinfo_num;
global $currentuser;
global $currentuuser_num;
$currentuinfo=array ();
$currentuser=array ();
$loginok=0;
header("Cache-Control: no-cache");

if (SQUID_ACCL) {
  $fullfromhost=$_SERVER["HTTP_X_FORWARDED_FOR"];
  if ($fullfromhost=="") {
      $fullfromhost=$_SERVER["REMOTE_ADDR"];
      $fromhost=$fullfromhost;
  }
  else {
	$str = strrchr($fullfromhost, ",");
	if ($str!=FALSE)
		$fromhost=substr($str,1);
        else
		$fromhost=$fullfromhost;
  }
}
else {
  $fromhost=$_SERVER["REMOTE_ADDR"];
  $fullfromhost=$fromhost;
}

bbs_setfromhost($fromhost,$fullfromhost);

$utmpkey = $_COOKIE["UTMPKEY"];
$utmpnum = $_COOKIE["UTMPNUM"];
$userid = $_COOKIE["UTMPUSERID"];
if ($utmpkey!="") {
  if (bbs_setonlineuser($userid,$utmpnum,$utmpkey,$currentuinfo)==0) {
    $loginok=1;
    $currentuinfo_num=bbs_getcurrentuinfo();
    $currentuser_num=bbs_getcurrentuser($currentuser);
  }
}

function valid_filename($fn)
{
	if ((strstr($fn,"..")!=FALSE)||(strstr($fn,"/")))
		return 0;
	return 1;
}

function getboardfilename($boardname,$filename)
{
	return "boards/" . $boardname . "/" . $filename;
}

function error_alert($msg)
{
?>
<SCRIPT language="javascript">
window.alert(<? echo "\"$msg\""; ?>);
history.go(-1);
</SCRIPT>
<?
}

function error_nologin()
{
?>
<SCRIPT language="javascript">
window.location="/nologin.html";
</SCRIPT>
<?
}

if (($loginok!=1)&&($_SERVER["PHP_SELF"]!="/bbslogin.jsp")) {
	error_nologin();
	return;
}
?>
