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
			if(!strcmp($action,"add")){

				if(isset($_GET["num"])){
					$num = $_GET["num"];
				}else{
					html_error_quit("参数错误");
				}
				settype($num,"integer");
				if($num < 0 || $num > 39)
					html_error_quit("参数错误1");

				if(isset($_GET["path"])){
					$path = $_GET["path"];
				}else{
					html_error_quit("参数错误2");
				}

				if(isset($_GET["title"])){
					$title = $_GET["title"];
					$actionret = bbs_add_import_path($path,$title,$num);
				}else
					$actionret = bbs_add_import_path($path,$num);

				if($actionret > 0) $actionret = 1;
				else $actionret = -1;

			}
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
				if($actionret == 1 )
					echo $path."丝路增加设置成功<p>";
				else if($actionret == -1)
					echo $path."丝路增加设置失败<p>";
		html_normal_quit();
	}
?>
