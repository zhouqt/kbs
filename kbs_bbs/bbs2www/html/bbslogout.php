<?php
require("www2-funcs.php");
login_init();
bbs_wwwlogoff();
cache_header("nocache");
setcookie("UTMPKEY","",time()-3600,"/");
setcookie("UTMPNUM","",time()-3600,"/");
setcookie("UTMPUSERID","",time()-3600,"/");
setcookie("WWWPARAMS","",time()-3600,"/");

header("Location: index.html");
?>
