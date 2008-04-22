<?php

/* KBS Codename WaterWood
 *   interfaces for new version of web
 *  pig2532@newsmth, 2007
 */

define("PAGE_SIZE", 20);
define("THREAD_PAGE_SIZE", 20);
define("WWW2ROOT", "/home/www/htdocs/");
exit;
/*$clientip = $_SERVER["REMOTE_ADDR"];
if(($clientip != "127.0.0.1") && ($clientip != "192.168.1.100"))
    exit;*/

include(WWW2ROOT . "www2-funcs.php");
include(WWW2ROOT . "www2-board.php");
login_init(TRUE);
$sid = bbs_getsessionid();
$uid = $currentuser["index"];
$xml_header = false;
$domain = bbs_get_webdomain();

$retstr = "";

function int_string($str) {
    return(iconv("gb2312", "UTF-8", $str));
}

function int_finish($response) {
    $responsetext = json_encode($response);
    header("Content-Length: " . strlen($responsetext));
    print($responsetext);
}

function ie($text) {
    $response->error = $text;
    $responsetext = json_encode($response);
    header("Content-Length: " . strlen($responsetext));
    print($responsetext);
    exit;
}

function int_article($article, $fname, $bid) {
    global $domain;
    $arr = array();
    $ret = bbs_parse_article($fname, $arr, 0);
    if($ret < 0)
        ie("cannot parse article.");
    $response->id = $article["ID"];
    $response->reid = $article["REID"];
    $response->groupid = $article["GROUPID"];
    $response->author->name = $arr["userid"];
    $response->author->nick_name = int_string(htmlspecialchars($arr["username"]));
    $response->title = int_string(htmlspecialchars($article["TITLE"]));
    $response->text = int_string(bbs_printansifile($fname, 1, "http://{$domain}/bbscon.php?bid={$bid}&amp;id={$article["ID"]}", 0, 0));
    $response->publish_time = $article["POSTTIME"];
    $response->publish_author = $article["OWNER"];
    return($response);
}

function int_getreq() {
    $fp = fopen("php://input", "r");
    $text = "";
    while(!feof($fp)) {
        $text .= fread($fp, 1024);
    }
    fclose($fp);
    return($text);
}

?>
