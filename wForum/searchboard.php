<?php
require("inc/funcs.php");
require("inc/user.inc.php");
require("inc/board.inc.php");

global $keywords;
global $exact;

setStat("版面搜索");

preprocess();

show_nav();

showUserMailBoxOrBR();

head_var();

if ($keywords) {
	doSearch($keywords, $exact);
}
showSearchForm($keywords);

show_footer();

function preprocess(){
	global $keywords;
	global $exact;
	@$keywords = $_REQUEST['board'];
	if (isset($_REQUEST['exact'])) {
		$exact = $_REQUEST['exact'] ? 1 : 0;
	} else {
		$exact = 0;
	}
}

function doSearch($keywords, $exact) {
	$boards = array();
?>
<TABLE cellPadding=3 cellSpacing=1 class=TableBorder1 align=center>
<TR valign=middle>
<Th height=25 width=40>序号</Th>
<Th width=120>讨论区</Th>
<Th width=180>说  明</Th>
<Th width=*>关键字</Th>
</TR>
<?php
	if (bbs_searchboard($keywords,$exact,$boards)) {
		if (sizeof($boards)==1) {
?>
<script language="javascript">
window.location.href="board.php?name=<?php echo urlencode($boards[0]['NAME']); ?>";
</script>
<?php
		} else {
			$i = 1;
	    	foreach ($boards as $board) {
	        	echo '<tr><td height="27" align="center" class="TableBody2">'.$i.'</td>' .
	        	 '<td class="TableBody1">&nbsp;<a href="board.php?name='.urlencode($board['NAME']).'">'.htmlspecialchars($board['NAME']).'</a></td>' .
	             '<td class="TableBody2">&nbsp;'.htmlformat($board['TITLE']).'</td>'.
	             '<td class="TableBody1">&nbsp;'.htmlformat($board['DESC']).'&nbsp;</td></tr>';    
	        	$i ++;
	        }
	    }
	} else {
		echo '<tr><td height="27" align="center" colspan="4" class="TableBody1">没有搜索到任何版面</td></tr>';
	}
?>
</table>
<?php
}

function showSearchForm($keywords) {
?>
<form method="GET" action="searchboard.php">
<table align=center><tr><td>
请输入关键字:
<input type="text" name="board" value="<?php echo htmlspecialchars($keywords); ?>" />
<input type="checkbox" name="exact" id="exact" />精确匹配
<input type="submit" name="submit" value="查询版面">
</td></tr></table>
</form>
<?php
}
?>
