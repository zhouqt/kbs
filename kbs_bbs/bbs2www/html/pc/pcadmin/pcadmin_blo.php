<?php
require("pcadmin_inc.php");
pc_admin_check_permission();
$link = pc_db_connect();
$nid = intval($_GET["nid"]);
$query = "UPDATE nodes SET changed  = changed  , nodetype = 1 WHERE nid = '".$nid."' LIMIT 1;";
mysql_query($query,$link);
pc_db_close($link);
?>
<script language="javascript">
history.go(-1);
</script>