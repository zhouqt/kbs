<?php
/**
 * display the top 5 hot threads in board
 * @author: windinsn May 28.2004
 */
require ('funcs.php');
login_init();

if (!defined('BBS_NEWPOSTSTAT'))
   exit ();
    
define('BBS_STAT_HOT',1);
define('BOARD_HOT_THREADS',5);
require ('boards.php');

if (isset($_GET["board"]))
    $board = $_GET["board"];
else
    exit ();

$brdarr = array();
$brdnum = bbs_getboard($board, $brdarr);
if ($brdnum == 0)
    exit ();
    
$board = $brdarr["NAME"];
$usernum = $currentuser["index"];
if (bbs_checkreadperm($usernum, $brdnum) == 0)
    exit ();
if ($brdarr["FLAG"]&BBS_BOARD_GROUP) 
    exit ();
if (bbs_normalboard($board)) {
    if (update_cache_header())
        exit ();
}
$threads = array();
$err = '';
if (!bbs_get_hot_threads($board,BOARD_HOT_THREADS,$threads,$err))
    exit ();

if (sizeof($threads)==0)
    exit ();
    
Header('Content-type: application/octet-stream');
Header('Content-Disposition: inline;filename=bbshot.js');

?>
document.write('<center><table cellspacing="0" cellpadding="5" border="0" width="98%"><tr><td width="100" align="center">[<font color="red">热门话题</font>]</td><td><marquee onmouseover="this.stop()" onmouseout="this.start()">');
<?php
    foreach ($threads as $thread) {
?>
document.write('<a href="/bbscon.php?board=<?php echo urlencode($board); ?>&id=<?php echo $thread['gid']; ?>"><?php echo htmlspecialchars($thread['title']); ?></a>&nbsp;[<a href="/bbstcon.php?board=<?php echo urlencode($board); ?>&gid=<?php echo $thread['gid']; ?>">同主题</a>](<?php echo $thread['count']; ?>)&nbsp;&nbsp;&nbsp;&nbsp;');
<?php
    }
?>
document.write('</marquee></td></tr></table></center>');

