<?php
	@$attachpos = $_GET["ap"]; //pointer to the size after ATTACHMENT PAD
	if ($attachpos!=0) {
		$needlogin=0;
		$nocookie = 1;
	}
	require("inc/funcs.php");
	require("inc/attachment.inc.php");

	if (!$loginok && $attachpos!=0) bbs_setguest_nologin();
	
	$brdarr = array();
	if( isset( $_GET["bid"] ) ){
		$brdnum = $_GET["bid"] ;
		if( $brdnum == 0 ){
			echo 2;
			exit(0);
		}
		$board = bbs_getbname($brdnum);
		if( !$board ){
			echo 3;
			exit(0);
		}
		if( $brdnum != bbs_getboard($board, $brdarr) ){
			echo 4;
			exit(0);
		}
	} else {
		echo 5;
		exit(0);
	}
	if (!($isnormalboard = bbs_normalboard($board))) {
		if($loginok == 1) {
			$usernum = $currentuser["index"];
			if (bbs_checkreadperm($usernum, $brdnum) == 0) {
				echo 6;
				exit(0);
			}
		} else {
			echo 7;
			exit(0);
		}
	}

	if (isset($_GET["id"])) {
		$id = $_GET["id"];
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
	$total = bbs_countarticles($brdnum, $ftype);
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

	if ($attachpos==0) {
		// jump to the article, this link must be generated from telnet session. added by atppp
		$gid = $articles[1]["GROUPID"];
		$boardName = $brdarr["NAME"];
		$haveprev = 0;
		$num = bbs_get_threads_from_gid($brdnum, $gid, $gid, $articles, $haveprev );
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
		$filename=bbs_get_board_filename($brdarr["NAME"], $articles[1]["FILENAME"]);
		if ($isnormalboard) {
			if (cache_header("public",filemtime($filename),300)) {
				exit(0);
			}
		}
		output_attachment($filename, $attachpos);
	}
?>
