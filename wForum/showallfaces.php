<?php
$needlogin = 0;
require("inc/funcs.php");
require("inc/userdatadefine.inc.php");
html_init();
?>
<body>
<script src="inc/browser.js"  language="javascript"></script>
<script language="JavaScript">
	function setimg(i) {
		if (self.opener == null) return;
		getRawObjectFrom("face", self.opener).selectedIndex = i - 1;
		o = getRawObjectFrom("imgmyface", self.opener);
		o.src = 'userface/image' + i + '.gif';
		o.width = 32;
		o.height = 32;

		o = getRawObjectFrom('myface', self.opener);
		if (o != null) o.value = '';
		window.close();
	}
</script>
<table cellpadding=3 cellspacing=1 align=center class=TableBorder1>
<?php
$str = "";
for ($i=1;$i<=USERFACE_IMG_NUMS;$i++) {
	if ($i % 10 == 1) $str .= "<tr>";
	$str .= "<td width=10% align=center class=TableBody1><a href=\"javascript:setimg($i);\"><img border=\"0\" src=\"userface/image$i.gif\"></a></td>";
	if ($i % 10 == 0) $str .= "</tr>";
}
for ( ; $i % 10 == 0; $i++) {
	$str .= "<td width=10% align=center class=TableBody1>&nbsp;</td>";
	if ($i % 10 == 0) $str .= "</tr>";
}
echo $str;
?>
<TR align=center> 
<TD height=24 colspan="10" class=TableBody2><a href=# onclick="window.close();">¡º ¹Ø±Õ´°¿Ú ¡»</a></TD>
</TR>
</TABLE>
<?php
show_footer(false);
?>
