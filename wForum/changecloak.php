<?php

require("inc/funcs.php");
setStat("切换隐身状态");

requireLoginok("guest不能使用隐身");

do_changeCloak();

jumpReferer();

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