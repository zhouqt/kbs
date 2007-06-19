<?php

    include("funcs.php");

    $bid = $_GET["bid"];
    if(isset($_GET["page"]))
        $page = $_GET["page"];
    else
        $page = 0;
    
    $bname = bbs_getbname($bid);
    if($bname == "")
        ie("board not found.");
    if(!bbs_checkreadperm($uid, $bid))
        ie("permission denied.");
    bbs_checkorigin($bname);
    $total = bbs_countarticles($bid, $dir_modes["ORIGIN"]);
    $pagecount = ceil($total / PAGE_SIZE);
    if($page < 0)
        $page = 0;
    if($page > $pagecount)
        $page = $pagecount;
    if($page == 0)
        $page = $pagecount;
    $start = ($page - 1) * PAGE_SIZE + 1;
    $count = PAGE_SIZE;

    if($page == $pagecount)
        cache_header("nocache");
    else {
        $origin_if = bbs_get_board_index($bname, $dir_modes["ORIGIN"]);
        if(cache_header("public", @filemtime($origin_if), 10))
            exit;
    }
    
    $articles = bbs_getarticles($bname, $start, $count, $dir_modes["ORIGIN"]);
    if($articles == FALSE)
        ie("cannot read index.");
    $response->pagecount = $pagecount;
    $arr = array();
    $i = 0;
    foreach($articles as $article) {
        $response->topic[$i]->id = $article["ID"];
        $response->topic[$i]->title = int_string(htmlspecialchars($article["TITLE"]));
        $response->topic[$i]->author = $article["OWNER"];
        $response->topic[$i]->reply = $article["REPLYCOUNT"];
        $response->topic[$i]->last_reply_time = $article["LAST_POSTTIME"];
        $response->topic[$i]->last_reply_author = $article["LAST_OWNER"];
        $filename = bbs_get_board_filename($bname, $article["FILENAME"]);
        $ret = bbs_parse_article($filename, $arr, 1);
        if($ret == 0) {
            $response->topic[$i]->brief = int_string(htmlspecialchars($arr["brief"]));
        }
        $i++;
    }
    
    int_finish($response);

?>
