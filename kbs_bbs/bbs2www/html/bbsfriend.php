<?php
	/**
	 * This file list all friends online.
	 * by binxun
	 * $Id
	 */
	 
	require("funcs.php");
	
	if ($loginok != 1)
		html_nologin();
	else
	{
		html_init("gb2312");
	 	$friends = bbs_getonlinefriends();
		if ($friends == 0)
			$num = 0;
		else
			$num = count($friends);
?>
<center><?php echo BBS_FULL_NAME; ?> -- 在线好友列表 [使用者: <?php echo $currentuser["userid"];?>]<hr>
<?php 
 	if ($num == 0) echo "目前没有好友在线";
	else
	{
?>
<table border="1" width="610">
<tr><td>序号<td>友<td>使用者代号<td>使用者昵称<td>来自<td>动态<td>发呆
<?php
		for($i = 0; $i < $num; $i++)
		{
			echo "<tr><td>" . ($i + 1);
			echo "<td>√";
			if($friends[$i]["invisible"]) echo "<font color=\"green\">C</font>";
			else
				echo " ";
?>
<td><a href="/bbsqry.php?userid=<?php echo $friends[$i]["userid"];?>"><?php echo $friends[$i]["userid"];?></a>
<td><a href="/bbsqry.php?userid=<?php echo $friends[$i]["userid"];?>"><?php echo htmlspecialchars($friends[$i]["username"]);?></a>
<td><?php echo $friends[$i]["userfrom"];?>
<?php
			if($friends[$i]["invisible"]) echo "<td>隐身中...";
			else
				echo "<td>" . $friends[$i]["mode"];
			if($friends[$i]["idle"] == 0) echo "<td> ";
			else
				echo "<td>" . $friends[$i]["idle"];
			echo "</table>";
		}
	}
?>
<hr>
[<a href="/bbsfall.php">全部好友名册</a>]
</center>
<?php
	html_normal_quit();
	}
?>
