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

Header('Content-type: application/octet-stream');
Header('Content-Disposition: inline;filename=bbshot.js');
?>
document.write('<center><table cellspacing="0" cellpadding="3" border="0" class="t1" width="98%">');
<?php
    foreach ($threads as $thread) {
?>
document.write('<tbody><tr>');
document.write('<td class="t4" width="20"><font color="red">HOT</font></td>');
document.write('<td class="t4" width="80"><a href="/bbsqry.php?userid=<?php echo $thread['userid']; ?>"><?php echo $thread['userid']; ?></a></td>');
document.write('<td class="t4" width="80"><?php echo date('M d'); ?></td>');
document.write('<td class="t5"><a href="/bbscon.php?board=<?php echo urlencode($board); ?>&id=<?php echo $thread['gid']; ?>"><?php echo htmlspecialchars($thread['title']); ?></a>&nbsp;&nbsp;[<a href="/bbstcon.php?board=<?php echo urlencode($board); ?>&gid=<?php echo $thread['gid']; ?>">同主题</a>]</td>');
document.write('<td class="t4" width="120">[讨论人数: <?php echo $thread['count']; ?>]</td>');
document.write('</tr></tbody>');
<?php
    }
?>
document.write('</table></center>');
