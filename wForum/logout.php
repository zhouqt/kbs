<?php

$needlogin=0;
require_once("inc/funcs.php");
if  ( ($loginok==1) || ($guestloginok==1) ) {
	bbs_wwwlogoff();
}

$path='';
setcookie("UTMPKEY",'',time()+36000000,$path);
setcookie("UTMPNUM",'',time()+36000000,$path);
setcookie("UTMPUSERID",'',time()+36000000,$path);
setcookie("LOGINTIME",'',time()+36000000,$path);
setcookie("PASSWORD",'',time()+36000000,$path);

header('Location: '.$SiteURL);
?>
