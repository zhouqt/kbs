<?php
require("inc/funcs.php");

require("inc/usermanage.inc.php");

require("inc/user.inc.php");

setStat("编辑显示签名档");

show_nav();

if ($loginok==1) {
?>
<table border="0" width="97%">
<?php
	showUserMailbox();
?>
</table>
<?php
}

head_var($userid."的控制面板","usermanagemenu.php",0);

if ($loginok==1) {
	main();
}else {
	foundErr("本页需要您以正式用户身份登陆之后才能访问！");
}


if (isErrFounded()) {
		html_error_quit();
} 
show_footer();

function main(){
	global $currentuser;
	$filename=bbs_sethomefile($currentuser["userid"],"signatures");
	$fp=@fopen($filename,"w+");
    if ($fp==false) {
		foundErr("无法存盘，请联系管理员！");
		return false;
	}
	fwrite($fp,str_replace("\r\n", "\n", $_POST["text"]));
	fclose($fp);
	setSucMsg("签名档已成功修改！");
	return html_success_quit();
}
?>
