<?php
	/**
	 * This file list online users.
	 * $Id$
	 */
	 
	require("funcs.php");
	
	if ($loginok != 1)
		html_nologin();
	else
	{
		html_init("gb2312");
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

<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=gb2312">
<link rel="stylesheet" type="text/css" href="/bbs-bf.css">
<link rel="stylesheet" type="text/css" href="/ansi-web-middle.css">
</head><center>
<?php echo BBS_FULL_NAME; ?> -- 在线用户列表 [目前在线: <?php echo bbs_getonlinenumber(); ?>人]<hr>
<?php
    if ($count == 0) echo "目前没有用户在线";
    else {
?>
<table border="1" width="610">
<tr><td>序号</td><td>友</td><td>使用者代号</td><td>使用者昵称</td><td>来自</td><td>动态</td><td>发呆</td></tr>
<?php
        for($i = 0; $i < $count; $i++) {
            echo "<tr><td>" . ($i+$start) . "</td>";
            echo "<td>" . ($users[$i]["isfriend"]?"√" : "  ") . "</td>";
            echo "<td><a href=\"bbsqry.php?userid=" . $users[$i]["userid"] . "\">" . $users[$i]["userid"] . "</a></td>";
            echo "<td><a href=\"bbsqry.php?userid=" . $users[$i]["userid"] . "\">" . $users[$i]["username"] . "</a></td>";
            echo "<td>" . $users[$i]["userfrom"] . "</td>";
            echo "<td>" . $users[$i]["username"] . "</td>";
            echo "<td>" . $users[$i]["idletime"]!=0?$users[$i]["idletime"]:" " . "</td></tr>";
        }
?>
</table>
<?php
    } //$count==0   
?>
<hr>
<?php
    for ($i=0;$i<26;$i++) {
      echo "[<a href=\"/cgi-bin/bbs/bbsufind?search=" . chr(ord('A')+$i) . "\">" . chr(ord('A')+$i) . "</a>]";
    }
?>
<br>
[<a href="bbsfriend.php">在线好友</a>] [<a href="bbsuser.php?start=<?php echo $start+$num;?>">下一页</a>]<br><form method="GET">
<input type="submit" value="跳转到第"> <input type="input" size="4" name="start"> 个使用者</form></center>

<?php 
    } //nologinok

?>
</html>

