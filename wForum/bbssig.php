<?php

require("inc/funcs.php");
require("inc/usermanage.inc.php");
require("inc/user.inc.php");

setStat("编辑显示签名档");

requireLoginok();

show_nav();

showUserMailbox();
head_var($userid."的控制面板","usermanagemenu.php",0);
main();

show_footer();

function main(){
	global $currentuser;
?>
<form name=form1 method="post" action="bbssavesig.php">
<table cellpadding=3 cellspacing=1 class=TableBorder1 align=center>
	<tr>
    <th width=100% height=25 colspan=2 align=center>编辑显示签名档 [使用者: <?php echo $currentuser["userid"]; ?>]</th>
    </tr>
	<tr>
          <td width=100% class=TableBody1 align="center"><textarea name="text"  onkeydown='if(event.keyCode==87 && event.ctrlKey) {document.form1.submit(); return false;}'  onkeypress='if(event.keyCode==10) return document.form1.submit()' rows="20" cols="100" wrap="physical">
<?php
	$filename=bbs_sethomefile($currentuser["userid"],"signatures");
    $fp = @fopen ($filename, "r");
    if ($fp!=false) { //ToDo: 用单一函数来做
		while (!feof ($fp)) {
			$buffer = fgets($fp, 300);
			echo htmlspecialchars($buffer);
		}
		fclose ($fp);
    }

?>
</textarea></td></tr>
	<tr><td width=100% class=TableBody1 align="center">
	<input type="submit" value="存盘" />&nbsp;&nbsp;&nbsp;&nbsp;<input type="reset" value="复原" />
	</td></tr>
</table>
</form>
<?php
}
?>
