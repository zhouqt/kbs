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
    $arr = array();
    foreach($articles as $article) {
        print("<topic>");
        xi("id", $article["ID"]);
        xi("type", "");
        xi("title", int_string(htmlspecialchars($article["TITLE"])));
        xi("link", "");
        xi("if_new", "");
        xi("author", $article["OWNER"]);
        xi("reply", $article["REPLYCOUNT"]);
        xi("view", "");
        xi("last_reply_time", $article["LAST_POSTTIME"]);
        xi("last_reply_author", $article["LAST_OWNER"]);
        $filename = bbs_get_board_filename($bname, $article["FILENAME"]);
        $ret = bbs_parse_article($filename, $arr, 1);
        if($ret == 0) {
            xi("brief", "<![CDATA[" . int_string(htmlspecialchars($arr["brief"])) . " ]]>");
        }
        print("</topic>");
    }
    print("</topics>");

?>
