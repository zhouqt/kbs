<?php
$needlogin=1;
require("inc/funcs.php");
require("inc/board.inc.php");
html_init();
?>
<script>
parent.document.forms[0].Submit.disabled=false;
parent.document.forms[0].Submit2.disabled=false;
</script>
<?php

?>
<body topmargin=0 leftmargin=0>
<table width="100%" border=0 cellspacing=0 cellpadding=0>
<tr><td class=TableBody2 valign=top height=40>
<?php 
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

$attachdir=getattachtmppath($currentuser["userid"] ,$utmpnum);
@mkdir($attachdir);

@$errno=$_FILES['upfile']['error'];
if ($errno==UPLOAD_ERR_OK) {
	$buf=$_FILES['upfile']['name'];
	$tok = strtok($buf,"/\\");
	$act_attachname="";
	while ($tok) {
		$act_attachname=$tok;
		$tok = strtok("/\\");
	}
	$act_attachname=strtr($act_attachname,$filename_trans);
	$act_attachname=substr($act_attachname,-60);
	if ($act_attachname!="") {
		if ($_FILES['upfile']['size']>ATTACHMAXSIZE) 
			$errno=UPLOAD_ERR_FORM_SIZE;
	} else
		$errno=100;
}
$filecount=0;
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
		$filenames[] = $name;
		$filesizes[$name] = filesize($file);
			$totalsize+=$filesizes[$name];
			$filecount++;
		$allnames = $allnames . $name . ";";
	}
	fclose($fp);
}
if ($_FILES['upfile']['size']+$totalsize>ATTACHMAXTOTALSIZE) {
	unlink($attachdir . "/" . $act_attachname);
	unset($filenames,$act_attachname);
	$errno=UPLOAD_ERR_FORM_SIZE;
}
if ($filecount>ATTACHMAXCOUNT) {
	errorQuit( "附件个数超过规定！");
	break;
} 
switch ($errno) {
case UPLOAD_ERR_OK:
	@mkdir($attachdir);
	$tmpfilename=tempnam($attachdir,"att");
	if (isset($filesizes[$act_attachname])) {
		errorQuit( "存在同名文件！");
	} else {
		if (is_uploaded_file($_FILES['upfile']['tmp_name'])) {
			move_uploaded_file($_FILES['upfile']['tmp_name'], 
				$tmpfilename);
			 /* 填写 .index*/
			if (($fp=@fopen($attachdir . "/.index", "a"))==FALSE) {
					unlink($attachdir . "/" . $act_attachname);
			} else {
				fputs($fp,$tmpfilename . " " . $act_attachname . "\n");
				fclose($fp);
				$filenames[] = $act_attachname;
				$filesizes[$act_attachname] = filesize($tmpfilename);
				$totalsize+=$filesizes[$act_attachname];
				$filecount++;
				$allnames = $allnames . $act_attachname . ";";
				break;
			}
		}
		errorQuit("保存附件文件失败！");
	}
	break;
case UPLOAD_ERR_INI_SIZE:
case UPLOAD_ERR_FORM_SIZE:
	errorQuit( "文件超过预定的大小" . intval(ATTACHMAXSIZE/1024) . "KB");
	break;
case UPLOAD_ERR_PARTIAL:
	errorQuir( "文件传输出错！");
	break;
case UPLOAD_ERR_NO_FILE:
	errorQuit( "没有文件上传！");
	break;
case 100:
	errorQuit( "无效的文件名！");
	break;
default:
	errorQuit( "未知错误");
	break;
}
?>
<script>parent.frmAnnounce.Content.value+='[upload=<?php echo $filecount; ?>][/upload]'</script>
<?php
	if($filecount < ATTACHMAXCOUNT)
		print $filecount."个文件上传成功 [ <a href=# onclick=history.go(-1)>继续上传</a> ]";
	else
		print $filecount."个文件上传成功!本次已达到上传数上限。";
?>
</td></tr>
</table>
</body>
</html>
<?php
function errorQuit($str){
?>
&nbsp; <?php echo $str; ?>
</td></tr>
</body>
</html>
<?php
	exit(0);
}
?>
