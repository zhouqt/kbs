<?php
	require("www2-funcs.php");
	login_init();
	bbs_session_modify_user_mode(BBS_MODE_FRIEND);
	assert_login();
	page_header("在线好友列表");

	$friends = bbs_getonlinefriends();
	if ($friends == 0)
		$num = 0;
	else
		$num = count($friends);

?>
<table class="main adj">
<col class="center"/><col/><col/><col/><col/><col/><col class="right"/>
<tbody>
<tr><th>序号</th><th>友</th><th>使用者代号</th><th>使用者昵称</th><th>来自</th><th>动态</th><th>发呆</th></tr>
<?php
	for($i = 0; $i < $num; $i++)
	{
		echo "<tr><td>" . ($i + 1) . "</td>";
		echo "<td>√";
		if($friends[$i]["invisible"]) echo "C";
		else
			echo " ";
?>
</td>
<td><a href="bbsqry.php?userid=<?php echo $friends[$i]["userid"];?>"><?php echo $friends[$i]["userid"];?></a></td>
<td><a href="bbsqry.php?userid=<?php echo $friends[$i]["userid"];?>"><?php echo "<script type=\"text/javascript\"><!--\nprints('" . str_replace("\033", "\\r", $friends[$i]["username"]) . "');\n--></script>";?></a></td>
<td><?php echo $friends[$i]["userfrom"];?></td>
<?php
		if($friends[$i]["invisible"]) echo "<td>隐身中...</td>";
		else {
			$mode = $friends[$i]["mode"];
			if ($friends[$i]["pid"] == 1) $mode = "<span class='blue'>" . $mode . "</span>";
			echo "<td>" . $mode . "</td>";
		}
		if($friends[$i]["idle"] == 0) echo "<td> </td>";
		else
			echo "<td>" . $friends[$i]["idle"] . "</td>";
	}
?>
</tbody></table>
<div class="oper">
[<a href="bbsfall.php">全部好友名册</a>]
</div>
<?php
	page_footer();
?>
