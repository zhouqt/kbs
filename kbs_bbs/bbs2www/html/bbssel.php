<?php
/**
 * search board
 * windinsn.04.05.17
 */
 
require('funcs.php');
require('board.inc.php');
login_init();
html_init('GB2312','','',1);

if (isset($_GET['board']))
    $keyword = trim(ltrim($_GET['board']));
elseif (isset($_POST['text']))
    $keyword = trim(ltrim($_POST['board']));
else
    html_error_quit('请输入关键字');

if (!$keyword)
    html_error_quit('请输入关键字');

$exact = (!isset($_GET['exact']) && !isset($_POST['exact']))?0:1;

$boards = array();

?>
<body topmargin="0">
<table width="100%" border="0" cellspacing="0" cellpadding="3">
  <tr> 
    <td class="b2">
	    <a href="bbssec.php" class="b2"><?php echo BBS_FULL_NAME; ?></a>
	    -
	    搜索讨论区
    </td>
   </tr>
   <tr>
   <td height="30"> </td>
   </tr>
   <tr>
        <td align="center">
<?php

if (bbs_searchboard($keyword,$exact,$boards)) {
    if (sizeof($boards)==1) {
?>
<script language="javascript">
window.location.href="/bbsdoc.php?board=<?php echo urlencode($boards[0]['NAME']); ?>";
</script>
<?php        
    }
    else {
?>
<b>系统共为您找到 <font coor="red"><?php echo sizeof($boards); ?>个</font> 符合条件的讨论区</b><br /><hr>
<table cellspacing="0" cellpadding="5" border="0" width="95%" class="t1">
    <tr>
        <td width="40" class="t2">序号</td>
        <td width="120" class="t2">讨论区</td>
        <td width="180" class="t2">说明</td>
        <td class="t2">关键字</td>
    </tr>
<?php
    $i = 1;
    foreach ($boards as $board) {
        echo '<tr><td class="t3">'.$i.'</td><td class="t4"><a href="/bbsdoc.php?board='.urlencode($board['NAME']).'">'.htmlspecialchars($board['NAME']).'</a></td>'.
             '<td class="t8">'.htmlformat($board['TITLE']).'</td>'.
             '<td class="t7">'.htmlformat($board['DESC']).'&nbsp;</td></tr>';    
        $i ++;
    }
?>
</table>
<?php
    }
}
else {
?><br /><br /><br />
<font color="red"><b>对不起，未找到符合条件的讨论区！</b></font>
<?php    
}  

?>
        </td>
   </tr>
   <tr>
   <td height="30"> </td>
   </tr>
   <tr>
        <td align="center">
        <form action="<?php echo $_SERVER['PHP_SELF']; ?>" method="get">
        ::搜索讨论区::
        <input type="text" name="board" value="<?php echo htmlspecialchars($keyword); ?>" />
        <input type="checkbox" name="exact" />精确查找讨论区名
        <input type="submit" value="开始搜" />
        </form>
        </td>
   </tr>
</table>
<?php     
html_normal_quit();
?>