<?php

require("bbs0anbm_pre.php");

if($has_perm_boards < 1)
	html_error_quit("您没有权限操作该目录。");
	
page_header("创建目录", "精华区操作");

?>
<form action="bbs0anbm_mkdir.php?path=<?php echo rawurlencode($path); ?>" method="post" class="medium">
	<fieldset><legend>创建精华区目录</legend>
		<div class="inputs">
			<label>文件名：</label><input type="text" maxlength="38" size="15" name="filename"><br>
			<label>标　题：</label><input type="text" maxlength="38" size="38" name="title"><br>
			<label>版　主：</label><input type="text" maxlength="38" size="15" name="bm"><br>
		</div>
	</fieldset>
	<div class="oper"><input type="submit" value="创建目录"></div>
</form>
<?php

page_footer();
	
?>