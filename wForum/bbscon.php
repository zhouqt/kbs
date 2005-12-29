<?php
	@$attachpos = $_GET["ap"]; //pointer to the size after ATTACHMENT PAD
	if ($attachpos!=0) {
		$needlogin=0;
	}
	require("inc/funcs.php");

	if (!$loginok && !$guestloginok && $attachpos!=0) bbs_setguest_nologin();
	
	$brdarr = array();
	if( isset( $_GET["bid"] ) ){
		$boardID = intval($_GET["bid"]);
		if( $boardID == 0 ){
			echo 2;
			exit(0);
		}
		$board = bbs_getbname($boardID);
		if( !$board ){
			echo 3;
			exit(0);
		}
		if( $boardID != bbs_getboard($board, $brdarr) ){
			echo 4;
			exit(0);
		}
	} else if (isset($_GET["board"])) {
		$board = $_GET["board"];
		if( $boardID != bbs_getboard($board, $brdarr) ){
			echo 24;
			exit(0);
		}
		$board = $brdarr["NAME"];
	} else {
		echo 5;
		exit(0);
	}
	if (!($isnormalboard = bbs_normalboard($board))) {
		if($loginok == 1) {
			$usernum = $currentuser["index"];
			if (bbs_checkreadperm($usernum, $boardID) == 0) {
				echo 6;
				exit(0);
			}
		} else {
			echo 7;
			exit(0);
		}
	}

	if (isset($_GET["id"])) {
		$id = intval($_GET["id"]);
	} else {
		echo 8;
		exit(0);
	}
	settype($id, "integer");

	if (isset($_GET["ftype"])){
		$ftype = $_GET["ftype"];
		if($ftype != $dir_modes["ZHIDING"])
			$ftype = $dir_modes["NORMAL"];
	} else
		$ftype = $dir_modes["NORMAL"];
	$total = bbs_countarticles($boardID, $ftype);
	if ($total <= 0) {
		echo 9;
		exit(0);
	}
	$articles = array ();
	$num = bbs_get_records_from_id($brdarr["NAME"], $id, 
			$ftype, $articles);
	if ($num == 0)	{
		echo 10;
		return;
	}

	$boardName = $brdarr["NAME"];
	if ($attachpos==0) {
		if (!isset($_GET["lw"])) {
			// jump to the article, this link must be generated from telnet session. added by atppp
			$gid = $articles[1]["GROUPID"];
			$haveprev = 0;
			$num = bbs_get_threads_from_gid($boardID, $gid, $gid, $articles, $haveprev );
			if ($num > 0) {
				for ($i = 0; $i < $num; $i++) { //可以用二分查找加速，以后再说吧
					if ($id == $articles[$i]["ID"]) {
						header("Location: disparticle.php?boardName=$boardName&ID=$gid&start=$i&listType=1");
						return;
					}
				}
			}
			echo 1;
		} else {
			viewArticle($articles[1], $boardID, $boardName);
		}
	} else {
		$filename=bbs_get_board_filename($boardName, $articles[1]["FILENAME"]);
		if ($isnormalboard) {
			if (cache_header("public",filemtime($filename),300)) {
				exit(0);
			}
		}
		require("inc/attachment.inc.php");
		output_attachment($filename, $attachpos);
	}
	
function viewArticle($article, $boardID, $boardName) {
	global $SiteURL;
	require("inc/ubbcode.php");

	$is_tex = SUPPORT_TEX ? $article["IS_TEX"] : 0;
	setStat(htmlspecialchars($article['TITLE'] ,ENT_QUOTES) . " " );
	html_init("","",$is_tex);
	$this_link = 'bbscon.php?bid='.$boardID.'&amp;id='.$article['ID'];
	$full_link = 'disparticle.php?boardName='.$boardName.'&amp;ID='.$article["GROUPID"];
?>
<body>
<table cellpadding="0" cellspacing="0" class="TableBorder1" style="table-layout:fixed;word-break:break-all">
<tr><td class="TableBody1"><?php
	$filename = bbs_get_board_filename($boardName, $article["FILENAME"]);
	$str = bbs_printansifile($filename,1,$this_link,$is_tex,0);
	echo DvbTexCode($str, 0, "TableBody2", $is_tex);
?></td></tr>
</table>
<p>
原文链接：<a href="<?php echo $this_link; ?>"><?php echo $SiteURL.$this_link; ?></a><br />
主题链接：<a href="<?php echo $full_link; ?>"><?php echo $SiteURL.$full_link; ?></a>
</p>
</body>
</html>
<?php
}
?>
