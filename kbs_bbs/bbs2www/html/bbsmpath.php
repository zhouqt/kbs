<?php
	/**
	 *  This file control import_path .
     *  action:  todo: add, del, ...
     *  author:  stiger
	 */
	require("funcs.php");
	if ($loginok !=1 )
		html_nologin();
	else
	{
		html_init("gb2312");

		if(isset($_GET["action"])){
			$action = $_GET["action"];
			
		}

		$pathret = bbs_get_import_path();
		if( $pathret == FALSE )
			html_error_quit("没有设置思路");
?>
<body>
<center><p><?php echo BBS_FULL_NAME; ?> -- [丝路控制] [用户: <?php echo $currentuser["userid"];?>]</p>
<hr class="default"/>
<table width="613">
<tr><td>序号</td><td>丝路名</td><td>路径</td></tr>
<?php
		for($i = 0; $i < 40; $i ++){
?>
<tr><td>
<?php echo $i;?>
</td><td>
<?php echo $pathret[$i]["TITLE"];?>
</td><td>
<?php if($pathret[$i]["TITLE"][0]) echo $pathret[$i]["PATH"];?>
</td></tr>
<?php
		}
?>
</table>
<?php
		html_normal_quit();
	}
?>
