<?php
require("inc/funcs.php");
require("inc/ubbcode.php");

setStat("ЬћзгдЄРР");

$is_tex = SUPPORT_TEX ? intval($_POST["texflag"]) : 0;
html_init("","","",$is_tex);
?>
<script src="inc/funcs.js"  language="javascript"></script>
<body>
<table cellpadding="3" cellspacing="1" align="center" class="TableBorder1">
	<tr><th height="25">ЬћзгдЄРР</th></tr>
	<tr><td class="TableBody1" 
height="24"><b><?php echo htmlspecialchars($_POST["title"]); ?></b></td></tr>
</table><br/>
<table cellpadding="3" cellspacing="1" align="center" class="TableBorder1" style="table-layout:fixed;word-break:break-all">
<tr><td 
class="TableBody1"><?php
    echo DvbTexCode(bbs_printansifile($_POST["body"],1,'bbsuploadcon.php?s',$is_tex,1), 0, "TableBody2", $is_tex); ?></td></tr>
</table>
<?php
	show_footer(false);
?>
