<?php

require("inc/funcs.php");
setStat("切换隐身状态");

requireLoginok("guest不能使用隐身");

do_changeCloak();

if (!isset($_SERVER["HTTP_REFERER"]) || ( $_SERVER["HTTP_REFERER"]=="") )
{
	header("Location: index.php");
}   else  {
	header("Location: ".$_SERVER["HTTP_REFERER"]);
} 

function do_changeCloak()
{
	global $currentuser;
	global $currentuinfo;

	if (!($currentuser["userlevel"] & BBS_PERM_CLOAK)) {
		foundErr("您没有隐身权限");
	} else {
		bbs_update_uinfo("invisible", !$currentuinfo["invisible"]);
	}
} 
?>