<?php
	require("funcs.php");
login_init();
	if ($loginok != 1)
		html_nologin();
	else
	{
		html_init("gb2312");
		if( $currentuser["userid"]=="guest" )
				html_error_quit("没有消息");

		$msgs = bbs_getwebmsgs();

		if( $msgs <= 0 )
				html_error_quit("系统错误");

		$i=0;
		cache_header("nocache");
?>
<html>
<table border=1>
<tr><td>序号</td><td>时间</td><td>类型</td><td>对象</td><td>内容</td></tr>
<?php
		foreach( $msgs as $msg ){
			$i++;
?>
<tr><td><?php echo $i;?></td>
<td><?php echo date("D M j H:i:s Y", $msg["TIME"]);?></td>
<td><?php if($msg["SENT"]) echo "<a href=\"bbssendmsg.php?destid=".$msg["ID"]."\">收</a>"; else echo "发";?></td>
<td><?php echo $msg["ID"];?></td>
<td><?php echo htmlspecialchars($msg["content"]);?></td>
</tr>
<?php
		}
?>
</table>
<a onclick="return confirm('你真的要清除所有讯息吗?')" href="/bbsdelmsg.php">清除所有讯息</a> <a href="/bbsmailmsg.php">寄回所有信息</a>
</html>
<?php
	}
?>
