<?php

require("inc/funcs.php");
setStat("切换隐身状态");

do_changeCloak();

if (isErrFounded()) {
		show_nav();
		head_var("切换隐身状态");
		html_error_quit();
		show_footer();
} else {
  if (!isset($_SERVER["HTTP_REFERER"]) || ( $_SERVER["HTTP_REFERER"]=="") )
  {
	  header("Location: ".$SiteURL);
  }   else  {
	 header("Location: ".$_SERVER["HTTP_REFERER"]);
  } 
} 


function do_changeCloak()
{
	global $currentuser;
	global $currentuinfo;
	
	if (!strcasecmp($currentuser["userid"],"guest")) {
		foundErr("guest不能使用隐身");
		return;
	}
	if (!($currentuser["userlevel"] & BBS_PERM_CLOAK)) {
		foundErr("您没有隐身权限");
		return;
	}
	bbs_update_uinfo("invisible", !$currentuinfo["invisible"]);
} 
?>