<?php
	$needlogin=0;
	require("inc/funcs.php");
	require("inc/attachment.inc.php");

	@$attachpos=$_GET["ap"];//pointer to the size after ATTACHMENT PAD
	if ($attachpos==0) {
		echo 1;
		exit(0);
	}
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
	}
    $isnormalboard=bbs_normalboard($board);
    if (($loginok != 1) && !$isnormalboard) {
		echo $loginok;
		echo 5;
         exit(0);
    }
    bbs_set_onboard($brcnum,1);
	if($loginok == 1)
		$usernum = $currentuser["index"];
	if (!$isnormalboard && bbs_checkreadperm($usernum, $brdnum) == 0) {
		echo 6;
		exit(0);
	}
	if (isset($_GET["id"])) {
		$id = $_GET["id"];
	}	else {
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
	}else{
		$filename=bbs_get_board_filename($brdarr["NAME"], $articles[1]["FILENAME"]);
		if ($isnormalboard) {
	       	if (cache_header("public",filemtime($filename),300)) {
				exit(0);
			}
        }
//		Header("Cache-control: nocache");

		output_attachment($filename, $attachpos);
	}
?>
