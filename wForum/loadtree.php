<?php
require("inc/funcs.php");
require("inc/treeview.inc.php");
header("Expires: .0");

if (!isset($_GET['bname'])){
	exit(0);
}
$boardName=$_GET['bname'];
if (!isset($_GET['ID'])){
	exit(0);
}

$groupID=intval($_GET['ID']);
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
$articles = bbs_get_article($boardName, $groupID);
@$article=$articles[0];
if ($article==NULL) {
	exit(0);
}
$threadNum=bbs_get_thread_article_num($boardName,intval($article['ID']));
$total=$threadNum+1;
$threads=bbs_get_thread_articles($boardName, intval($article['ID']),0,$total);
$total=count($threads);
*/
$num = bbs_get_threads_from_gid($boardID, $groupID, $groupID , $articles , $haveprev );
if ($num==0) {
	exit(0);
}

//showTree($boardName,$boardID,$groupID,$article,$threads,$total);
?>
<body>
<script language="javascript" type="text/javascript" src="inc/browser.js"></script>
<script language="javascript">
	oTd=getParentRawObject("followTd<?php echo $groupID; ?>");
	oTd.innerHTML='<TABLE border=0 cellPadding=0 cellSpacing=0 width="100%" align=center><TBODY><?php showTree($boardName,$groupID,$articles,"showTreeItem");?></TBODY></TABLE>';
</script>
</body>

<?php
/*  rem by roy 2003.7.28
	按文章之间的继承关系排序

function showTree($boardName,$boardID,$groupID,$article,$threads,$threadNum) {
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
			showTreeItem($boardName,$groupID,$threads[$threadNum-$i-1],$i,$p);
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
function showTree($boardName,$boardID,$groupID,$article,$threads,$threadNum) {
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
			showTreeItem($boardName,$groupID,$article, 0, 0);
		else 
			showTreeItem($boardName,$groupID,$threads[$nodes[$head]],$nodes[$head], $level[$head]);
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

function showTreeItem($boardName,$groupID,$article,$startNum,$level, $lastflag){
	echo '<TR><TD class=TableBody1 width="100%" height=25>';
	for ($i=0;$i<$level;$i++) {
		if ($lastflag[$i]) {
			if ($i == $level - 1) echo '<img src="pic/treenode2.gif">'; // |-
			else echo '<img src="pic/treenode.gif">';                   // |
		} else {
			if ($i == $level - 1) echo '<img src="pic/treenode1.gif">'; // \
			else echo "&nbsp;&nbsp;";                               // nothing
		}
	}
	if ($article == null) {
		echo ' ... <span style="color:red">还有更多</span> ...';
	} else {
		echo '<img src="pic/nofollow.gif"><a href="disparticle.php?boardName='.$boardName.'&ID='.$groupID.'&start='.$startNum.'&listType=1">';
		/* 再多也多不过 ARTICLE_TITLE_LEN，别省略了吧 - atppp */
	/*	if (strLen($article['TITLE'])>22) { 
			echo htmlspecialchars(substr($article['TITLE'],0,22),ENT_QUOTES).'...';
		} else {
			echo htmlspecialchars($article['TITLE'],ENT_QUOTES);
		} */
		echo htmlspecialchars($article['TITLE'],ENT_QUOTES);
		echo '</a> -- <a href="dispuser.php?id='.$article['OWNER'].'">'.$article['OWNER'].'</a>';
	}
	echo '</td></tr>';
}
?>
