<?php
	require("inc/funcs.php");
	
	@$attachnum = $_GET["ap"];
	if ($attachnum == 0) {
		echo 1;
		exit;
	}
	
	if ($loginok != 1) {
		echo 2;
		exit;
	}

	$attachdir=bbs_getattachtmppath($currentuser["userid"] ,$utmpnum);

	if (($fp=@fopen($attachdir . "/.index", "r")) == FALSE) {
		echo 3;
		exit;
	}
	$id = 0;
	while (!feof($fp)) {
		$buf = fgets($fp);
		$id++;
		if ($id != $attachnum) continue;
		
		$buf=substr($buf,0,-1); //remove "\n"
		if ($buf=="") {
			echo 4;
			break;
		}
		$file = substr($buf,0,strpos($buf,' '));
		if ($file == "")  {
			echo 5;
			break;
		}
		$name = strstr($buf,' ');
		$name = substr($name,1);
		bbs_file_output_attachment($file, 0, $name);
		break;
	}
	fclose($fp);
?>
