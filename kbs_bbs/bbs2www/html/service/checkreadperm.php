<?php
require ('service.php');

if (!web_service_check_host ())
    web_service_error (1,'Unknow Host');

$usernum = intval ($_POST['usernum']);
$bid = intval ($_POST['bid']);

if ($usernum < 2 || $bid == 0)
    web_service_error (1,'Parameters Error');

if (bbs_checkreadperm($usernum, $bid) == 0)
    web_service_error (1,'No Permission');
else
    web_service_error (0,'OK');

exit ();
?>