<?php

require("bbs0anbm_pre.php");

if($has_perm_boards < 1)
	html_error_quit("您没有权限操作该目录。");

$p = strrpos($filename, "/");
$realpath = substr($filename, 0, $p);
$oldfname = substr($filename, $p + 1, strlen($filename) - $p - 1);
$redirectpath = rawurlencode(substr($realpath, 9, strlen($realpath) - 9));
$text = "";
if(isset($_POST["filename"]))
{
	$newfname = $_POST["filename"];
	$newtitle = $_POST["title"];
	$newbm = $_POST["bm"];
	$ret = bbs_ann_editdir($realpath, $oldfname, $newfname, $newtitle, $newbm);
	switch($ret)
	{
		case 0:
			header("Location: bbs0anbm.php?path=" . $redirectpath);
			exit;
		case -1:
			html_error_quit("精华区目录不存在。");
			break;
		case -2:
			$text = "错误：文件名包含非法字符。";
			break;
		case -3:
			$text = "错误：同名目录或文件已经存在。";
			break;
		case -4:
			html_error_quit("系统错误，精华区索引有问题。");
			break;
		case -5:
			html_error_quit("操作失败，可能有其他版主正在处理同一目录。");
			break;
	}
}
else
{	
	if(isset($_GET["title"]))
		$newtitle = $_GET["title"];
	else
		html_error_quit("参数错误。");
	if(isset($_GET["bm"]))
		$newbm = $_GET["bm"];
	else
		html_error_quit("参数错误。");
	$newfname = $oldfname;
	
}
	
page_header("修改目录", "精华区操作");

?>
<form action="bbs0anbm_editdir.php?path=<?php echo rawurlencode($path); ?>" method="post" class="medium">
	<fieldset><legend>修改精华区目录</legend>
		<div class="inputs">
			<div style="color:#FF0000"><?php echo $text; ?></div>
			<label>文件名：</label><input type="text" maxlength="38" size="15" name="filename" value="<?php echo htmlspecialchars($newfname); ?>"><br>
			<label>标　题：</label><input type="text" maxlength="38" size="38" name="title" value="<?php echo htmlspecialchars($newtitle); ?>"><br>
			<label>版　主：</label><input type="text" maxlength="38" size="15" name="bm" value="<?php echo htmlspecialchars($newbm); ?>"><br>
		</div>
	</fieldset>
	<div class="oper"><input type="submit" value="确定修改"> [<a href="bbs0anbm.php?path=<?php echo $redirectpath; ?>">返回精华区目录</a>]</div>
</form>
<?php

page_footer();
	
?>