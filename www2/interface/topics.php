<?php

    include("funcs.php");

    int_xml_header();
    
    $bid = $_GET["bid"];
    $start = $_GET["start"];
    $count = $_GET["count"];
    
    $bname = bbs_getbname($bid);
    if($bname == "")
        xe("board not found.");
    bbs_checkorigin($bname);
    $total = bbs_countarticles($bid, $dir_modes["ORIGIN"]);
    if($start > $total)
        $start = $total;
    $articles = bbs_getarticles($bname, $start, $count, $dir_modes["ORIGIN"]);
    if($articles == FALSE)
        xe("cannot read index.");
    print("<topics>");
    foreach($articles as $article) {
        print("<topic>");
        xi("type", "");
        xi("title", htmlspecialchars($article["TITLE"]));
        xi("link", "");
        xi("if_new", "");
        xi("author", $article["OWNER"]);
        xi("reply", $article["REPLYCOUNT"]);
        xi("view", "");
        xi("last_reply_time", "");
        xi("last_reply_author", "");
        print("</topic>");
    }
    print("</topics>");

?>
