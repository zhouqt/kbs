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
if ($boardID==0) {
	exit(0);
}
$usernum = $currentuser["index"];
if (bbs_checkreadperm($usernum, $boardID) == 0) {
	exit(0);
	return false;
}
bbs_set_onboard($boardID,1);
$articles = bbs_getthreads($boardName, $articleID, 1);
@$article=$articles[0];
if ($article==NULL) {
	exit(0);
}
$threadNum=bbs_get_thread_article_num($boardName,intval($article['ID']));
$total=$threadNum+1;
$threads=bbs_get_thread_articles($boardName, intval($article['ID']),0,$total);
$total=count($threads);

?>
<script>
	parent.followTd<?php echo $articleID; ?>.innerHTML='<TABLE border=0 cellPadding=0 cellSpacing=0 width="100%" align=center><TBODY><?php showTree($boardName,$boardID,$articleID,$article,$threads,$total);?></TBODY></TABLE>';
</script>

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
function showTree($boardName,$boardID,$articleID,$article,$threads,$threadNum) {
		$IDs=array();
	$nodes=array();
	$printed=array();
	$level=array();
	$head=0;
	$bottom=0;
	$IDs[$bottom]=intval($threads[$threadNum-1]['ID']);
	$level[$bottom]=0;
	$printed[0]=1;
	$nodes[0]=0;
	$bottom++;
	while($head<$bottom) {
		showTreeItem($boardName,$articleID,$threads[$threadNum-$nodes[$head]-1],$nodes[$head],$level[$head]);
		for ($i=0;$i<$threadNum;$i++){
			if ( (!isset($printed[$i])) && ($threads[$threadNum-$i-1]['REID']==$IDs[$head]) ) {
				$IDs[$bottom]=intval($threads[$threadNum-$i-1]['ID']);
				$level[$bottom]=$level[$head]+1;
				$printed[$i]=1;
				$nodes[$bottom]=$i;
				$bottom++;
			}
		}
		$head++;
	}
}

function showTreeItem($boardName,$articleID,$thread,$threadID,$level){
	echo '<TR><TD class=tablebody1 width="100%" height=25>';
	for ($i=0;$i<$level;$i++) {
		echo "&nbsp;&nbsp;";
	}
	echo '<img src="pic/nofollow.gif"><a href="disparticle.php?boardName='.$boardName.'&ID='.$articleID.'&start='.$threadID.'&listType=1">';
	if (strLen($thread['TITLE'])>22) {
		echo htmlspecialchars(substr($thread['TITLE'],0,22),ENT_QUOTES).'...';
	} else {
		echo htmlspecialchars($thread['TITLE'],ENT_QUOTES);
	}
	echo '</a> -- <a href="dispuser.asp?name='.$thread['OWNER'].'">'.$thread['OWNER'].'</a></td></tr>';

}
?>
