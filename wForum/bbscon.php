<?php
	/**
	 * This file displays article to user.
	 * $Id$
	 */
	$needlogin=0;
	require("inc/funcs.php");
function get_mimetype($name)
{
	$dot = strrchr($name, '.');
	if ($dot == $name)
		return "text/plain; charset=gb2312";
	if (strcasecmp($dot, ".html") == 0 || strcasecmp($dot, ".htm") == 0)
		return "text/html; charset=gb2312";
	if (strcasecmp($dot, ".jpg") == 0 || strcasecmp($dot, ".jpeg") == 0)
		return "image/jpeg";
	if (strcasecmp($dot, ".gif") == 0)
		return "image/gif";
	if (strcasecmp($dot, ".png") == 0)
		return "image/png";
	if (strcasecmp($dot, ".pcx") == 0)
		return "image/pcx";
	if (strcasecmp($dot, ".css") == 0)
		return "text/css";
	if (strcasecmp($dot, ".au") == 0)
		return "audio/basic";
	if (strcasecmp($dot, ".wav") == 0)
		return "audio/wav";
	if (strcasecmp($dot, ".avi") == 0)
		return "video/x-msvideo";
	if (strcasecmp($dot, ".mov") == 0 || strcasecmp($dot, ".qt") == 0)
		return "video/quicktime";
	if (strcasecmp($dot, ".mpeg") == 0 || strcasecmp($dot, ".mpe") == 0)
		return "video/mpeg";
	if (strcasecmp($dot, ".vrml") == 0 || strcasecmp($dot, ".wrl") == 0)
		return "model/vrml";
	if (strcasecmp($dot, ".midi") == 0 || strcasecmp($dot, ".mid") == 0)
		return "audio/midi";
	if (strcasecmp($dot, ".mp3") == 0)
		return "audio/mpeg";
	if (strcasecmp($dot, ".pac") == 0)
		return "application/x-ns-proxy-autoconfig";
	if (strcasecmp($dot, ".txt") == 0)
		return "text/plain; charset=gb2312";
	if (strcasecmp($dot, ".xht") == 0 || strcasecmp($dot, ".xhtml") == 0)
		return "application/xhtml+xml";
	if (strcasecmp($dot, ".xml") == 0)
		return "text/xml";
	return "application/octet-stream";
}

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
				echo 11;
				exit(0);
			}
        }
//		Header("Cache-control: nocache");

		$file = fopen($filename, "rb");
		fseek($file,$attachpos);
		$attachname='';
		while (1) {
			$char=fgetc($file);
			if (ord($char)==0) break;
			$attachname=$attachname . $char;
		}
		$str=fread($file,4);
		$array=unpack('Nsize',$str);
		$attachsize=$array["size"];
		Header("Content-type: " . get_mimetype($attachname));
		Header("Accept-Ranges: bytes");
		Header("Accept-Length: " . $attachsize);
		Header("Content-Disposition: filename=" . $attachname);
		echo fread($file,$attachsize);
		fclose($file);
		exit;

	}
	if ($loginok==1)
		bbs_brcaddread($brdarr["NAME"], $articles[1]["ID"]);
?>
