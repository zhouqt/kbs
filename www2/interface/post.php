<?php

    include("funcs.php");

    $requesttext = int_getreq();
    $request = json_decode($requesttext);
    
    set_fromhost();

    if(bbs_check_ban_ip($request->userid, $fromhost) != 0)
        ie("ip denied.");
    
    if(($request->userid == "guest") || (bbs_checkpasswd($request->userid, $request->passwd) != 0))
        ie("invalid user.");

    $bid = $request->bid;
    $userec = array();
    bbs_getuser($request->userid, $userec);
    $uid = $userec["index"];
    
    $bname = bbs_getbname($bid);
    if($bname == "")
        ie("board not found.");
    if(!bbs_checkreadperm($uid, $bid))
        ie("permission denied.");
    
    $barr = array();
    bbs_getboard($bname, $barr);
    if(bbs_is_readonly_board($barr))
        ie("board is readonly.");
    if(!bbs_checkpostperm($uid, $bid))
        ie("post is denied.");

    $title = rtrim($request->title);
    if($title == "")
        ie("empty title.");
    if($request->content == "")
        ie("empty content.");

    $articles = array();
    if($request->reid > 0) {
        $ret = bbs_get_records_from_id($bname, $request->reid, $dir_modes["NORMAL"], $articles);
        if($ret == 0)
            ie("no such article.");
        if($articles[1]["FLAGS"][2] == "y")
            ie("reply is denied.");
    }
    
    $ret = bbs_postarticle($bname, $title, $content, 0, $request->reid, 0, 0, 0, 0);
    if($ret < 0)
        ie("error code: {$ret}.");
    $response->id = $ret;
    int_finish($response);

?>

