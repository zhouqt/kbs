<?php
/* Í¬²½ PCORP_FLAGS */
set_time_limit(0);
require("pcadmin_inc.php");
pc_admin_check_permission();
$link = pc_db_connect();
$query = 'SELECT `username` FROM users;';
$result = mysql_query($query);
$allUsers = array();
while($rows = mysql_fetch_array($result))
	$allUsers[] = $rows[0];
pc_db_close($link);

echo "<pre>";
foreach ($allUsers as $userid) {
	$ret = bbs_user_setflag($userid, BBS_PCORP_FLAG, 1);
	echo $userid . ($ret ? "" : ": <font color='red'>FAILED</font>") . "\n";
}
echo count($allUsers) . " users.</pre>";
?>
