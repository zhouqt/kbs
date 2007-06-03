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
    $retstr .= "<topic>";
    
    $start = 0;
    if($articles[0]["ID"] == $articles[0]["GROUPID"]) {
        $retstr .= "<original>";
        $filename = bbs_get_board_filename($bname, $articles[0]["FILENAME"]);
        int_article($articles[0], $filename, $bid);
        $retstr .= "</original>";
        $start = 1;
    }
    
    $retstr .= <replys>";
    for($i=$start; $i<$ret; $i++) {
        $retstr .= "<reply>";
        $filename = bbs_get_board_filename($bname, $articles[$i]["FILENAME"]);
        int_article($articles[$i], $filename, $bid);
        $retstr .= "</reply>";
    }
    $retstr .= "</replys>";
    
    $retstr .= "</topic>";
    
    int_xml_finish();

?>
