<?php
if (!defined('_BBS_WWW2_BOARD_PHP_'))
{
define('_BBS_WWW2_BOARD_PHP_', 1);

function bbs_boards_navigation_bar()
{
?>
<p align="center">
[<a href="<?php echo MAINPAGE_FILE; ?>">首页导读</a>]
[<a href="bbssec.php">分类讨论区</a>]
[<a href="bbsxmlbrd.php?flag=2">新开讨论区</a>]
[<a href="bbsxmlbrd.php?flag=0">推荐讨论区</a>]
[<a href="bbsxmlbrd.php?flag=1">讨论区人气排名</a>]
[<a href="bbs0an.php">精华公布栏</a>]
[<a href="javascript:history.go(-1)">快速返回</a>]
<br />
</p>
<?php	
}

function undo_html_format($str)
{
	$str = preg_replace("/&apos;/i", "'", $str);
	$str = preg_replace("/&gt;/i", ">", $str);
	$str = preg_replace("/&lt;/i", "<", $str);
	$str = preg_replace("/&quot;/i", "\"", $str);
	$str = preg_replace("/&amp;/i", "&", $str);
	return $str;
}

if (version_compare(PHP_VERSION,'5','>='))
	require_once('domxml-php4-to-php5.inc.php'); //Load the PHP5 converter

# iterate through an array of nodes
# looking for a text node
# return its content
function get_content($parent)
{
	$nodes = $parent->child_nodes();
	while($node = array_shift($nodes))
		if ($node->node_type() == XML_TEXT_NODE)
			return $node->node_value();
	return "";
}

# get the content of a particular node
function find_content($parent,$name)
{
	$nodes = $parent->child_nodes();
	while($node = array_shift($nodes))
		if ($node->node_name() == $name)
			return undo_html_format(urldecode(get_content($node)));
	return "";
}

function bbs_rss_link($board, $ftype) {
	global $dir_modes;
	$type = '';
	switch($ftype) {
		case $dir_modes["DIGEST"]: $type = 'g'; break;
		case $dir_modes["MARK"]:   $type = 'm'; break;
		//case $dir_modes["ORIGIN"]: $type = 'o'; break;
		default: break;
	}
	return $type ? 'rss.php?' . $type . $board : '';
}

function bbs_board_header($brdarr,$ftype,$managemode,$isnormalboard=FALSE) {
	global $dir_modes, $dir_name;
	$brd_encode = urlencode($brdarr["NAME"]);
	
	$adds = $isnormalboard ? bbs_rss_link($brd_encode, $ftype) : "";
	if ($adds) {
		$adds = '<link title="版面 RSS'	. $dir_name[$ftype] 
		      . '" type="application/rss+xml" rel="alternate" href="' . $adds . '"/>';
	}
	if (!$adds) $adds = false;
	page_header($brdarr["NAME"] . " 版" . $dir_name[$ftype], FALSE, $adds);
?>
<body><div class="nav smaller">
<div class="fleft">
<a href="<?php echo MAINPAGE_FILE; ?>"><?php echo BBS_FULL_NAME; ?></a><span id="idExp"></span> → 
<?php
	$sec_index = get_secname_index($brdarr["SECNUM"]);
	if ($sec_index >= 0) {
?>
<a href="bbsboa.php?group=<?php echo $sec_index; ?>"><?php echo constant("BBS_SECNAME".$sec_index."_0"); ?></a> → 
<?php
	}
?>
<a href="bbsdoc.php?board=<?php echo $brdarr["NAME"]; ?>"><?php echo htmlspecialchars($brdarr["DESC"]); ?></a>
(<a href="bbsfav.php?bname=<?php echo $brdarr["NAME"]; ?>&select=0">收藏</a>)</div>
<div class="fright">
版主: <script>writeBMs('<?php echo $brdarr["BM"]; /* No addslashes! */ ?>');</script>, 在线 <?php echo $brdarr["CURRENTUSERS"]+1; ?> 人
</div>
</div>
<h1 class="bt"><?php echo $brdarr["NAME"]."(".htmlspecialchars($brdarr["DESC"]).")"; ?> 版</h1>
<?php
}

 
function bbs_ann_updirs($path,&$board,&$up_dirs) {
	$board = '';
	$path = ltrim(trim($path));
	if ($path[0]!='/') $path='/'.$path;
	if ($path[strlen($path)-1]=='/') $path = substr($path,0,strlen($path)-1);
	$up_dirs = array();
	$buf = '';
	$dirs = explode('/',$path);
	$j = 0;
	foreach($dirs as $dir) {
		if (($dir)&&($dir!='.')) {
			if (!strcmp('0Announce',$dir))
				continue;
			$buf .= '/'.$dir;
			$up_dirs[] = $buf;
			if ($j == 2) $board = $dir;    
			$j ++;
		}
	}
	return sizeof($up_dirs);
}

function bbs_ann_header($board='') {
	if ($board) {
		page_header("精华公布栏", '<a href="bbsdoc.php?board='.$board.'">'.$board.'版</a>');
		echo '<h1 class="bt">' . $board . ' 版精华公布栏</h1>';
	} else {
		page_header("精华公布栏");
	}
}

function bbs_ann_xsearch($board) {
	if (!defined("SITE_NEWSMTH")) return;
?>
<form action="bbsxsearch.php" class="right wide smaller">
	<label><a href="bbsxsearch.php">令狐冲精华区搜索引擎</a></label>
	<input type="text" name="q" size="31" />
<?php
	if ($board) {
?>
范围
<input type="radio" name="b" value="" />全站
<input type="radio" name="b" checked value="<?php echo urlencode($board); ?>" /><?php echo $board; ?>版
<?php        
	}
?>	     
	<input type="submit" style="width: 80px" value="开始搜" />
</form>    
<?php
}


function bbs_ann_foot($parent) {
?>   
<div class="oper smaller">
[<?php bbs_add_super_fav ('精华区'); ?>]
[<a href="<?php echo MAINPAGE_FILE; ?>">返回首页</a>]
<?php   
	if ($parent){
		if(isset($_GET["annbm"]))
		{
?>
[<a href="bbs0anbm.php?path=<?php echo rawurlencode($parent); ?>">上级目录</a>]
<?php
		}
		else
		{
?>
[<a href="bbs0an.php?path=<?php echo rawurlencode($parent); ?>">上级目录</a>]
<?php
		}
	}
?>
[<a href="bbs0an.php">根目录</a>]
<?php
	if (defined("SITE_NEWSMTH")) {
?>
[<a href="bbsxsearch.php">令狐冲精华区搜索</a>]
<?php
	}
?>
[<a href="#listtop">返回顶部</a>]
[<a href="javascript:location.reload()">刷新</a>] 
[<a href="javascript:history.go(-1)">返回</a>] 
</div>
<?php    
}

function bbs_ann_bm_foot($parent) {
?>   
<div class="oper smaller">
<?php   
	if ($parent){
?>
[<a href="bbs0anbm.php?path=<?php echo rawurlencode($parent); ?>">上级目录</a>]
<?php
	}
?>
[<a href="#listtop">返回顶部</a>]
[<a href="javascript:history.go(-1)">返回</a>] 
</div>
<?php    
}









/**
 * Constants of board flags, packed in an array.
 */
$BOARD_FLAGS = array(
	"VOTE" => 0x01,
	"NOZAP" => 0x02,
	"READONLY" => 0x04,
	"JUNK" => 0x08,
	"ANONY" => 0x10,
	"OUTGO" => 0x20,
	"CLUBREAD" => 0x40,
	"CLUBWRITE" => 0x80,
	"CLUBHIDE" => 0x100,
	"ATTACH" => 0x200,
	"NOREPLY" => 0x2000
	);



/**
 * Checking whether a board is set with some specific flags or not.
 * 
 * @param $board the board object to be checked
 * @param $flag the flags to check
 * @return TRUE  the board is set with the flags
 *         FALSE the board is not set with the flags
 * @author flyriver
 */
function bbs_check_board_flag($board,$flag)
{
	if ($board["FLAG"] & $flag)
		return TRUE;
	else
		return FALSE;
}

/**
 * Checking whether a board is an anonymous board or not.
 * 
 * @param $board the board object to be checked
 * @return TRUE  the board is an anonymous board
 *         FALSE the board is not an anonymous board
 * @author flyriver
 */
function bbs_is_anony_board($board)
{
	global $BOARD_FLAGS;
	return bbs_check_board_flag($board, $BOARD_FLAGS["ANONY"]);
}

/**
 * Checking whether a board is an outgo board or not.
 * 
 * @param $board the board object to be checked
 * @return TRUE  the board is an outgo board
 *         FALSE the board is not an outgo board
 * @author flyriver
 */
function bbs_is_outgo_board($board)
{
	global $BOARD_FLAGS;
	return bbs_check_board_flag($board, $BOARD_FLAGS["OUTGO"]);
}

/**
 * Checking whether a board is a junk board or not.
 * 
 * @param $board the board object to be checked
 * @return TRUE  the board is a junk board
 *         FALSE the board is not a junk board
 * @author flyriver
 */
function bbs_is_junk_board($board)
{
	global $BOARD_FLAGS;
	return bbs_check_board_flag($board, $BOARD_FLAGS["JUNK"]);
}

/**
 * Checking whether a board is an attachment board or not.
 * 
 * @param $board the board object to be checked
 * @return TRUE  the board is an attachment board
 *         FALSE the board is not an attachment board
 * @author flyriver
 */
function bbs_is_attach_board($board)
{
	global $BOARD_FLAGS;
	return bbs_check_board_flag($board, $BOARD_FLAGS["ATTACH"]);
}

/**
 * Checking whether a board is a readonly board or not.
 * 
 * @param $board the board object to be checked
 * @return TRUE  the board is a readnoly board
 *         FALSE the board is not a readonly board
 * @author flyriver
 */
function bbs_is_readonly_board($board)
{
	global $BOARD_FLAGS;
	return bbs_check_board_flag($board, $BOARD_FLAGS["READONLY"]);
}

function bbs_is_noreply_board($board)
{
	global $BOARD_FLAGS;
	return bbs_check_board_flag($board, $BOARD_FLAGS["NOREPLY"]);
}


} // !define ('_BBS_WWW2_BOARD_PHP_')
?>
