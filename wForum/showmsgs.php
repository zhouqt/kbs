<?php


require("inc/funcs.php");

require("inc/usermanage.inc.php");

require("inc/user.inc.php");

setStat("用户短信服务");

show_nav();

if ($loginok==1) {
?>
<table border="0" width="97%">
<?php
	showUserMailbox();
?>
</table>
<?php
	head_var($userid."的控制面板","usermanagemenu.php",0);
	main();
}else {
	foundErr("本页需要您以正式用户身份登陆之后才能访问！");
}

if (isErrFounded()) {
		html_error_quit();
}

show_footer();

function main() {
	global $currentuser;
	$filename=bbs_sethomefile($currentuser["userid"],"msgfile");
	echo $filename;
    $fp = @fopen ($filename, "r");
    if ($fp!=false) {
        while (!feof ($fp)) {
            $buffer = fgets($fp, 300);
            echo ansi_convert($buffer,"#000000","#F0F0FF");
        }
        fclose ($fp);
    } else {
?>
没有任何讯息
<?php
	}
}
?>