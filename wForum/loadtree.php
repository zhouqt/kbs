<?php
require("inc/funcs.php");
header("Expires: .0");

if (!isset($_GET['bname'])){
	exit(0);
}
$boardName=$_GET['bname'];
if (!isset($_GET['ID'])){
	exit(0);
}

$articleID=intval($_GET['ID']);
$brdArr=array();
$boardID= bbs_getboard($boardName,$brdArr);
$boardArr=$brdArr;
$boardName=$brdArr['NAME'];
if ($boardID==0) {
	exit(0);
}
$usernum = $currentuser["index"];
if (bbs_checkreadperm($usernum, $boardID) == 0) {
	exit(0);
	return false;
}
bbs_set_onboard($boardID,1);

/* wforum's original implementaion. going to remove */
/*
$articles = bbs_get_article($boardName, $articleID);
@$article=$articles[0];
if ($article==NULL) {
	exit(0);
}
$threadNum=bbs_get_thread_article_num($boardName,intval($article['ID']));
$total=$threadNum+1;
$threads=bbs_get_thread_articles($boardName, intval($article['ID']),0,$total);
$total=count($threads);
*/
$num = bbs_get_threads_from_gid($boardID, $articleID, 0 , $threads , $haveprev );
if ($num==0) {
	exit(0);
}

//showTree($boardName,$boardID,$articleID,$article,$threads,$total);
?>
<body>
<script language="javascript" type="text/javascript" src="inc/browser.js"></script>
<script language="javascript">
	oTd=getParentRawObject("followTd<?php echo $articleID; ?>");
	oTd.innerHTML='<TABLE border=0 cellPadding=0 cellSpacing=0 width="100%" align=center><TBODY><?php showTree($boardName,$boardID,$articleID,$threads,count($threads));?></TBODY></TABLE>';
</script>
</body>

<?php
/*  rem by roy 2003.7.28
	按文章之间的继承关系排序

function showTree($boardName,$boardID,$articleID,$article,$threads,$threadNum) {
	$printed=array(); 
	$nowID=array();
	$t=array();
	$p=0;
	$nowID[0]=0;
	$try[0]=-1;
	for(;;) {
		for ($i=$try[$p]+1;$i<$threadNum;$i++){
			if ( (!isset($printed[$i])) && ( ($threads[$threadNum-$i-1]['REID']==$nowID[$p]) || $p==0) ) 
				break;
		}
		if ($i<$threadNum) {
			$try[$p]=$i;
			$p++;
			showTreeItem($boardName,$articleID,$threads[$threadNum-$i-1],$i,$p);
			$printed[$i]=1;
			$nowID[$p]=intval($threads[$threadNum-$i-1]['ID']);
			$try[$p]=-1;
		} else {
			$p--;
			if ($p<0) 
				break;
		}
	} 

}
*/
/* rem by atppp 2004.06.07
function showTree($boardName,$boardID,$articleID,$article,$threads,$threadNum) {
	$IDs=array();
	$nodes=array();
	$printed=array();
	$level=array();
	$head=0;
	$bottom=0;
	$IDs[$bottom]=intval($article['ID']);
	$level[$bottom]=0;
	$printed[0]=1;
	$nodes[0]=0;
	$bottom++;
	while($head<$bottom) {
		if ($head==0) 
			showTreeItem($boardName,$articleID,$article, 0, 0);
		else 
			showTreeItem($boardName,$articleID,$threads[$nodes[$head]],$nodes[$head], $level[$head]);
		for ($i=1;$i<=$threadNum;$i++){
			if ( (!isset($printed[$i])) && ($threads[$i]['REID']==$IDs[$head]) ) {
				$IDs[$bottom]=intval($threads[$i]['ID']);
				$level[$bottom]=$level[$head]+1;
				$printed[$i]=1;
				$nodes[$bottom]=$i;
				$bottom++;
			}
		}
		$head++;
	}
}
*/


class TreeNode {
	var $data;
	var $index;
	var $first_child;
	var $last_child;
	var $next_sibling;
	
	function TreeNode($data, $index) {
		$this->data = &$data;
		$this->index = $index;
		$this->first_child = $this->last_child = $this->next_sibling = null;
	}
	
	function addChild(&$node) { /* here it's very important to assign by reference */
		if ($this->first_child == null) $this->first_child = &$node;
		if ($this->last_child != null) {
			$this->last_child->next_sibling = &$node;
		}
		$this->last_child = &$node;
	}
}

function showTree($boardName,$boardID,$articleID,$threads,$threadNum) {
	$more = ($threadNum > 30);
	if ($more) $threadNum = 30; //最多显示30个，先这样吧。
	
	/* 产生回复树结构 */
	$treenodes = array();
	for($i=0; $i < $threadNum; $i++) {
		$treenodes[$i] = new TreeNode($threads[$i], $i);
	}
	for($i=1; $i < $threadNum; $i++) {
		for ($j=0; $j < $threadNum; $j++) {
			if ($i == $j) continue;
			if ($threads[$i]['REID'] == $threads[$j]['ID']) {
				$treenodes[$j]->addChild($treenodes[$i]);
				break;
			}
		}
	}
	
	$lastflag = array(); //$lastflag[level]: 0: no more follows at this level; 1: more follows at this level;
	showTreeRecursively($boardName, $boardID, $articleID, $treenodes, 0, 0, $lastflag);
	
	if ($more) echo '<tr><td class=TableBody1 width="100%" height=25 style="color:red"> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;... 还有更多 ...</td></tr>';
}

function showTreeRecursively($boardName, $boardID, $articleID, &$treenodes, $index, $level, &$lastflag) {
	showTreeItem($boardName,$articleID, $treenodes[$index]->data, $index, $level, $lastflag);
	$cur = &$treenodes[$index]->first_child;
	while($cur != null) {
		$temp = &$cur->next_sibling;
		$lastflag[$level] = ($temp != null);
		showTreeRecursively($boardName, $boardID, $articleID, $treenodes, $cur->index, $level+1, $lastflag);
		$cur = &$temp;
	}
}

function showTreeItem($boardName,$articleID,$thread,$threadID,$level, $lastflag){
	echo '<TR><TD class=TableBody1 width="100%" height=25>';
	for ($i=0;$i<$level;$i++) {
		if ($lastflag[$i]) {
			if ($i == $level - 1) echo '<img src="pic/line2.gif">'; // |-
			else echo '<img src="pic/line.gif">';                   // |
		} else {
			if ($i == $level - 1) echo '<img src="pic/line1.gif">'; // \
			else echo "&nbsp;&nbsp;";                               // nothing
		}
	}
	echo '<img src="pic/nofollow.gif"><a href="disparticle.php?boardName='.$boardName.'&ID='.$articleID.'&start='.$threadID.'&listType=1">';
	/* 再多也多不过 ARTICLE_TITLE_LEN，别省略了吧 - atppp */
/*	if (strLen($thread['TITLE'])>22) { 
		echo htmlspecialchars(substr($thread['TITLE'],0,22),ENT_QUOTES).'...';
	} else {
		echo htmlspecialchars($thread['TITLE'],ENT_QUOTES);
	} */
	echo htmlspecialchars($thread['TITLE'],ENT_QUOTES);
	echo '</a> -- <a href="dispuser.php?id='.$thread['OWNER'].'">'.$thread['OWNER'].'</a></td></tr>';
}
?>
