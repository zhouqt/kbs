<?php

/* KBS BBS interface */
/* pig2532 */

$kbssite[1] = "http://127.0.0.1:10080/interface/";

function wwint_get($url) {
    $responsetext = http_get($url);
    $pos = strpos($responsetext, "\r\n\r\n") + 4;
    $resulttext = substr($responsetext, $pos, strlen($responsetext) - $pos);
    $result = json_decode($resulttext);
    return($result);	  	  	
}

function wwint_topics($sid, $bid, $page=0) {
    global $kbssite;
    $url = "{$kbssite[$sid]}topics.php?bid={$bid}";
    if($page > 0)
        $url .= "&page={$page}";
    $response = wwint_get($url);
    return($response);
}

function wwint_topic($sid, $bid, $id, $page=0) {
    global $kbssite;
    $url = "{$kbssite[$sid]}topic.php?bid={$bid}&id={$id}";
    if($page > 0)
        $url .= "&page={$page}";
    $response = wwint_get($url);
    return($response);
}

?>
