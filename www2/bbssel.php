<?php
require('www2-funcs.php');
login_init();
bbs_session_modify_user_mode(BBS_MODE_SELECT);
if (isset($_GET['board']))
	$keyword = trim(ltrim($_GET['board']));
elseif (isset($_POST['board']))
	$keyword = trim(ltrim($_POST['board']));
else
	$keyword = "";

$exact = (!isset($_GET['exact']) && !isset($_POST['exact']))?0:1;

$boards = array();
if ($keyword) {
	if (bbs_searchboard(trim($keyword),$exact,$boards)) {
		if (sizeof($boards)==1) {
			cache_header("nocache");
			header("Location: bbsdoc.php?board=" . urlencode($boards[0]['NAME']));
			exit;
		} else if (sizeof($boards)==0) {
			$boards = false;
		}
	} else {
		$boards = false;
	}
}

page_header("搜索讨论区");
if ($boards) {
?>
<script type="text/javascript"><!--
var ta = new tabWriter(1,'main wide','系统共为您找到 <?php echo sizeof($boards); ?> 个符合条件的讨论区',
[['序号',0,'center'],['讨论区',0,'center'],['说明',0,'center'],['关键字',0,0]]);
<?php
	foreach ($boards as $board) {
		if (!$board['NAME']) continue;
		$col1 = '<a href="bbsdoc.php?board='.urlencode($board['NAME']).'">'.htmlspecialchars($board['NAME'],ENT_QUOTES).'</a>';
		$col2 = htmlspecialchars($board['TITLE'],ENT_QUOTES);
		$col3 = htmlspecialchars($board['DESC'],ENT_QUOTES).'&nbsp;';
		echo "ta.r('$col1','$col2','$col3');\n";
	}
?>
ta.t();
//-->
</script>
<?php
} else {
?>
<div class="warning">对不起，未找到符合条件的讨论区！</div>
<?php    
}  
?>
<form action="<?php echo $_SERVER['PHP_SELF']; ?>" method="get" class="medium">
	<fieldset><legend>搜索讨论区</legend>
		<div class="inputs">
			<label>关键字:</label><input type="text" name="board" value="<?php echo htmlspecialchars($keyword); ?>"/>
			<input type="checkbox" name="exact" id="exact" /><label for="exact" class="clickable">精确匹配</label>
		</div>
	</fieldset>
	<div class="oper"><input type="submit" value="开始搜" /></div>
</form>
<?php     
page_footer();
?>
