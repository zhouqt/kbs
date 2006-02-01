<?php

	$filenames=array();
	$filesizes=array();
	require("www2-funcs.php");
	$sessionid = login_init(TRUE);
	assert_login();
	
	@$action=$_GET["act"];
	$msg = "";
	if ($action=="delete") {
		@$act_attachname=$_GET["attachname"];
		settype($act_attachname, "string");
		$ret = bbs_upload_del_file($act_attachname);
		switch($ret) {
			case -2:
				$msg = "没有这个文件";
				break;
			case 0:
				$msg = "删除文件成功";
				break;
			default:
				break;
		}
	} else if ($action=="add") {
		if (isset($_FILES['attachfile'])) {
			@$errno=$_FILES['attachfile']['error'];
		} else {
			$errno = UPLOAD_ERR_PARTIAL;
		}
		switch ($errno) {
		case UPLOAD_ERR_OK:
			$ofile = $_FILES['attachfile']['tmp_name'];
			$oname = $_FILES['attachfile']['name'];
			if (!is_uploaded_file($ofile)) {
				die;
			}
			if (defined("AUTO_BMP2JPG_THRESHOLD")) {
				$oname = basename($oname);
				if (strcasecmp(".bmp", substr($oname, -4)) == 0 && (filesize($ofile) > AUTO_BMP2JPG_THRESHOLD)) {
					$h = popen("identify -format \"%m\" ".$ofile, "r");
					if ($h) {
						$read = fread($h, 1024);
						pclose($h);
						if (strncasecmp("BMP", $read, 3) == 0) {
							$tp = tempnam("/tmp", "BMP2JPG");
							exec("convert -quality 90 $ofile jpg:$tp");
							if (file_exists($tp)) {
								unlink($ofile);
								$ofile = $tp;
								$oname = substr($oname, 0, -4) . ".jpg";
							}
						}
					}
				}
			}
			$ret = bbs_upload_add_file($ofile, $oname);
			switch($ret) {
				case 0:
					$msg = "文件上载成功！";
					break;
				case -1:
					$msg = "系统错误";
					break;
				case -2:
					$msg = "附件个数超过规定！";
					break;
				case -3:
					$msg = "无效的文件名！";
					break;
				case -4:
					$msg = "存在同名文件！";
					break;
				case -5:
					$msg = "保存附件文件失败！";
					break;
				case -6:
					$msg = "文件总量超过上限 " . sizestring(BBS_MAXATTACHMENTSIZE) . " 字节";
					break;
				default:
					break;
			}
			break;
		case UPLOAD_ERR_INI_SIZE:
		case UPLOAD_ERR_FORM_SIZE:
			$msg = "文件超过上限 " . sizestring(BBS_MAXATTACHMENTSIZE) . " 字节";
			break;
		case UPLOAD_ERR_PARTIAL:
			$msg = "文件传输出错！";
			break;
		case UPLOAD_ERR_NO_FILE:
			$msg = "没有文件上传！";
			break;
		default:
			$msg = "未知错误";
		}
	}
	$attachments = bbs_upload_read_fileinfo();
	$filecount = count($attachments);
	$allnames = array();$totalsize=0;
	for($i=0;$i<$filecount;$i++) {
		$allnames[] = $attachments[$i]["name"];
		$totalsize += $attachments[$i]["size"];
	}
	$allnames=implode(",",$allnames);
	page_header("粘贴附件", FALSE);
?>
<body>
<script type="text/javascript">
<!--
function addsubmit() {
	var obj=document.forms[0].elements["attachfile"];
	if (!obj) return true;
	if (obj.value == ""){
		alert('您还没选择上传的附件');
		return false;
	} else {
		var e2="bbsupload.php?act=add";
		getObj("winclose").style.display = "none";
		document.forms[0].action=e2;
		document.forms[0].paste.value='附件上载中，请稍候...';
		document.forms[0].paste.disabled=true;
		document.forms[0].submit();
		return true;
	}
}

function deletesubmit(f) {
	var e2="bbsupload.php?act=delete&attachname="+f;
	document.forms[1].action=e2;
	document.forms[1].submit();
}

function clickclose() {
	if (document.forms[0].elements["attachfile"].value == "") return window.close();
	else if (confirm("您填写了文件名，但没有上载。是否确认关闭？")==true) return window.close();
	return false;
}

if (opener) {
	opener.document.forms["postform"].elements["attachname"].value = "<?php echo $allnames; ?>";
} else {
	addBootFn(function() { getObj("winclose").style.display = "none"; });
}
//-->
</script>
<div style="width: 550px; margin: 1em auto;">
<?php if ($msg) echo "<font color='red'> 提示：".$msg."</font>"; ?>
<form name="addattach" method="post" ENCTYPE="multipart/form-data" class="left" action="">
<?php if ($sessionid) echo "<input type='hidden' name='sid' value='$sessionid' />"; ?>
选择需要上传的文件后点粘贴：（如有多个附件文件要粘贴，请重复这个步骤）<br/>
<?php
	if ($filecount<BBS_MAXATTACHMENTCOUNT) {
?>
		<input type="hidden" name="MAX_FILE_SIZE" value="<?php echo(BBS_MAXATTACHMENTSIZE);?>" />
		<input type="file" name="attachfile" size="20" />
		<input type="button" value="粘贴" name="paste" onclick="addsubmit();" />
<?php
	} else {
?>
		附件个数已满！</td>
		<input type="hidden" name="attachfile" />
<?php
	}
?>
&nbsp;&nbsp;&nbsp;<input type="button" id="winclose" value="上传完成, 关闭窗口" onclick="return clickclose()" />
<p>附件文件总量：<?php echo sizestring($totalsize); ?> 字节,
上限：<?php echo sizestring(BBS_MAXATTACHMENTSIZE); ?> 字节,
还能上传：<font color="#FF0000"><b><?php $rem = BBS_MAXATTACHMENTSIZE-$totalsize; 
	if ($rem < 0) $rem = 0; echo sizestring($rem); ?> 字节</b></font>.</p>
</form>

<form name="deleteattach" ENCTYPE="multipart/form-data" method="post" class="left" action=""> 
<?php if ($sessionid) echo "<input type='hidden' name='sid' value='$sessionid' />"; ?>
<ol style="padding-left: 2em; margin-left: 0em;">已经上传的附件列表: (最多能上传 <?php echo BBS_MAXATTACHMENTCOUNT; ?>
 个, 还能上传 <font color="#FF0000"><b><?php echo (BBS_MAXATTACHMENTCOUNT-$filecount); ?></b></font> 个)
<?php
	for($i=0;$i<$filecount;$i++) {
		$f = $attachments[$i];
		echo "<li>".$f["name"]." (".sizestring($f["size"])."字节) <a href=\"javascript:deletesubmit('".$f["name"]."');\">删除</a></li>";
	}
?>
</ol>
</form>
</div>
</body>
</html>
