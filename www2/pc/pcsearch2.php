<?php
	require("pcfuncs.php");
	if(pc_update_cache_header(60))
		return;
	
	pc_html_init("gb2312","Blog搜索");
?>
<center><br><br><br>
<form action="pcsearch.php" method="get" onsubmit="if(this.keyword.value==''){alert('请输入关键字');return false;}">
<p align="center" class="f2">Blog搜索:<p>
<p align="center" class="f1">
<input type="text" name="keyword" size="60" class="b2">
<br /><br />(进行模糊搜索时,请用空格隔开多个关键字)
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
<p align="center">
<?php
    /**
     *    水木的web代码bbslib和cgi部分是修改于NJUWWWBBS-0.9，此部分
     * 代码遵循原有的nju www bbs的版权声明（GPL）。php部分的代码（
     * phplib以及php页面）不再遵循GPL，正在考虑使用其他开放源码的版
     * 权声明（BSD或者MPL之类）。
     *
     *   希望使用水木代码的Web站点加上powered by kbs的图标.该图标
     * 位于html/images/poweredby.gif目录,链接指向http://dev.kcn.cn
     * 使用水木代码的站点可以通过dev.kcn.cn获得代码的最新信息.
     *
     */
    powered_by_smth();
?>
</p>
<?php	
	html_normal_quit();
?>