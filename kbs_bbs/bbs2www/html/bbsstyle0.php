<?php
require_once("funcs.php");
login_init();
require_once("style.inc.php");

$bbsstyle = bbs_style_getstyle();
html_init("GB2312");
?>
<body>
<br /><br /><br />
<center>
<form action="/bbsstyle.php" method="get" />
<table cellspacing="0" cellpadding="5" border="0">
<tr><td><b>请选择界面方案:</b></td></tr>
<tr><td>
<input type="radio" name="style" value="0" <?php if($bbsstyle==0) echo "checked"; ?> />默认方案<br />
<input type="radio" name="style" value="1" <?php if($bbsstyle==1) echo "checked"; ?>/>蓝色经典(小字体)<br />
<input type="radio" name="style" value="2" <?php if($bbsstyle==2) echo "checked"; ?>/>蓝色经典(中字体)<br />
<input type="radio" name="style" value="3" <?php if($bbsstyle==3) echo "checked"; ?>/>蓝色经典(大字体)<br />
</td></tr>
</table>

<input type="submit" value="更改" />
<input type="reset"  value="复原" />
<input type="button" value="返回" onclick="history.go(-1)" />
</form>
</center>
<?php
html_normal_quit();
?>
