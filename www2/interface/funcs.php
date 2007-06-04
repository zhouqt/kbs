<?php

/* KBS Codename WaterWood
 *   interfaces for new version of web
 *  pig2532@newsmth, 2007
 */

$domain = bbs_get_webdomain();

/*$clientip = $_SERVER["REMOTE_ADDR"];
if(($clientip != "127.0.0.1") && ($clientip != "192.168.1.100"))
    exit;*/

include("../www2-funcs.php");
login_init(TRUE);
$sid = bbs_getsessionid();
cache_header("nocache");
$uid = $currentuser["index"];

$retstr = "";

function int_string($str) {
    return(iconv("gb2312", "UTF-8", $str));
}

function int_xml_header() {
    global $sid, $currentuser;
    header("Content-Type: text/xml; charset=utf-8");
    print("<?xml version=\"1.0\" encoding=\"utf-8\" ?><wwint>");
    print("<sid>{$sid}</sid>");
    print("<userid>{$currentuser["userid"]}</userid>");
}

function int_xml_finish() {
    global $retstr;
    print($retstr);
    print("</wwint>");
}

function xi($key, $value) {
    global $retstr;
    $retstr .= "<{$key}>{$value}</{$key}>";
}

function xe($text) {
    global $retstr;
    $retstr = "";
    xi("error", $text);
    int_xml_finish();
    exit;
}

function int_article($article, $fname, $bid) {
    global $domain;
    $arr = array();
    $ret = bbs_parse_article($fname, $arr, 0);
    if($ret < 0)
        xe("cannot parse article.");
    $retstr .= "<author>";
    xi("name", $arr["userid"]);
    xi("nick_name", int_string(htmlspecialchars($arr["username"])));
    xi("reg_date", "");
    xi("a_post", "");
    xi("a_reply", "");
    $retstr .= "</author>";
    xi("title", int_string(htmlspecialchars($article["TITLE"])));
    xi("text", "<![CDATA[" . int_string(bbs_printansifile($fname, 1, "http://{$domain}/bbscon.php?bid={$bid}&amp;id={$article["ID"]}", 0, 0)) . " ]]>");
    xi("publish_time", $article["POSTTIME"]);
    xi("publish_author", $article["OWNER"]);
    xi("modify_time", "");
    xi("modify_author", "");
}

?>
