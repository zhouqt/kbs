<?php

    include("funcs.php");

    int_xml_header();
    
    $bid = $_GET["bid"];
    $id = $_GET["id"];
    
    $bname = bbs_getbname($bid);
    if($bname == "")
        xe("board not found.");
    if(!bbs_checkreadperm($uid, $bid))
        xe("permission denied.");
    $haveprev = 0;
    $articles = array();
    $ret = bbs_get_threads_from_gid($bid, $id, 1, $articles, $haveprev);
    if($ret == 0)
        xe("cannot read threads.");
    print("<topic>");
    
    $start = 0;
    if($articles[0]["ID"] == $articles[0]["GROUPID"]) {
        print("<original>");
        $filename = bbs_get_board_filename($bname, $articles[0]["FILENAME"]);
        int_article($articles[0], $filename, $bid);
        print("</original>");
        $start = 1;
    }
    
    print("<replys>");
    for($i=$start; $i<$ret; $i++) {
        print("<reply>");
        $filename = bbs_get_board_filename($bname, $articles[$i]["FILENAME"]);
        int_article($articles[$i], $filename, $bid);
        print("</reply>");
    }
    print("</replys>");
    
    print("</topic>");
    
?>
