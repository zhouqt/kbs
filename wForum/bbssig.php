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
?>
<form name=form1 method="post" action="bbssavesig.php">
<table cellpadding=3 cellspacing=1 class=tableborder1 align=center>
	<tr>
    <th width=100% height=25 colspan=2 align=center>编辑显示签名档 [使用者: <?php echo $currentuser["userid"]; ?>]</th>
    </tr>
	<tr>
          <td width=100% class=tablebody1 align="center"><textarea name="text"  onkeydown='if(event.keyCode==87 && event.ctrlKey) {document.form1.submit(); return false;}'  onkeypress='if(event.keyCode==10) return document.form1.submit()' rows="20" cols="100" wrap="physical">
<?php
	$filename=bbs_sethomefile($currentuser["userid"],"signatures");
    $fp = @fopen ($filename, "r");
    if ($fp!=false) {
		while (!feof ($fp)) {
			$buffer = fgets($fp, 300);
			echo $buffer;
		}
		fclose ($fp);
    }

?>
</textarea></td></tr>
	<tr><td width=100% class=tablebody1 align="center">
	<input type="submit" value="存盘" />&nbsp;&nbsp;&nbsp;&nbsp;<input type="reset" value="复原" />
	</td></tr>
</table>
</form>
<?php
}
?>
