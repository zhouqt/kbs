<?php

require("inc/funcs.php");
require("inc/user.inc.php");

setStat("站点配置参数");

requireLoginok("本页面必须要管理员登录才能使用。");

preprocess();

show_nav();

showUserMailBox();
head_var("站点配置", 'admin.site_defines.php');
if (isset($_GET["action"])) {
	$action = $_GET["action"];
} else $action = "";
if ($action == "gen") {
	generate_site();
	show_site();
} else if ($action == "mv") {
	apply_change();
} else {
	show_conf();
}
show_footer();

function preprocess() {
	global $currentuser;
	global $curConf;
	global $bakConf;
	global $newConf;
	if (!($currentuser["userlevel"] & BBS_PERM_SYSOP)) {
		foundErr("本页面必须要管理员登录才能使用。");
	}
	$topdir = dirname($_SERVER['SCRIPT_FILENAME']);
	$curConf = $topdir . "/inc/site.php";
	if (is_link($curConf)) $curConf = realpath($curConf); //readlink() returns relative path
	$bakConf = $topdir . "/inc/sites/site.bak.php";
	$newConf = $topdir . "/inc/sites/site.new.php";
}

function apply_change() {
	global $curConf;
	global $newConf;
	global $bakConf;
	@copy($curConf, $bakConf) or foundErr("备份旧配置文件出错！");
	@copy($newConf, $curConf) or foundErr("复制新配置文件出错！");
	setSucMsg("新配置文件生效。");
	return html_success_quit('返回站点配置', "admin.site_defines.php");
}

function write_var_php($var) {
	if (is_string($var)) {
		return "\"" . addcslashes($var, "\\\"") . "\""; //注意这个地方不能用 addslashes()，因为单引号不可以被 escape
	} else {
		return $var;
	}
}

function write_array_php($varname) { //假设最多 array 套一层 array。本来可以用 var_export() 不过输出格式不太好看
	$var = $GLOBALS[$varname];
	$stat = "\$" . $varname . " = array(";
	$ccc = count($var);
	for ($i = 0; $i < $ccc; $i++) {
		if (is_array($var[$i])) {
			$stat .= "\n\tarray(";
			$cc = count($var[$i]);
			for ($j = 0; $j < $cc; $j++) {
				$stat .= write_var_php($var[$i][$j]);
				if ($j != $cc - 1) $stat .= ", ";
			}
			$stat .= ")";
			if ($i != $ccc - 1) $stat .= ",";
			else $stat .= "\n";
		} else {
			$stat .= write_var_php($var[$i]);
			if ($i != $ccc - 1) $stat .= ", ";
		}
	}
	$stat .= ");\n";
	return $stat;
}

function generate_site() {
	global $site_defines;
	global $newConf;
	
	$stat = "<?php\n/* automatically generated site configuration file */\n";
	$stat .= "\n/* 分类讨论区 */\n";
	$stat .= write_array_php("section_nums");
	$stat .= write_array_php("section_names");
	
	$stat .= "\n/* 特殊的个人自定义参数 */\n";
	if (isset($GLOBALS["user_define_default"])) $stat .= write_array_php("user_define");
	if (isset($GLOBALS["user_define1_default"])) $stat .= write_array_php("user_define1");
	if (isset($GLOBALS["mailbox_prop_default"])) $stat .= write_array_php("mailbox_prop");
	
	$ccc = count($site_defines);
	for ($i = 0; $i < $ccc; $i++) {
		if ($site_defines[$i] === false) break;
		if (is_string($site_defines[$i])) {
			$stat .= "\n/* " . $site_defines[$i] . " */\n";
			continue;
		}
		$is_boolean = ($site_defines[$i][2] == "b");
		$is_string =  ($site_defines[$i][2] == "s");
		$is_integer =  ($site_defines[$i][2] == "i");
		$is_constant = ($site_defines[$i][0] == 0);
		$varname = $site_defines[$i][1];
		$default = $site_defines[$i][3];
		if (!isset($_POST["c$i"])) foundErr("参数不全，请重新配置。");
		$cur = $_POST["c$i"];
		if ($is_boolean) {
			$cur = (($cur==1) ? true : false);
		} else if ($is_integer) {
			if (!is_numeric($cur)) foundErr("数字参数指定了非数字的值");
			$cur = intval($cur);
		}
		if ($default == $cur) continue;
		if ($is_constant) {
			$stat .= "define(\"" . $varname . "\", ";
		} else {
			$stat .= "\$" . $varname . " = ";
		}
		if ($is_string) {
			$stat .= "\"" . addcslashes($cur, "\\\"") . "\"";
		} else if ($is_boolean) {
			$stat .= ($cur ? "true" : "false");
		} else {			
			$stat .= $cur;
		}
		if ($is_constant) $stat .= ")";
		$stat .= ";\n";
	}
	$stat .= "\n/* 默认配置 */\nrequire \"default.php\";\n?>\n";
	if (($fp = @fopen($newConf, "w")) !== false) {
		fwrite($fp, $stat);
		fclose($fp);
	}
}

function show_site() {
	global $curConf;
	global $newConf;
	global $bakConf;
?>
<script language="javascript">
<!--
	function con() {
		return confirm("如果新的配置带来问题，您确定您知道怎么恢复到原来的配置吗？");
	}
//-->
</script>
<form action="admin.site_defines.php?action=mv" method="post" onsubmit="return con()" name="site_defines">
<table cellpadding=3 cellspacing=1 border=0 align=center class=TableBorder1>
<tr> 
	<th width="50%">当前配置: <?php echo $curConf; ?></th>
	<th width="50%">新的配置: <?php echo $newConf; ?></th>
</tr>
<tr>
	<td class=TableBody2 style="padding: 10pt;" valign="top">
<?php
	show_source($curConf);
?>
	</td>
	<td class=TableBody2 style="padding: 10pt;" valign="top">
<?php
	if (file_exists($newConf)) show_source($newConf);
	else echo "没找到文件，哭了。";
?>
	</td>
</tr>
<tr> 
	<td colspan="2" class="TableBody1" align="center" style="padding: 5pt;" >
		<b>您现在应该干什么...</b>
	</td>
</tr>
<tr> 
	<td colspan="2" class="TableBody2" align="center" style="padding: 10pt;">
	<table><tr><td align="left">
		<ul>
			<li>看一下上面新的配置文件有什么问题。</li>
			<li>按下面的按钮将会让新的配置生效。旧的配置将会保存在<br><?php echo $bakConf; ?><br>如果之后出现问题，请登录 BBS 主机执行<br>cp -f <?php echo $bakConf ?> <?php echo $curConf ?><br>改回旧的配置。</li>
			<li>如果您不明白我在说什么，那就先搞明白了再点下面的按钮。</li>
		</ul>
	</td></tr></table>
	<input type="submit" value="让新的配置生效！">
	</td>
</tr>
</table>
<?php
}

function show_conf() {
	global $site_defines;
?>
<form action="admin.site_defines.php?action=gen" method="post" name="site_defines">
<table cellpadding=3 cellspacing=1 border=0 align=center class=TableBorder1>
<tr> 
	<th colspan="3">站点配置参数</th>
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
			$varname = $site_defines[$i][1];
			$default = $site_defines[$i][3];
			if ($is_constant) {
				$cur = constant($varname);
			} else {
				$cur = $GLOBALS[$varname];
			}
			$is_default = ($default == $cur);
?>
<tr>
	<td class=TableBody1>
<?php
			if (!$is_default) echo "<b>";
			echo htmlspecialchars($site_defines[$i][4]);
			if (!$is_default) {
				echo "</b>&nbsp;&nbsp;[默认值：";
				if ($is_boolean) {
					echo ($default?"是":"否");
				} else {
					echo htmlspecialchars($default);
				}
				echo "]";
			}
?>
	</td>
	<td class=TableBody1 align="center">
<?php
			if ($is_boolean) {
				echo "<input type=\"radio\" name=\"c$i\" value=\"1\"" . ($cur? " checked=\"checked\"" : "") . " />是&nbsp;&nbsp;&nbsp;&nbsp;";
				echo "<input type=\"radio\" name=\"c$i\" value=\"0\"" . ($cur? "" : " checked=\"checked\"") . " />否";
			} else {
				echo "<input class=\"TableBorder2\" type=\"text\" name=\"c$i\" value=\"" . htmlspecialchars($cur) . "\" size=\"50\" />";
			}
?>	
	</td>
	<td class=TableBody1>
<?php
			echo $is_constant?"常量":"<b>变量</b>";
			echo " 名称：".htmlspecialchars($varname);
?>	
	</td>
</tr>
<?php
		}
	}
?>
<tr> 
	<td colspan="3" class="TableBody2" align="center"><input type="submit" value="自动生成 site.php"></td>
</tr>
</table>
</form>
<?php
}
?>
