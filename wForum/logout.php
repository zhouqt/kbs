<?php

$needlogin=0;
require_once("inc/funcs.php");
if  ( ($loginok==1) || ($guestloginok==1) ) {
	bbs_wwwlogoff();
}

$path='';
setcookie("W_UTMPKEY",'',time()+36000000,$path);
setcookie("W_UTMPNUM",'',time()+36000000,$path);
setcookie("W_UTMPUSERID",'',time()+36000000,$path);
setcookie("W_LOGINTIME",'',time()+36000000,$path);
setcookie("W_PASSWORD",'',time()+36000000,$path);

header('Location: '.$SiteURL);
?>
