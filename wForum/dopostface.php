<?php
require("inc/funcs.php");
require("inc/board.inc.php");
if (!USER_FACE) exit;
html_init();
if ($loginok != 1) die("游客？");
?>
<body style="margin: 0pt;">
<script src="inc/browser.js"  language="javascript"></script>
<script language="javascript">
	oSubmit=getParentRawObject("oSubmit");
	oSubmit2=getParentRawObject("oSubmit2");
	oSubmit.disabled=false;
	oSubmit2.disabled=false;	
</script>
<table width="100%" border=0 cellspacing=0 cellpadding=0>
<tr><td class=TableBody2 valign=middle height=28>
<?php

function get_extname($name) //我的本意其实只是防止 .PHP 类文件，不过先这样吧... - atppp
{
	$dot = strrchr($name, '.');
	if ($dot == $name)
		return false;
	if (strcasecmp($dot, ".jpg") == 0 || strcasecmp($dot, ".jpeg") == 0)
		return $dot;
	if (strcasecmp($dot, ".gif") == 0)
		return $dot;
	if (strcasecmp($dot, ".png") == 0)
		return $dot;
	return false;
}

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
		if ($_FILES['upfile']['size']>MYFACEMAXSIZE) 
			$errno=UPLOAD_ERR_FORM_SIZE;
		else {
			$ext = get_extname($act_attachname);
			if ($ext === false) $errno = 100;
		}
	} else {
		$errno=100;
	}
}
switch ($errno) {
case UPLOAD_ERR_OK:
	$tmp_filename = $_FILES['upfile']['tmp_name'];
	if (is_uploaded_file($tmp_filename)) {
		$sizeinfo = @getimagesize ($tmp_filename); //这个其实可以交给浏览器去做，用 JS 自动填充长宽值。不过反正也没多少 load，先这样吧。- atppp
		if ($sizeinfo === false) errorQuit("看起来不像有效的图像文件！");
		$width = $sizeinfo[0];
		$height = $sizeinfo[1];
		if ($width > 120 || $height > 120) {
			if ($width > $height) {
				$height = (int)((float)$height / $width * 120);
				$width = 120;
			} else {
				$width = (int)((float)$width / $height * 120);
				$height = 120;
			}
		}
		require_once("inc/myface.inc.php");
		$myface_filename = get_myface_filename($currentuser["userid"], $ext);
		if (move_uploaded_file($tmp_filename, get_myface_fs_filename($myface_filename))) {
			break;
		}
		errorQuit("保存附件文件失败！");
	}
	break;
case UPLOAD_ERR_INI_SIZE:
case UPLOAD_ERR_FORM_SIZE:
	errorQuit( "文件超过预定的大小" . intval(MYFACEMAXSIZE/1024) . "KB");
	break;
case UPLOAD_ERR_PARTIAL:
	errorQuir( "文件传输出错！");
	break;
case UPLOAD_ERR_NO_FILE:
	errorQuit( "没有文件上传！");
	break;
case 100:
	errorQuit( "文件名或者扩展名有点问题！");
	break;
default:
	errorQuit( "未知错误");
	break;
}
?>
<script language="javascript">
	newsrc = '<?php echo $myface_filename; ?>';
	o = getParentRawObject("imgmyface");
	o.src = newsrc;
	o.width = <?php echo $width; ?>;
	o.height = <?php echo $height; ?>;
	getParentRawObject("myface").value = newsrc;
	getParentRawObject("width").value = <?php echo $width; ?>;
	getParentRawObject("height").value = <?php echo $height; ?>;
</script>
文件上传成功 [ <a href="postface.php">好难看！我要重新上传</a> ]
</td></tr>
</table>
</body>
</html>
<?php

function errorQuit($str){
?>
&nbsp; <?php echo $str; ?> [ <a href="postface.php">重新上传</a> ]
</td></tr>
</body>
</html>
<?php
	exit(0);
}
?>
