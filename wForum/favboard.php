<?php
require("inc/funcs.php");
require("inc/usermanage.inc.php");
require("inc/user.inc.php");

setStat("用户收藏版面");

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

if (isErrFounded())	{
	html_error_quit();
}

show_footer();

function main()	{
	global $currentuser;
	global $_GET; // 我记得 $_GET 是 super-global 变量应该不用说明，不过好像 wForum 里头都这么写那我也这么写了。- atppp
	
	if (isset($_GET["select"]))
		$select	= $_GET["select"];
	else
		$select	= 0;

	if ($select	< 0) {
		foundErr("错误的参数");
		return false;
	}
	if (bbs_load_favboard($select)==-1) {
		foundErr("错误的参数");
		return false;
	}
	showSecs($select, 0, true, 1); //第三个参数是 isFold，暂时设定为永远展开。如果要改那 showSecs() 函数也要改。- atppp
}
?>
