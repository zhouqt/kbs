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
	global $currentuser;
	if ($currentuser["userlevel"] & BBS_PERM_SYSOP) return;
	foundErr("不许看 :)");
}

function show_value($value, $is_boolean) {
	if ($is_boolean) return ($value?"是":"否");
	else return htmlspecialchars($value);
}

function main() {
	global $site_defines;
?>
<table cellpadding=3 cellspacing=1 border=0 align=center class=TableBorder1>
<tr> 
	  <th colspan="3">站点配置参数显示</th>
</tr>
<?php
	$ccc = count($site_defines);
	for ($i = 0; $i < $ccc; $i++) {
		if ($site_defines[$i] === false) break;
		if (is_string($site_defines[$i])) {
?>
<tr>
	  <td colspan="3" class=TableBody2 align="center"><b><?php echo htmlspecialchars($site_defines[$i]); ?></b></td>
</tr>
<?php
		} else {
			$is_boolean = ($site_defines[$i][2] == "b");
			$is_constant = ($site_defines[$i][0] == 0);
			$cur_var = $site_defines[$i][1];
			$default = $site_defines[$i][3];
			if ($is_constant) {
				$cur = constant($cur_var);
			} else {
				$cur = $GLOBALS[$cur_var];
			}
			$is_default = ($default == $cur);
?>
<tr>
	<td class=TableBody1>
<?php
			if (!$is_default) echo "<b>";
			echo htmlspecialchars($site_defines[$i][4]);
			if (!$is_default) echo "</b>&nbsp;&nbsp;[默认值：".show_value($default, $is_boolean)."]";
?>
	</td>
	<td class=TableBody1>
<?php
			if (!$is_default) echo "<b>";
			echo show_value($cur, $is_boolean);
			if (!$is_default) echo "</b>";
?>	
	</td>
	<td class=TableBody1>
<?php
			echo $is_constant?"常量":"<b>变量</b>";
			echo " 名称：".htmlspecialchars($cur_var);
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
