<?php
	require("www2-funcs.php");
	require("www2-board.php");
	login_init();
	bbs_session_modify_user_mode(BBS_MODE_MMENU);
	if (update_cache_header(60))
		exit ();
	page_header("分类讨论区");
?>
<script type="text/javascript"><!--
var ta = new tabWriter(0,'main','分类讨论区',[['区号',0,'center'],['类别',0,0],['描述',0,0]]);
<?php
		for($i = 0; $i < BBS_SECNUM; $i++) {
?>
ta.r('<?php echo $i; ?>','<a href="bbsboa.php?group=<?php echo $i; ?>"><?php echo constant("BBS_SECNAME".$i."_0"); ?></a>','<a href="bbsboa.php?group=<?php echo $i; ?>"><?php echo constant("BBS_SECNAME".$i."_1"); ?></a>');
<?php
		}
?>
ta.t();
//-->
</script>
<?php
	bbs_boards_navigation_bar();
	page_footer();
?>

