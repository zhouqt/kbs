<?php
require ('service.php');

if (!web_service_check_host ())
    web_service_error (1,'Unknow Host');
    
$board = $_POST['board'];
if (!$board)
    web_service_error (1,'Need Board Name');

$brdarr = array();
$bid = bbs_getboard ($board,$brdarr);

if (!$bid) 
    web_service_error (1,'Board Not Exist');

web_service_etems($brdarr);
exit ();
?>