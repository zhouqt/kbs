<?php

$needlogin = 0;
require("inc/funcs.php");

setStat("检测 ID");

html_init();
@$id = urldecode($_GET["id"]);
?>
<body>
<form method="GET" action="checkid.php">
<table cellpadding=3 cellspacing=1 align=center class=TableBorder1>
	<TR><Th height=25>检测 ID<?php if ($id != "") echo ": " . htmlspecialchars($id); ?></Th></TR>
	<TR><TD class=TableBody1 
height=24 align="center">
<?php
	
	if ($id == "") echo "填个 ID 先吧。";
	else {
		$ret = bbs_is_invalid_id($id);
		switch($ret) {
			case 0:
				echo "该 ID 可以注册。";
				break;
			case 1:
				echo "该 ID 使用了禁用字符。";
				break;
			case 2:
				echo "该 ID 太短。";
				break;
			case 3:
				echo "该 ID 使用了禁用字符。"; //不雅 ID
				break;
			case 4:
				echo "该 ID 已存在。";
				break;
			case 5:
				echo "该 ID 太长。";
				break;
		}
	}
?>
	</TD></TR>
	<TR><TD class=TableBody2 
height=24 align="center">
		检测别的 ID: <input type="text" name="id">&nbsp;<input type="submit" name="submit" value="检测 ID">
	</td></tr>
</TABLE></form>
<br>
<?php
	show_footer(false);
?>
