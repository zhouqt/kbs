<?php
$needlogin=1;
require("inc/funcs.php");
require("inc/board.inc.php");
html_init();
if (!isset($_GET['board'])) {
	errorQuit("未指定版面。");
}
$boardName=$_GET['board'];
$brdArr=array();
$boardID= bbs_getboard($boardName,$brdArr);
$boardArr=$brdArr;
$boardName=$brdArr['NAME'];
if ($boardID==0) {
	errorQuit("指定的版面不存在。");
}
$usernum = $currentuser["index"];
if (bbs_checkreadperm($usernum, $boardID) == 0) {
	errorQuit("您无权阅读本版！");
}
if (bbs_is_readonly_board($boardArr)) {
		errorQuit("本版为只读讨论区！");
		return false;
}
if (bbs_checkpostperm($usernum, $boardID) == 0) {
	errorQuit("您无权在本版发表文章！");
}
?>
<body topmargin=0 leftmargin=0>
<script src="inc/browser.js"  language="javascript"></script>
<script language="javascript">
function disableEdit(){
	if (isIE4) {
		parent.frmAnnounce.Submit.disabled=true;
		parent.frmAnnounce.Submit2.disabled=true;
	} else if (isW3C) {
		oSubmit=parent.document.getElementById("oSubmit");
		oSubmit2=parent.document.getElementById("oSubmit2");
		oSubmit.disabled=true;
		oSubmit2.disabled=true;		
	}
}
</script>
<form name="form" method="post" action="dopostupload.php?board=<?php echo $_GET['board']; ?>" enctype="multipart/form-data" onSubmit="disableEdit();" id="oForm">
<table width="100%" border=0 cellspacing=0 cellpadding=0>
<tr><td class=TableBody2 valign=top height=30>
<input type="hidden" name="MAX_FILE_SIZE" value="<?php echo ATTACHMAXSIZE; ?>">
<input type="file" name="upfile">
<input type="submit" name="Submit" value="上传">
<font color=#FF0000 >本文还可上传<?php   echo ATTACHMAXCOUNT-getAttachmentCount(); ?>个，总大小<?php   echo intval((ATTACHMAXTOTALSIZE-$totalsize)/1024) ;?>K</font>；
  论坛限制：一篇文章<?php   echo ATTACHMAXCOUNT; ?>个，<!--一天<?php   echo $GroupSetting[50]; ?>个,-->每个<?php   echo intval(ATTACHMAXSIZE/1024); ?>K，附件总大小<?php   echo intval(ATTACHMAXTOTALSIZE/1024); ?>K
</td></tr>
</table>
</form>
</body>
</html>
<?php 
$conn=null;

function getAttachmentCount(){
	global $currentuser;
	global $utmpnum;
	global $totalsize;
	$totalsize=0;
	$filecount=0;
	$attachdir=getattachtmppath($currentuser["userid"] ,$utmpnum);
	@mkdir($attachdir);

	if (($fp=@fopen($attachdir . "/.index","r"))!=FALSE) {
		while (!feof($fp)) {
			$buf=fgets($fp);
			$buf=substr($buf,0,-1); //remove "\n"
			if ($buf=="")
				continue;
			$file=substr($buf,0,strpos($buf,' '));
			if ($file=="")
				continue;
			$name=strstr($buf,' ');
			$name=substr($name,1);
			$totalsize+=filesize($file);
			$filecount++;
		}
		fclose($fp);
	}
	return $filecount;
}

function errorQuit($str){
?>
<body topmargin=0 leftmargin=0>
<table width="100%" border=0 cellspacing=0 cellpadding=0>
<tr><td class=TableBody2 valign=top height=30>
&nbsp; <?php echo $str; ?>
</td></tr>
</body>
</html>
<?php
	exit(0);
}
?>
