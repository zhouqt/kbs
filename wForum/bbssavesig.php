<?php

require("inc/funcs.php");
require("inc/usermanage.inc.php");
require("inc/user.inc.php");

setStat("编辑显示签名档");

requireLoginok();

show_nav(false);

showUserMailbox();
head_var($userid."的控制面板","usermanagemenu.php",0);
main();

show_footer();

function main(){
	global $currentuser;
	$filename=bbs_sethomefile($currentuser["userid"],"signatures");
	$fp=@fopen($filename,"w+");
    if ($fp==false) {
		foundErr("无法存盘，请联系管理员！");
	}
	fwrite($fp,str_replace("\r\n", "\n", $_POST["text"]));
	fclose($fp);
	bbs_recalc_sig();
	setSucMsg("签名档已成功修改！");
	return html_success_quit();
}
?>
