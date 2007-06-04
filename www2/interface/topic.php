<?php

    include("funcs.php");

    $bid = $_GET["bid"];
    $id = $_GET["id"];
    if(isset($_GET["page"]))
        $page = $_GET["page"];
    else
        $page = 1;
    
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
    $pagecount = ceil($ret / THREAD_PAGE_SIZE);
    if($page < 1)
        $page = 1;
    if($page > $pagecount)
        $page = $pagecount;
    
    if($page == $pagecount)
        cache_header("nocache");
    else {
        $all_if = bbs_get_board_index($bname, $dir_modes["NORMAL"]);
        if(cache_header("public", @filemtime($all_if), 300))
            exit;
    }
   
    int_xml_header();
   
    $retstr .= "<topic>";
    xi("page_count", $pagecount);
    
    $start = ($page - 1) * THREAD_PAGE_SIZE;
    if($start > ($ret - 1))
        $start = $ret - 1;
    $end = $start + THREAD_PAGE_SIZE - 1;
    if($end > ($ret - 1))
        $end = $ret - 1;
    if($articles[$start]["ID"] == $articles[$start]["GROUPID"]) {
        $retstr .= "<original>";
        $filename = bbs_get_board_filename($bname, $articles[$start]["FILENAME"]);
        int_article($articles[$start], $filename, $bid);
        $retstr .= "</original>";
        $start++;
    }
    
    $retstr .= "<replys>";
    for($i=$start; $i<=$end; $i++) {
        $retstr .= "<reply>";
        $filename = bbs_get_board_filename($bname, $articles[$i]["FILENAME"]);
        int_article($articles[$i], $filename, $bid);
        $retstr .= "</reply>";
    }
    $retstr .= "</replys>";
    
    $retstr .= "</topic>";
    
    int_xml_finish();

?>
