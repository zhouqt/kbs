<?php
/**
 * display the top 5 hot threads in board
 * @author: windinsn May 28.2004
 */
require ('www2-funcs.php');
login_init();

if (!defined('BBS_NEWPOSTSTAT'))
   exit ();
	
define('BBS_STAT_HOT',1);
define('BOARD_HOT_THREADS',5);

if (isset($_GET["board"]))
	$board = $_GET["board"];
else
	exit ();

$brdarr = array();
$normalboard = bbs_safe_getboard(0, $board, $brdarr);
if (is_null($normalboard)) die;

$board = $brdarr["NAME"];
if ($brdarr["FLAG"]&BBS_BOARD_GROUP) 
	exit ();
if ($normalboard) {
	if (update_cache_header(60))
		exit ();
} else die;



include ('db.php');  // include the database class
if (!($db = new BbsDb)) {
	html_error_quit($db->err);    
}
/**
 * get hot threads of a borad
 * bbs_get_hot_threads(string board)
 * @author: windinsn
 */
function bbs_get_hot_threads($board,$num,&$threads,&$err) 
{
	global $db;
	$brdarr = array();
	$bid = bbs_getboard($board,$brdarr);
	if (!$bid) {
		$err = '版面 '.$board.' 不存在';
		return false;
	}
	$board = $brdarr['NAME'];
	$tt = date('Ymd')."000000";
	$sql = 'SELECT threadid,userid,title,time AS created,MAX(time) AS changed,count(DISTINCT userid) AS count FROM postlog WHERE time>='.$tt.
	       ' AND bname = \''.addslashes($board).'\' GROUP BY threadid ORDER BY count DESC , id DESC LIMIT 0 , '.(intval($num)*2).';';
	if (!$db->query($sql,1)) {
		$err = $db->err;
		return false;    
	}
	$threads = array();
	$n = 0;
	for ($i = 0 ; $i < $db->nums ; $i ++ ) {
		$title = $db->arrays[$i]['title'];
		$gid = $db->arrays[$i]['threadid'];

        $articles = array ();
        if (bbs_get_records_from_id($board, $gid, 0, $articles) <= 0) continue;

		if (substr($title,0,4)=='Re: ')
			$title = substr($title,4);
		$threads[] = array(
				'gid' => $gid,
				'userid' => $db->arrays[$i]['userid'],
				'created' => $db->arrays[$i]['created'],
				'changed' => $db->arrays[$i]['changed'],
				'count'  => $db->arrays[$i]['count'],
				'title' => $title
			);
		$n++;
		if ($n == $num) break;
	}
	return true;
}

$threads = array();
$err = '';
if (!bbs_get_hot_threads($board,BOARD_HOT_THREADS,$threads,$err))
	exit ();

page_header("热门话题", FALSE);
?>
<body><script type="text/javascript"><!--
parent.setHots([<?php
if (sizeof($threads)>0) {
	foreach ($threads as $thread) {
?>
[<?php echo $thread['gid']; ?>, '<?php echo htmlspecialchars($thread['title'], ENT_QUOTES); ?> ', <?php echo $thread['count']; ?>],
<?php
	}
}
?>
0]);
//-->
</script></body></html>