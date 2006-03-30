<?php

require("bbs0anbm_pre.php");

if($has_perm_boards < 1)
	html_error_quit("您没有权限操作该目录。");

$text = "";
if(isset($_POST["title"]))
{
	if(BBS_ANN_AUTONAME == 0)
		$newfname = $_POST["filename"];
	else
		$newfname = "";
	$newtitle = $_POST["title"];
	$newcontent = $_POST["content"];
	$import = 0;
	@$import = intval($_POST["import"]);
	$ret = bbs_ann_mkfile($filename, $newfname, $newtitle, $newcontent, $import);
	switch($ret)
	{
		case 0:
			header("Location: bbs0anbm.php?path=" . rawurlencode($path));
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
			html_error_quit("系统错误，请联系管理员。");
			break;
		case -5:
			html_error_quit("操作失败，可能有其他版主正在处理同一目录。");
			break;
	}
}
else
{	
	$newfname = "";
	$newtitle = "";
	$newcontent = "";
}
	
page_header("新建文件", "精华区操作");

?>
<script type="text/javascript">
function SwitchImport()
{
	if(document.getElementById('import').checked)
		document.getElementById('content').style.visibility = 'hidden';
	else
		document.getElementById('content').style.visibility = '';
}		
</script>
<form action="bbs0anbm_mkfile.php?path=<?php echo rawurlencode($path); ?>" method="post" class="large">
	<fieldset><legend>新建精华区文件</legend>
		<div class="inputs">
			<div style="color:#FF0000"><?php echo $text; ?></div>
<?php
if(BBS_ANN_AUTONAME == 0)
{
?>
			<label>文件名：</label><input type="text" maxlength="38" size="15" name="filename" value="<?php echo htmlspecialchars($newfname); ?>">
			<input type="checkbox" id="import" name="import" value="1" onclick="SwitchImport();">读入暂存档<br>
<?php
}
?>
			<label>标　题：</label><input type="text" maxlength="38" size="38" name="title" value="<?php echo htmlspecialchars($newtitle); ?>"><br>
			<textarea id="content" name="content"><?php echo htmlspecialchars($newcontent); ?></textarea>
		</div>
	</fieldset>
	<div class="oper"><input type="submit" value="保存"> [<a href="bbs0anbm.php?path=<?php echo rawurlencode($path); ?>">返回精华区目录</a>]</div>
</form>
<?php

page_footer();
	
?>