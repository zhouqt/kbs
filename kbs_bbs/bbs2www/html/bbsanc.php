<?php
	/**
	 * This file displays article to user in digest .
	 * $Id$
	 */
	require_once("funcs.php");
	require_once("board.inc.php");
	if (defined ("SITE_SMTH")) {
	    include_once ("roam_server.php");
	    roam_login_init();
	}
	else
        login_init();

	if ($loginok != 1)
		html_nologin();
	else
	{
		if (isset($_GET["path"]))
			$path = $_GET["path"];
		else {
			html_init("gb2312");
			html_error_quit("错误的文章");
		}

    	if (strstr($path, ".Names") || strstr($path, "..") || strstr($path, "SYSHome")){
			html_init("gb2312");
			html_error_quit("错误的文章");
		}

		if($path[0]=='/')
			$filename="0Announce".$path;
		else
			$filename="0Announce/".$path;

        if (defined ("SITE_SMTH")) {
            $ret =  bbs_roam_ann_traverse_check($filename,$currentuser["userid"]);
            if( $ret < 0 )
                html_error_quit("系统错误"); 
            if ($ret != 1) {
    			html_init("gb2312");
    			html_error_quit("错误的目录");
    		}
        }
        else {
            if( bbs_ann_traverse_check($filename,$currentuser["userid"]) < 0 ) {
    			html_init("gb2312");
    			html_error_quit("错误的目录");
    		}
        }
		
		if(! file_exists($filename)){
			html_init("gb2312");
			html_error_quit("错误的文章号...");
		}
		
		$board = '';
		$up_dirs = array();
		$up_cnt = bbs_ann_updirs($filename,$board,$up_dirs);
		
if ($board) {
    $brdarr = array();
    if (defined ("SITE_SMTH")) {
        $bid = bbs_roam_getboard($board,$brdarr);
        if ($bid < 0)
            html_error_quit('系统错误');
        if ($bid) {
            $board = $brdarr['NAME'];
            $usernum = $currentuser['index'];
            $ret = bbs_roam_checkreadperm($usernum, $bid);
            if ($ret < 0)
                html_error_quit('系统错误');
            if ($ret == 0) 
        		html_error_quit('不存在该目录');
            $ret = bbs_roam_normalboard($board);
            if ($ret < 0)
                html_error_quit('系统错误');
            if ($ret == 1) {
                if (cache_header('public, must-revalidate',filemtime($filename),10))
                    return;
            }
        }
        else {
            $board = '';
            if (cache_header('public, must-revalidate',filemtime($filename),10))
                return;
        }
    }
    else {
        $bid = bbs_getboard($board,$brdarr);
        if ($bid) {
            $board = $brdarr['NAME'];
            $usernum = $currentuser['index'];
            if (bbs_checkreadperm($usernum, $bid) == 0) 
        		html_error_quit('不存在该目录');
            bbs_set_onboard($bid,1);
            if (bbs_normalboard($board)) {
                if (cache_header('public, must-revalidate',filemtime($filename),10))
                    return;
            }
        }
        else {
            $board = '';
            if (cache_header('public, must-revalidate',filemtime($filename),10))
                return;
        }
    }
}
else {
    if (cache_header('public, must-revalidate',filemtime($filename),10))
        return;
    $bid = 0;
}

			@$attachpos=$_GET["ap"];//pointer to the size after ATTACHMENT PAD
			if ($attachpos!=0) {
				require_once("attachment.php");
				output_attachment($filename, $attachpos);
				exit;
			} else
			{
				html_init("gb2312","","",1);
				$bid?bbs_board_header($brdarr):bbs_ann_header($board);
				bbs_ann_xsearch($board);
?>
<center>
<table width="98%" border="0" class="t1" cellspacing="0" cellpadding="3" >
<tr><td class="t2">精华区文章阅读</td></tr>
<tr><td class="t7"><font class="content">
<?php
				bbs_print_article($filename,1,$_SERVER['PHP_SELF'] . '?' . $_SERVER['QUERY_STRING']);
?>
</font></td></tr></table></center>
<?php
            
            if ($bid)
            bbs_board_foot($brdarr,'');
    
			($up_cnt >= 2)?bbs_ann_foot($up_dirs[$up_cnt - 2]):bbs_ann_foot('');
			html_normal_quit();
			}
	}
?>
