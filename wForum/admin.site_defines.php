<?php

require("inc/funcs.php");
require("inc/user.inc.php");
require("inc/site_defines.php");

setStat("站点配置参数显示");

preprocess();

show_nav();

showUserMailBoxOrBR();
head_var("站点配置", '');
main();
show_footer();

function preprocess() {
	global $userid;
	if (isset($userid) && ($userid == "SYSOP")) return; //以后在改吧
	foundErr("不许看 :)");
}

function main() {
	global $site_defines;
?>
<table cellpadding=3 cellspacing=1 border=0 align=center class=TableBorder1>
<tr> 
	  <th colspan="3">站点配置参数显示</th>
</tr>
<tr> 
	  <th>参数说明</th><th>当前值</th><th>默认值</th>
</tr>
<?php
	$ccc = count($site_defines);
	for ($i = 0; $i < $ccc; $i++) {
		if ($site_defines[$i] === false) return;
		if (is_string($site_defines[$i])) {
?>
<tr>
	  <td colspan="3" class=TableBody2 align="center"><b><?php echo htmlspecialchars($site_defines[$i]); ?></b></td>
</tr>
<?php
		} else {
?>
<tr>
	<td class=TableBody1>
		<b><?php echo htmlspecialchars($site_defines[$i][4]); ?></b>
	</td>
	<td class=TableBody1>
<?php
			$varname = $site_defines[$i][1];
			if ($site_defines[$i][0]) { //变量
				$var = $GLOBALS[$varname];
			} else { //常量
				$var = constant($varname);
			}
			if ($site_defines[$i][2] == "b") echo ($var?"是":"否");
			else echo htmlspecialchars($var);
?>	
	</td>
	<td class=TableBody1>
<?php
	$var = $site_defines[$i][3];
	if ($site_defines[$i][2] == "b") echo ($var?"是":"否");
	else echo htmlspecialchars($var);
?>
	</td>
</tr>
<?php
		}
	}
?>
</table>
<?php
}
?>
