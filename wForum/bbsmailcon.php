<?php

require("inc/funcs.php");
require("inc/usermanage.inc.php");
require("inc/attachment.inc.php");

if ($loginok != 1) {
	echo 1;
	exit;
}

if (!isset($_GET['num'])) {
	echo 2;
	exit;
}
$num = intval($_GET['num']);
if (isset($boxName)) {
	$boxName = $_GET['boxname'];
} else {
	$boxName='inbox';
}
if (!getMailBoxPathDesc($boxName, $path, $desc)) {
	echo 3;
	exit;
}

$dir = bbs_setmailfile($currentuser["userid"], $path);

if( filesize( $dir ) <= 0 ) {
	echo 4;
	exit;
}

$articles = array ();
if( bbs_get_records_from_num($dir, $num, $articles) ) {
	$file = $articles[0]["FILENAME"];
}else{
	echo 5;
	exit;
}

$filename = bbs_setmailfile($currentuser["userid"],$file) ;

if(! file_exists($filename)){
	echo 6;
	exit;
}

@$attachpos = $_GET["ap"]; //pointer to the size after ATTACHMENT PAD
if ($attachpos==0) {
	echo 7;
	exit;
}

output_attachment($filename, $attachpos);
?>
