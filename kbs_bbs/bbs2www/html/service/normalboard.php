<?php
require ('service.php');

if (!web_service_check_host ())
    web_service_error (1,'Unknow Host');

$board = $_POST['board'];
$brdarr = array ();

if (bbs_getboard($board,$brdarr)==0)
    web_service_error (1,'Board Not Exist');

$board = $brdarr['NAME'];

if (bbs_normalboard($board))
    web_service_error (0,'OK');
else
    web_service_error (1,'NO');

exit ();
?>