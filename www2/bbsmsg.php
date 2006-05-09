<?php
    require("www2-funcs.php");
    login_init();
    bbs_session_modify_user_mode(BBS_MODE_LOOKMSGS);
	page_header("察看所有讯息");
	assert_login();

	$msgs = bbs_getwebmsgs();

	if( $msgs <= 0 ) html_error_quit("系统错误");

	$i=0;
?>
<script type="text/javascript"><!--
var ta = new tabWriter(1,'main wide fixed',0,[['序号','6%','center'],['时间','20%',0],['类型','6%','center'],['对象','10%',0],['内容',0,0]]);
<?php
	foreach( $msgs as $msg ){
?>
ta.r('<?php echo date("Y/m/j H:i:s", $msg["TIME"]);?>','<?php
if($msg["SENT"]) {
	if($msg["MODE"] == 3)
		echo '收';
	else
		echo '<a href="bbssendmsg.php?destid='.$msg["ID"].'">收</a>'; 
}
else
	echo '发';?>','<?php echo ($msg["MODE"]==3)?"站长广播":$msg["ID"];?>','<?php
echo htmljsformat($msg["content"],true);?> ');
<?php
	}
?>
ta.t();
//-->
</script>
<div class="oper">
<a onclick="return confirm('你真的要清除所有讯息吗?')" href="bbsdelmsg.php">清除所有讯息</a> <a href="bbsmailmsg.php">寄回所有信息</a>
</div>
<?php
	page_footer();
?>
