<?php
// $Id$
require_once("funcs.php");
require_once("style.inc.php");

$style = intval($_GET["style"]);
$new_wwwparams = $style . substr($wwwparams,1,strlen($wwwparams)-1);
setcookie("WWWPARAMS",$new_wwwparams,0,"");
if(strcmp($currentuser["userid"],"guest"))
	bbs_setwwwparameters($new_wwwparams);
//header("Location: mainpage.html");
?>
<script language="javascript">
history.go(-2)
</script>

