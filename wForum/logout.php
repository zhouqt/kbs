<?php

$needlogin=0;
require_once("inc/funcs.php");
if  ( ($loginok==1) || ($guestloginok==1) ) {
	bbs_wwwlogoff();
}

setcookie(COOKIE_PREFIX."UTMPKEY",'',time()+36000000,COOKIE_PATH);
setcookie(COOKIE_PREFIX."UTMPNUM",'',time()+36000000,COOKIE_PATH);
setcookie(COOKIE_PREFIX."UTMPUSERID",'',time()+36000000,COOKIE_PATH);
setcookie(COOKIE_PREFIX."LOGINTIME",'',time()+36000000,COOKIE_PATH);
setcookie(COOKIE_PREFIX."PASSWORD",'',time()+36000000,COOKIE_PATH);

if (isset($_GET["jumphome"])) {
	header("Location: index.php");
} else {
	jumpReferer();
}
?>
