<?php
require ('service.php');

if (!web_service_check_host ())
    web_service_error (1,'Unknow Host');

$filename = $_POST['filename'];
$userid   = $_POST['userid'];

if (bbs_ann_traverse_check($filename,$userid))
    web_service_error (0,'OK');
else
    web_service_error (1,'NO');

exit ();
?>