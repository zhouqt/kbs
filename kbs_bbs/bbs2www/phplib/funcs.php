<?
global $SQUID_ACCL;
//$fromhost=$_SERVER["REMOTE_ADDR"];
$fromhost;
$fullfromhost;
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
?>
