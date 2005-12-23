<?php
require("www2-funcs.php");
login_init();
bbs_wwwlogoff();
cache_header("nocache");
delete_all_cookie();

header("Location: index.html");
?>
