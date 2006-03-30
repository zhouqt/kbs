<?php

	require("www2-funcs.php");
	login_init();
	assert_login();
	
	$filename = "tmp/bm.".$currentuser["userid"];
	if(isset($_GET["act"]))
	{
		$act = $_GET["act"];
		if($act == "clear")
		{
			@unlink($filename);
			@unlink($filename.".attach");
?>
<script type="text/javascript">
alert("暂存档已清空。");
window.close();
</script>
<?php
			exit;
		}
	}
	page_header("查看暂存档");
	$s = bbs2_readfile($filename);
	if(is_string($s))
	{
?>
<br><div class="article">
<script type="text/javascript"><!--
<?php
		print($s);
?>
//-->
</script></div>
<br>[<a href="bbsimport.php?act=clear">清空</a>]<br><br>
<?php
	}
	else
	{
?>
<div align="center"><br>暂存档中没有内容。<br><br></div>
<?php
	}
	page_footer();

?>