<?php
	require("www2-funcs.php");
	login_init();
	bbs_session_modify_user_mode(BBS_MODE_GMENU);
	toolbox_header("好友名单");
	assert_login();

	if( isset( $_GET["start"] ) ){
		$start = $_GET["start"];
		settype($start, "integer");
	}else
		$start = 0;

	$total = bbs_countfriends($currentuser["userid"]);

	if( $total < 0 ) html_error_quit("系统错误");

	if( $total != 0 ){
		if($start >= $total){
			$start = $total - 20;
			if($start < 0) $start = 0;
		}else if($start < 0){
			$start = $total - 20;
			if($start < 0) $start = 0;
		}

		$friends = bbs_getfriends($currentuser["userid"], $start);
		if ($friends === FALSE){
			html_error_quit("系统错误1");
		}
?>
<table class="main adj">
<col class="center"/><col/><col/><col class="center"/>
<caption>好友名单 (共 <?php echo $total;?> 位好友)</caption>
<tbody>
<tr><th>序号</th><th>好友代号</th><th>好友说明</th><th>删除好友</th></tr>
<?php
		$i=0;
		
		foreach ($friends as $friend){
			$i++;
?>
<tr><td><?php echo $start+$i;?></td>
<td><a href="bbsqry.php?userid=<?php echo $friend["ID"];?>"><?php echo $friend["ID"];?></a></td>
<td><?php echo $friend["EXP"];?></td>
<td>[<a onclick="return confirm('确实删除吗?')" href="bbsfdel.php?userid=<?php echo $friend["ID"];?>">删除</a>]</td>
</tr>
<?php
		}
?>
</tbody></table>
<?php
	}
?>
<div class="oper">
[<a href="bbsfadd.php">添加新的好友</a>]
<?php
		if( $start > 0 ){
?>
[<a href="bbsfall.php?start=0">第一页</a>]
[<a href="bbsfall.php?start=<?php if($start > 20) echo $start - 20; else echo "0";?>">上一页</a>]
<?php
		}
		if( $start < $total - 20 ){
?>
[<a href="bbsfall.php?start=<?php echo $start + 20; ?>">下一页</a>]
[<a href="bbsfall.php?start=-1">最后一页</a>]
<?php
		}
?>
</div>
<?php
		page_footer();
?>
