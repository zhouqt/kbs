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
function showTree($boardName,$boardID,$articleID,$article,$threads,$threadNum) {
	$flags=array();
	for ($i=0;$i<$threadNum;$i++){
		showTreeItem($boardName,$articleID,$threads[$threadNum-$i-1],$i,$flags);
	}
}

function showTreeItem($boardName,$articleID,$thread,$threadID,&$flags){
	if (isset($flags[$thread['REID']]) ){
		$flags[$thread['ID']]=$flags[$thread['REID']]+1;
	} else {
		$flags[$thread['ID']]=0;
	}
	echo '<TR><TD class=tablebody1 width="100%" height=25>гагага';
	for ($i=0;$i<$flags[$thread['ID']];$i++) {
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
