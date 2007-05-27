<?php

/* KBS Codename WaterWood
 *   interfaces for new version of web
 *  pig2532@newsmth, 2007
 */

define("WWW2_ROOT", "/home/www/htdocs/");

$clientip = $_SERVER["REMOTE_ADDR"];
if(($clientip != "127.0.0.1") && ($clientip != "192.168.1.100"))
    exit;

include(WWW2_ROOT . "www2-funcs.php");
login_init();
cache_header("nocache");

function int_string($str) {
    return(iconv("gb2312", "UTF-8", $str));
}

function int_xml_header() {
    header("Content-Type: text/xml; charset=gbk");
    print("<?xml version=\"1.0\" encoding=\"gbk\" ?>");
}

function xi($key, $value) {
    print("<{$key}>{$value}</{$key}>");
}

function xe($text) {
    print("error: {$text}");
    exit;
}

function int_article($article, $fname) {
    $arr = array();
    $ret = bbs_parse_article($fname, $arr);
    if($ret < 0)
        xe("cannot parse article.");
    print("<author>");
    xi("name", $arr["userid"]);
    xi("nick_name", htmlspecialchars($arr["username"]));
    xi("reg_date", "");
    xi("a_post", "");
    xi("a_reply", "");
    print("</author>");
    xi("title", htmlspecialchars($article["TITLE"]));
    xi("text", "<![CDATA[" . bbs_printansifile($fname) . "]]>");
    xi("publish_time", $article["POSTTIME"]);
    xi("publish_author", $article["OWNER"]);
    xi("modify_time", "");
    xi("modify_author", "");
}

?>
