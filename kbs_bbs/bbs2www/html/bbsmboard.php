<?php
	/**
	 * This file manager boards.
	 * $Id$
	 */
	require("funcs.php");
	if ($loginok != 1)
		html_nologin();
	else
	{
		html_init("gb2312");

		/* check perm */
		if(0){
			html_error_quit("您没有权限");
		}

		if(isset($_GET["submit"])){

			if(isset($_GET["bname"])){
				$bname = $_GET["bname"];
			}else
				html_error_quit("版名错误");

			if(isset($_GET["btitle"])){
				$btitle = $_GET["btitle"];
			}else
				html_error_quit("中文版名错误");

			if(isset($_GET["bbm"])){
				$bbm = $_GET["bbm"];
			}else
				$bbm = "";

			if(isset($_GET["blevel"])){
				$blevel = $_GET["blevel"];
				settype($blevel, "integer");
			}else
				$blevel = 0;
			settype($blevel, "integer");

			if(isset($_GET["banony"])){
				$banony = 1;
			}else
				$banony = 0;

			if(isset($_GET["bjunk"])){
				$bjunk = 1;
			}else
				$bjunk = 0;

			if(isset($_GET["bout"])){
				$bout = 1;
			}else
				$bout = 0;

			if(isset($_GET["bgroup"])){
				$bgroup = $_GET["bgroup"];
			}else
				$bgroup = 0;

			$ret = bbs_new_board($bname,$btitle,$bbm,$blevel,$banony,$bjunk,$bout,$bgroup);

			if($ret < 0)
				html_error_quit("加入讨论区失败".$ret);
			else
				echo "新讨论区成立";

		}	//submit
		else{
?>

<form name="form0" action="<?php echo $_SERVER["PHP_SELF"]; ?>" method="get">
版面英文名称:<input type="text" name="bname" value=""><br>
版面中文名称:<input type="text" name="btitle" value=""><br>
版面管理者:<input type="text" name="bbm" value=""><br>
版面权限:<input type="text" name="blevel" value=""><br>
版面精华区位置:<input type="text" name="bgroup" value=""><br>
<input type="checkbox" name="banony">匿名版面<br>
<input type="checkbox" name="bjunk">版面不计文章数<br>
<input type="checkbox" name="bout">转信版面<br>
<input type="submit" name="submit" value="确定">
</form>
<?php
		}
		html_normal_quit();
	}
?>
