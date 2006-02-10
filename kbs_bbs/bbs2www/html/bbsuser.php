<?php
	require("www2-funcs.php");

	if (defined("SITE_SMTH")) // 不提供在线用户列表 add by windinsn, May 5,2004
		exit ();

	login_init();
	bbs_session_modify_user_mode(BBS_MODE_LUSERS);
	page_header("在线用户列表");
	
	if( isset( $_GET["start"] ) ){
		$start=$_GET["start"];
	} else {
		$start=1;
	}
	if ($start<=0) $start=1;
	$num=20;
	$users = bbs_getonline_user_list($start,$num);
	if ($users == 0)
		$count = 0;
	else
		$count = count($users);

?>
<table class="main adj">
<col class="center"/><col/><col/><col/><col/><col/><col class="right"/>
<tbody>
<tr><th>序号</th><th>友</th><th>使用者代号</th><th>使用者昵称</th><th>来自</th><th>动态</th><th>发呆</th></tr>
<?php
		for($i = 0; $i < $count; $i++) {
			$mode = $users[$i]["mode"];
			if ($users[$i]["pid"] == 1) $mode = "<span class='blue'>" . $mode . "</span>";
			echo "<tr><td>" . ($i+$start) . "</td>";
			echo "<td>" . ($users[$i]["isfriend"]?"√" : "  ") . "</td>";
			echo "<td><a href=\"bbsqry.php?userid=" . $users[$i]["userid"] . "\">" . $users[$i]["userid"] . "</a></td>";
			echo "<td><a href=\"bbsqry.php?userid=" . $users[$i]["userid"] . "\"><script type=\"text/javascript\"><!--\nprints('" . str_replace("\033", "\\r", $users[$i]["username"]) . "');\n--></script></a></td>";
			echo "<td>" . $users[$i]["userfrom"] . "</td>";
			echo "<td>" . $mode . "</td>";
			echo "<td>" . ($users[$i]["idle"]!=0?$users[$i]["idle"]:" ") . "</td></tr>\n";
		}
?>
</tbody></table>
<div class="oper">
[<a href="bbsfriend.php">在线好友</a>]
<?php
	$prev = $start - $num;
	if ($prev <= 0) $prev = 1;
	if( $prev < $start ){
?>
[<a href="bbsuser.php?start=<?php echo $prev;?>">上一页</a>]
<?php
	}
	if( $count >= $num ){
?>
[<a href="bbsuser.php?start=<?php echo $start+$num;?>">下一页</a>]
<?php
	}
?>
</div>
<form method="GET">
<input type="submit" value="跳转到第"> <input type="input" size="4" name="start"> 个使用者
</form><br />
<?php 
	page_footer();
?>
