<?php

$needlogin = 0;
require("inc/funcs.php");
require("inc/ubbcode.php");

setStat("ЬћзгдЄРР");

html_init();
?>
<script src="inc/funcs.js"  language="javascript"></script>
<body>
<table cellpadding="3" cellspacing="1" align="center" class="TableBorder1">
	<tr><th height="25">ЬћзгдЄРР</th></tr>
	<tr><td class="TableBody1" 
height="24"><b><?php echo htmlspecialchars($_POST["title"]); ?></b></td></tr>
</table><br/>
<table cellpadding="3" cellspacing="1" align="center" class="TableBorder1">
<tr><td 
class="TableBody1"><?php echo dvbcode(bbs_printansifile($_POST["body"],1,'bbsuploadcon.php?s',0,1), 0, "TableBody2"); ?></td></tr>
</table>
<?php
	show_footer(false);
?>
