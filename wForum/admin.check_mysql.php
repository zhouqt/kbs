<?php

require("inc/funcs.php");
require("inc/user.inc.php");

setStat("数据库配置检查");

requireLoginok("本页面必须要管理员登录才能使用。");

preprocess();

show_nav();

showUserMailBox();
head_var("站点配置", 'admin.site_defines.php');

check_mysql();

show_footer();

function preprocess() {
	global $currentuser;
	if (!($currentuser["userlevel"] & BBS_PERM_SYSOP)) {
		foundErr("本页面必须要管理员登录才能使用。");
	}
}

function check_mysql() {
    global $dbhost, $dbuser, $dbpasswd, $dbname;
?>
<table cellpadding=3 cellspacing=1 align=center class=TableBorder1 width="97%"><tr><th>数据库配置检查</td></tr>
<?php
    if (!DB_ENABLED) {
?>
<tr><td class="TableBody1">数据库支持被禁用。请在 site.php 中定义：define("DB_ENABLED", true);</td></tr>
<?php
    } else {
        if (!function_exists('mysql_connect'))  {
?>
<tr><td class="TableBody1">mysql_connect PHP 函数没有定义，PHP 没有加入 mysql 支持。请重新 configure PHP 编译安装。如果您的系统是 redhat 9，请安装 php-mysql rpm。</td></tr>
<?php
		} else {
    		@$pig = @mysql_connect($dbhost, $dbuser, $dbpasswd) or $pig = true;
    		if ($pig === true) {
?>
<tr><td class="TableBody1">mysql 连接失败，请检查 site.php 配置：$dbhost, $dbuser, $dbpasswd。</td></tr>
<?php
    		} else {
    		    @mysql_select_db($dbname) or $pig = true;
    		    if ($pig === true) {
?>
<tr><td class="TableBody1">数据库表选取失败。请检查 site.php 配置：$dbname。</td></tr>
<?php
    		    } else {
?>
<tr><td class="TableBody1">数据库配置没有问题！</td></tr>
<?php
    		    }
    		}
    	}
    }
?>
</table>
<?php
}
?>
