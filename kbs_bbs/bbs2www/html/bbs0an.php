<?php
/**
 * show annouce directory
 * windinsn May 19,2004
 *
 */
function bbs_ann_display_articles($articles) {
?>
<center>
<table width="98%" border="0" cellspacing="0" cellpadding="3" class="t1">
<tbody><tr><td class="t2" width="50px">序号</td><td class="t2" width="30px">类型</td><td class="t2">标题</td><td class="t2" width="80px">整理</td><td class="t2" width="80px">编辑日期</td></tr>
</tbody>
<?php
    $i = 1;
    foreach ($articles as $article) {
        echo '<tbody><tr><td class="t3">'.$i.'</td><td class="t4">';
        switch($article['FLAG']) {
            case 0:
                $img = '/images/oldgroup.gif';
                $alt = '错误';
                $url = '';
                break;
            case 1:
                $img = '/images/groupgroup.gif';
                $alt = '目录';
                $url = '/bbs0an.php?path='.rawurlencode($article['PATH']);
                break;
            case 2:
            case 3:
            default:
                $img = '/images/newgroup.gif';
                $alt = '文件';
                $url = '/bbsanc.php?path='.rawurlencode($article['PATH']);
        }
        echo '<img src="'.$img.'" alt="'.$alt.'" border="0" />';
        echo '</td><td class="t7">';
        if ($article['FLAG']==3)
            echo '<font color="red">@</font>';
        else
            echo '&nbsp;';
        if ($url)
            echo '<a href="'.$url.'">'.htmlformat($article['TITLE']).'</a>';
        else
            echo htmlformat($article['TITLE']);
        $bm = explode(' ',$article['BM']);
        $bm = $bm[0];
        echo '</td><td class="t4">'.($bm?'<a href="/bbsqry.php?userid='.$bm.'">'.$bm.'</a>':'&nbsp;').'</td><td class="t3">'.date('Y-m-d',$article['TIME']).'</td></tr></tbody>';
        $i ++;
    }
?>
</table></center>
<?php    
}

require_once('funcs.php');
require_once('board.inc.php');
if (defined ("SITE_SMTH")) {
    include_once ('roam_server.php');
    roam_login_init();
}
else
    login_init();

if (isset($_GET['path']))
	$path = trim($_GET['path']);
else 
	$path = "";

if (strstr($path, '.Names') || strstr($path, '..') || strstr($path, 'SYSHome'))
    html_error_quit('不存在该目录');

$board = '';
$articles = array();
$path_tmp = '';
$ret = bbs_read_ann_dir($path,$board,$path_tmp,$articles);

switch ($ret) {
    case -1:
        html_error_quit('精华区目录不存在');
        break;
    case -2:
        html_error_quit('无法加载目录文件');
        break;
    case -3:
        html_error_quit('该目录尚无文章');
        break;
    case -9:
        html_error_quit('系统错误');
        break;
    default;
}

$path = $path_tmp;
$up_cnt = bbs_ann_updirs($path,$board,$up_dirs);
if ($board) {
    $brdarr = array();
    if (defined ('SITE_SMTH')) {
        $bid = bbs_roam_getboard ($board, $brdarr);
        if ($bid < 0)
            html_error_quit('系统错误');
    }
    else
        $bid = bbs_getboard($board,$brdarr);
    if ($bid) {
        $board = $brdarr['NAME'];
        $usernum = $currentuser['index'];
        if (defined ('SITE_SMTH')) {
            $ret = bbs_roam_checkreadperm($usernum, $bid);
            if ( $ret <= 0)
        		html_error_quit('不存在该目录');
            $ret = bbs_roam_normalboard($board);
            if ( $ret < 0 )
                html_error_quit('系统错误');
            if ( $ret == 1 ) {
                $dotnames = BBS_HOME . '/' . $path . '/.Names';
                if (cache_header('public, must-revalidate',filemtime($dotnames),10))
                    return;
            }
        }
        else {
            if (bbs_checkreadperm($usernum, $bid) == 0)
        		html_error_quit('不存在该目录');
            bbs_set_onboard($bid,1);
            if (bbs_normalboard($board)) {
                $dotnames = BBS_HOME . '/' . $path . '/.Names';
                if (cache_header('public, must-revalidate',filemtime($dotnames),10))
                    return;
            }
        }
        html_init('gb2312','','',1);
        bbs_board_header($brdarr);
    }
    else {
        $board = '';
        html_init('gb2312','','',1);
        bbs_ann_header($board);
    }
    
}
else {
    $dotnames = BBS_HOME . '/' . $path . '/.Names';
    if (cache_header('public, must-revalidate',filemtime($dotnames),10))
        return;
    $bid = 0;
    html_init('gb2312','','',1);
    bbs_ann_header();
}

bbs_ann_xsearch($board);
bbs_ann_display_articles($articles);
if ($bid)
    bbs_board_foot($brdarr,'');
    
if ($up_cnt >= 2)
    bbs_ann_foot($up_dirs[$up_cnt - 2]);
else
    bbs_ann_foot('');
    
html_normal_quit();
?>