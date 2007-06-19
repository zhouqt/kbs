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
        ie("board not found.");
    if(!bbs_checkreadperm($uid, $bid))
        ie("permission denied.");
    $haveprev = 0;
    $articles = array();
    $ret = bbs_get_threads_from_gid($bid, $id, 1, $articles, $haveprev);
    if($ret == 0)
        ie("cannot read threads.");
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
   
    $response->pagecount = $pagecount;
    
    $start = ($page - 1) * THREAD_PAGE_SIZE;
    if($start > ($ret - 1))
        $start = $ret - 1;
    $end = $start + THREAD_PAGE_SIZE - 1;
    if($end > ($ret - 1))
        $end = $ret - 1;
    
    $ii = 0;
    for($i=$start; $i<=$end; $i++) {
        $filename = bbs_get_board_filename($bname, $articles[$i]["FILENAME"]);
        $response->article[$ii] = int_article($articles[$i], $filename, $bid);
        $ii++;
    }
    
    int_finish($response);

?>
