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

			if(isset($_GET["section"])){
				$section = $_GET["section"];
			}else
				$section = "";

			if(isset($_GET["desp"])){
				$desp = $_GET["desp"];
			}else
				$desp = "";

			if(isset($_GET["blevel"])){
				$blevel = $_GET["blevel"];
				settype($blevel, "integer");
			}else
				$blevel = 0;

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

			if(isset($_GET["battach"])){
				$battach = 1;
			}else
				$battach = 0;

			if(isset($_GET["bgroup"])){
				$bgroup = $_GET["bgroup"];
			}else
				$bgroup = 0;

			$ret = bbs_new_board($bname,$section,$desp,$btitle,$bbm,$blevel,$banony,$bjunk,$bout,$bgroup,$battach);

			if($ret < 0)
				html_error_quit("加入讨论区失败".$ret);
			else
				echo "新讨论区成立";

		}	//submit
		else{
			$explains = array();
			$explain_num = bbs_get_explain( $explains );
?>

<form name="form0" action="<?php echo $_SERVER["PHP_SELF"]; ?>" method="get">
版面英文名称:<input type="text" name="bname" value=""><br>
版面中文名称:<input type="text" name="btitle" value=""><br>
版面分区:<select name="section" class="input" style="WIDTH: 100px">
<?php
			for($i = 0; $section_nums[$i]!="" && $section_names[$i]!=""; $i++){
?>
<option <?php if($i == 0) echo "selected";?>value=<?php echo $section_nums[$i];?>><?php echo $section_names[$i][0];?></option>
<?php
			}
?>
</select><br>
版面分区具体描述(建议4个字符，即2个汉字):<input type="text" name="desp" value=""><br>
版面管理者:<input type="text" name="bbm" value=""><br>
版面权限:<input type="text" name="blevel" value=""><br>
版面精华区位置:<select name="bgroup" class="input" style="WIDTH: 100px">
<?php
			for($i = 0; $i < $explain_num; $i ++){
?>
<option value=<?php echo $explains[$i]["GROUPS"];?>><?php echo $explains[$i]["EXPLAIN"];?></option>
<?php
			}
?>
</select><br>
<input type="checkbox" name="banony">匿名版面<br>
<input type="checkbox" name="bjunk">版面不计文章数<br>
<input type="checkbox" name="bout">转信版面<br>
<input type="checkbox" name="battach">可粘贴附件<br>
<input type="checkbox" name="bclubread">读限制俱乐部<br>
<input type="checkbox" name="bclubwrite">写限制俱乐部<br>
<input type="checkbox" name="bclubhide">隐藏限制俱乐部<br>
<input type="submit" name="submit" value="确定">
</form>
<?php
		}
		html_normal_quit();
	}
?>
