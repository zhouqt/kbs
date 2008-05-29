<?php
require("www2-funcs.php");
require("www2-board.php");
login_init();
bbs_session_modify_user_mode(BBS_MODE_READING);

$bbsman_modes = array(
	"DEL"   => 1,
	"MARK"  => 2,
	"DIGEST"=> 3,
	"NOREPLY" => 4,
	"ZHIDING" => 5,
	"UNDEL" => 6,
	"PERCENT" => 7,
	"TODEL" => 8,
	"SHARP" => 9,
	"ANNOUNCE" => 10,
	"ANNOUNCEADD" => 11,
	"IMPORT_I" => 12,
	"IMPORT_J" => 13,
	"CENSOR" => 14
);

function do_manage_function($board) {
	global $bbsman_modes;
    $mode = intval($_POST['act']);
    $anncode = $bbsman_modes['ANNOUNCE'];
    if ($mode > 0 && $mode <= sizeof($bbsman_modes)) {
        for ($i = 0 ; $i < ARTCNT ; $i ++) {
            if (isset($_POST['art'.$i])) {
                if (intval($_POST['art'.$i])) {
                    $id = intval($_POST['art'.$i]);
                    $zhiding = 0;
                }
            }
            elseif (isset($_POST['ding'.$i])) {
                if (intval($_POST['ding'.$i])) {
                    $id = intval($_POST['ding'.$i]);
                    $zhiding = 1;
                }
            }
            else
                continue;
            
            if (!$id)   continue;
            
            if ($mode == $bbsman_modes['ANNOUNCE'])
            {
            	$ret = bbs_bmmanage($board, $id, $anncode, $zhiding);
            	if ($ret == -9)
            	{
            		html_error_quit('系统错误，无法修改精华区剪贴板。');
            		exit;
            	}
            	$anncode = $bbsman_modes['ANNOUNCEADD'];
            	continue;
            }
                          
            if ($zhiding && ($mode != $bbsman_modes['UNDEL'])) {
                 if ($mode !=  $bbsman_modes['DEL'] && $mode != $bbsman_modes['ZHIDING'])
                    continue;   
                 $mode = $bbsman_modes['DEL'];
            }
            
            $ret = bbs_bmmanage($board,$id,$mode,$zhiding);
            switch ($ret) {
                case -1:
                case -2:
                case -3:
                case -9:
                	if($mode != 6)
                	{
                    	html_error_quit('系统错误');
                	}
                    break;
                case -4:
                    html_error_quit('文章ID错误');
                    break;
                default:    
            }
        }
    }
    if($mode == $bbsman_modes["ANNOUNCE"])
    {
		$ann_path = bbs_getannpath($board);
		if ($ann_path != FALSE)	{
			if (!strncmp($ann_path,"0Announce/",10))
				$ann_path = substr($ann_path,9);
    		// 丝路功能问题多多 先不要用
			// header("Location: bbsipath.php?annpath={$ann_path}");
			header("Location: bbs0anbm.php?path={$ann_path}");
		}
    }
}




function display_articles($brdarr,$articles,$start,$ftype,$managemode,$page,$total,$showHot,$isnormalboard)
{
	global $brdnum, $usernum, $dir_modes, $show_none, $isclub;
	$board = $brdarr["NAME"];
	$ann_path = bbs_getannpath($board);
	if ($ann_path != FALSE)	{
		if (!strncmp($ann_path,"0Announce/",10))
			$ann_path = substr($ann_path,9);
	}
	if($ftype == $dir_modes["DELETED"])
	{
		$mancode = "2";
	}
	else
	{
		$mancode = $managemode?"1":"0";
	}
?>
<script type="text/javascript"><!--
var c = new docWriter('<?php echo addslashes($board); ?>',<?php echo $brdarr["BID"]; ?>,<?php echo $start;
?>,<?php echo $mancode; ?>,<?php echo $ftype; ?>,<?php echo $page; ?>,<?php echo $total;
?>,'<?php echo addslashes($ann_path); ?>',<?php echo $showHot?"1":"0"; ?>,<?php echo $isnormalboard?"1":"0"; ?>);
<?php
	if($show_none)
	{
?>
document.write('<tr><td align="center" colspan="<?php echo ($managemode?6:5); ?>">本区没有文章。</td></tr>');
<?php
	}
	else foreach ($articles as $article)
	{
?>
c.o(<?php echo $article["ID"]; ?>,<?php echo $article["GROUPID"]; ?>,'<?php echo $article["OWNER"]; ?>',<?php
		$flags = $article["FLAGS"];
		echo "'" . $flags[0] . $flags[3] . "'";
?>,<?php echo $article["POSTTIME"]; ?>,'<?php echo addslashes(htmlspecialchars($article["TITLE"], ENT_QUOTES)); ?> ',<?php echo $article["EFFSIZE"]; ?>,<?php echo ($flags[1]=="y")?"1":"0"; ?>,<?php echo $article["IS_TEX"] ? "1" : "0"; ?>);
<?php
	}
?>
c.t();c.f('<?php
	echo $isnormalboard ? bbs_rss_link(urlencode($board), $ftype) : "";
?>',<?php
	$s = TRUE;
	if( defined("SITE_SMTH") ) {
		 $relatefile = $_SERVER["DOCUMENT_ROOT"]."/brelated/".$brdarr["NAME"].".js";
		 if ( file_exists( $relatefile ) ) {
		 	if (@readfile( $relatefile ))
				$s = FALSE;
		}
	}
	if ($s) echo "0";
?>,<?php echo $isclub; ?>);
//-->
</script>
<?php
}

if (isset($_GET["board"]))
	$board = $_GET["board"];
else{
	html_error_quit("错误的讨论区");
}
// 检查用户能否阅读该版
$brdarr = array();
$isnormalboard = bbs_safe_getboard(0, $board, $brdarr);
if (is_null($isnormalboard)) {
	html_error_quit("错误的讨论区");
}
$board = $brdarr["NAME"];
$brdnum = $brdarr["BID"];
if ($brdarr["FLAG"]&BBS_BOARD_GROUP) {
	$i = get_secname_index($brdarr["SECNUM"]);
	if ($i >= 0) {
		 Header("Location: bbsboa.php?group=" . $i . "&group2=" . $brdnum);
		 return;
	}
	html_error_quit("错误的讨论区");
}
$usernum = $currentuser["index"];
$isbm = bbs_is_bm($brdnum, $usernum);

$managemode = isset($_GET["manage"]);
if ($managemode) {
	if (!$isbm)
		html_error_quit("你不是版主");
	define('ARTCNT', 50);
} else {
	define('ARTCNT', 20);
}

if($managemode)
{
	if(isset($_GET["ftype"]))
	{
		$ftype = intval($_GET["ftype"]);
		if(($ftype != $dir_modes["NORMAL"]) && ($ftype != $dir_modes["DELETED"]))
		{
			$ftype = $dir_modes["NORMAL"];
		}
	}
	else 
	{
		$ftype = $dir_modes["NORMAL"];
	}
}
else if(isset($_GET["ftype"])) {
	$ftype = intval($_GET["ftype"]);
	if (!bbs_is_permit_mode($ftype, 0)) {
		html_error_quit("错误的模式");
	}
}
else
{
	$ftype = $dir_modes["NORMAL"];
}

if(($ftype == $dir_modes["DELETED"]) && !$managemode)  //非管理模式不让看回收站，同时也保证不会被cache
{
	html_error_quit("你不能看这个东西哦。");
}

if (bbs_club_flag($board) > 0)
	$isclub = 1;
else
	$isclub = 0;

bbs_set_onboard($brdnum,1);
if ($ftype == $dir_modes["ORIGIN"]) {
	bbs_checkorigin($board);
} else if ($ftype == $dir_modes["MARK"]) {
	bbs_checkmark($board);
}
if (!$managemode && $isnormalboard && (isset($_GET["page"]) || $ftype) ) {
	$dotdirname = bbs_get_board_index($board, $ftype);
	if (cache_header("public",@filemtime($dotdirname),($ftype == $dir_modes["NORMAL"]) ? 10 : 300))
		return;
}

if (isset($_POST['act'])) {
	do_manage_function($board);
}

$show_none = 0;
$total = bbs_countarticles($brdnum, $ftype);
if ($total <= 0) {
	$show_none = 1;
}

if(!$show_none)
{
	if (isset($_GET["page"]))
		$page = $_GET["page"];
	elseif (isset($_POST["page"]))
		$page = $_POST["page"];
	else
	{
		if (isset($_GET["start"]))
		{ /* TODO: 去掉这个参数，递交之前用 javascript 算出 page 来 */
			$start = $_GET["start"];
			settype($start, "integer");
			$page = ($start + ARTCNT - 1) / ARTCNT;
		}
		else
			$page = 0;
	}
	settype($page, "integer");
	if ($page > 0)
		$start = ($page - 1) * ARTCNT + 1;
	else
		$start = 0;
	/*
	 * 这里存在一个时间差的问题，可能会导致序号变乱。
	 * 原因在于两次调用 bbs_countarticles() 和 bbs_getarticles()。
	 */
	if ($start == 0 || $start > ($total - ARTCNT + 1))
	{
		if ($total <= ARTCNT)
		{
			$start = 1;
			$page = 1;
		}
		else
		{
			$start = ($total - ARTCNT + 1);
			$page = ($start + ARTCNT - 1) / ARTCNT + 1;
		}
	}
	else
		$page = ($start + ARTCNT - 1) / ARTCNT;
	settype($page, "integer");
	$articles = bbs_getarticles($brdarr["NAME"], $start, ARTCNT, $ftype);
	if ($articles == FALSE){
		html_error_quit("读取文章列表失败");
	}
}
else
{
	$articles = 0;
	$start = 1;
	$page = 1;
}

bbs_board_header($brdarr,$ftype,$managemode,$isnormalboard);
display_articles($brdarr, $articles, $start, $ftype, $managemode, $page, $total,
	(defined('BBS_NEWPOSTSTAT') && !$managemode && $isnormalboard && !$ftype), $isnormalboard );

page_footer(/*$managemode ? FALSE : TRUE */);
?>
