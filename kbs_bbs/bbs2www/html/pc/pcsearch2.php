<?php
	$needlogin=0;
	require("pcfuncs.php");
	
	pc_html_init("gb2312","Blog搜索");
?>
<center><br><br><br>
<form action="pcsearch.php" method="get" onsubmit="if(this.keyword.value==''){alert('请输入关键字');return false;}">
<p align="center" class="f2">Blog搜索:<p>
<p align="center" class="f1">
<input type="text" name="keyword" size="20" class="b2">
(进行模糊搜索时,请用空格隔开多个关键字)
</p><p align="center" class="f1">
方式:
<input type="radio" name="exact" value="1" class="b2" checked>精确
<input type="radio" name="exact" value="0" class="b2">模糊
</p><p align="center" class="f1">
类型:
<input type="radio" name="key" value="u" class="b2" checked>用户名
<input type="radio" name="key" value="c" class="b2">Blog名
<input type="radio" name="key" value="t" class="b2">主题
<input type="radio" name="key" value="d" class="b2">Blog描述
</p><p align="center" class="f1">
<input type="submit" value="开始搜" class="b1">
</p>
</form>
</center>
<p align="center">
<?php pc_main_navigation_bar(); ?>
</p>
<?php	
	html_normal_quit();
?>