<?php

require("inc/funcs.php");
require("inc/usermanage.inc.php");
require("inc/user.inc.php");

setStat("ÇÐ»»¶©ÔÄ×´Ì¬");

requireLoginok();

show_nav();

showUserMailbox();
head_var($userid."µÄ¿ØÖÆÃæ°å","usermanagemenu.php",0);
main();
showUserManageMenu();
html_success_quit();
show_footer();

function main() {
	global $currentuser;
	$mode=bbs_alter_yank();
?>
<script language="JavaScript">
<!--
    refreshLeft();
//-->
</script>
<?php
	return setSucMsg("×´Ì¬ÒÑÇÐ»»Îª".(($mode==0)?'ÏÔÊ¾È«²¿':'ÏÔÊ¾Ö»ÏÔÊ¾¶©ÔÄ°æ'));
}

?>
