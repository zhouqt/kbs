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

function wwint_post_data($url, $content) {
    $responsetext = http_post_data($url, $content);
    $pos = strpos($responsetext, "\r\n\r\n") + 4;
    $resulttext = substr($responsetext, $pos, strlen($responsetext) - $pos);
    print($resulttext);
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

function wwint_post($sid, $userid, $passwd, $bid, $reid, $title, $content) {
    global $kbssite;
    $url = "{$kbssite[$sid]}post.php";
    $request->userid = $userid;
    $request->passwd = $passwd;
    $request->bid = $bid;
    $request->reid = $reid;
    $request->title = $title;
    $request->content = $content;
    $requesttext = json_encode($request);
    $response = wwint_post_data($url, $requesttext);
    return($response);
}

?>
