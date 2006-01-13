<?php

$needlogin=0;
require_once("inc/funcs.php");
if  ( ($loginok==1) || ($guestloginok==1) ) {
	bbs_wwwlogoff();
}
cache_header("nocache");
setcookie(COOKIE_PREFIX."UTMPKEY",'',time()+36000000,COOKIE_PATH,COOKIE_DOMAIN);
setcookie(COOKIE_PREFIX."UTMPNUM",'',time()+36000000,COOKIE_PATH,COOKIE_DOMAIN);
setcookie(COOKIE_PREFIX."UTMPUSERID",'',time()+36000000,COOKIE_PATH,COOKIE_DOMAIN);
setcookie(COOKIE_PREFIX."LOGINTIME",'',time()+36000000,COOKIE_PATH,COOKIE_DOMAIN);
setcookie(COOKIE_PREFIX."PASSWORD",'',time()+36000000,COOKIE_PATH,COOKIE_DOMAIN);

jumpReferer(isset($_GET["jumphome"]));
?>
