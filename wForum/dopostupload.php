<?php
$needlogin=1;
require("inc/funcs.php");
html_init();
?>
<body topmargin=0 leftmargin=0>
<form name="form" method="post" action="dopostupload.php?board=<?php echo $_GET['board']; ?>" enctype="multipart/form-data">
<table width="100%" border=0 cellspacing=0 cellpadding=0>
<tr><td class=tablebody2 valign=top height=30>
<input type="hidden" name="MAX_FILE_SIZE" value="<?php echo ATTACHMAXSIZE; ?>">
<input type="file" name="upfile">
<input type="submit" name="Submit" value="上传" onclick="fname.value=file1.value,parent.document.forms[0].Submit.disabled=true,
parent.document.forms[0].Submit2.disabled=true;">
<font color=#FF0000 >本文还可上传<?php   echo ATTACHMAXCOUNT-getAttachmentCount(); ?>个</font>；
  论坛限制：一篇文章<?php   echo ATTACHMAXCOUNT; ?>个，<!--一天<?php   echo $GroupSetting[50]; ?>个,-->每个<?php   echo intval(ATTACHMAXSIZE/1024); ?>K
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
	$filecount=0;
	$attachdir=getattachtmppath($currentuser["userid"] ,$utmpnum);
	@mkdir($attachdir);
	if (($fp=@fopen($attachdir . "/.index","r"))!=FALSE) {
		while (!feof($fp)) {
			$buf=fgets($fp);
			$buf=substr($buf,0,-1); //remove "\n"
			if ($buf=='')
				continue;
			$file=substr($buf,0,strpos($buf,' '));
			if ($file=='')
				continue;
			$filecount++;
		}
		fclose($fp);
	}
	return $filecount;
}
?>
