<?php

require("inc/funcs.php");
require("inc/ubbcode.php");

setStat("ЬћзгдЄРР");

html_init();
?>
<body>
<table cellpadding=3 cellspacing=1 align=center class=TableBorder1>
	<TR><Th height=25>ЬћзгдЄРР</Th></TR>
	<TR><TD class=TableBody1 
height=24><b><?php echo htmlspecialchars($_POST["title"]); ?></b></TD></TR>
</TABLE><br>
<table cellpadding=3 cellspacing=1 align=center class=TableBorder1>
<TR><TD 
class=TableBody1><?php echo dvbcode(str_replace("\n", "<br />", htmlspecialchars(str_replace("\r", "", $_POST["body"]))), 0, "TableBody2"); ?></TD></TR>
</TABLE>
<?php
	show_footer(false);
?>
