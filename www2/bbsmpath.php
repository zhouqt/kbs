<?php
	/**
	 *  This file control import_path .
     *  action:  todo: add, del, ...
     *  author:  stiger
	 */
	require("funcs.php");
login_init();
	if ($loginok !=1 )
		html_nologin();
	else
	{
		html_init("gb2312");

		$showform = 0;
		$actionret = 0;

		if(isset($_GET["action"])){
			$action = $_GET["action"];
			if(!strcmp($action,"add")){

				if(isset($_GET["num"]))
					$num = $_GET["num"];
				else
					$showform = 1;

				if(isset($_GET["path"]))
					$path = $_GET["path"];
				else
					$showform = 1;

				if(isset($_GET["title"]))
					$title = $_GET["title"];

				if($showform == 0){

					settype($num,"integer");
					if($num < 0 || $num > 39)
						html_error_quit("参数错误1");

					if(isset($_GET["title"]))
						$actionret = bbs_add_import_path($path,$title,$num);
					else
						$actionret = bbs_add_import_path($path,$num);

					if($actionret > 0) $actionret = 1;
					else $actionret = -1;
				}
			}
			else if(!strcmp($action,"edit")){
				if(isset($_GET["num"]))
					$num = $_GET["num"];
				else
					html_error_quit("参数错误2");

				settype($num,"integer");
				if($num < 0 || $num > 39)
					html_error_quit("参数错误3");

				$showform = 1;
			}
			else if(!strcmp($action,"select")){
				$showform = 2;
			}
		}
		else
			$action = "show";

		$pathret = bbs_get_import_path();
		if( $pathret == FALSE )
			html_error_quit("没有设置思路");
?>
<body>
<center><p><?php echo BBS_FULL_NAME; ?> -- [丝路控制] [用户: <?php echo $currentuser["userid"];?>]</p>
<hr class="default"/>
<?php
		if($showform == 1){
			if(!strcmp($action,"edit")){
				$path = $pathret[$num]["PATH"];
				$title = $pathret[$num]["TITLE"];
			}
?>
<form action="bbsmpath.php" method="get">
路径:<input type="text" size="30" name="path" value="<?php echo $path;?>">
丝路名:<input type="text" size="30" name="title" value="<?php echo $title?>">
序号:<select name="num" class="input"  style="WIDTH: 40px">
<?php
			for($i = 0; $i < 40; $i++){
?>
<option <?php if($i == $num) echo "selected";?> value=<?php echo $i;?>><?php echo $i;?>
<?php
			}
?>
</option></select> 
<input type="hidden" name="action" value="add">
<input type="submit" value="添加">
</form>
<hr class="default"/>
<?php
		}else if($showform == 2){
?>
<script language=javascript>
	function clickclose(){
		opener.document.forms["form1"].elements["num"].value = document.forms["selectImPath"].elements["num"].value ;
		return window.close();
	}
</script>
<form name="selectImPath" method="post" action="">
请选择丝路序号:<select name="num" class="input"  style="WIDTH: 40px">
<?php
			for($i = 0; $i < 40; $i++){
?>
<option <?php if($i == 0) echo "selected";?> value=<?php echo $i;?>><?php echo $i;?>
<?php
			}
?>
</option></select> 
<input type="button" width="60" value="选择" onclick="return clickclose()">
</form>
<hr class="default"/>
<?php
		}else if($actionret != 0){
			if($actionret == 1 ){
?>
序号:<?php echo $num;?> 丝路增加成功
<?php
			}else if($actionret == -1){
?>
序号:<?php echo $num;?> 丝路增加失败
<?php
			}
?>
<hr class="default"/>
<?php
		}
?>
<table width="613">
<tr><td>&nbsp;</td><td>序号</td><td>丝路名</td><td>路径</td></tr>
<?php
		for($i = 0; $i < 40; $i ++){
?>
<tr><td>
<?php
			if( $showform != 2 ){
?>
<a href="bbsmpath.php?action=edit&num=<?php echo $i;?>">修改</a>
<?php
			}
?>
</td><td>
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
<hr class="default"/>
[<a href="javascript:history.go(-1)">返回上一页</a>]
<?php
		html_normal_quit();
	}
?>
