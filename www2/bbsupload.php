<?php
	require("www2-funcs.php");
	require("www2-bmp.php");
	$sessionid = login_init(TRUE);
	assert_login();
	
	@$action=$_GET["act"];
	$msg = "";
	if ($action=="delete") {
		@$act_attachname=$_GET["attachname"];
		settype($act_attachname, "string");
		$filename = base64_decode($act_attachname);
		$ret = bbs_upload_del_file($filename);
		if ($ret) $msg = bbs_error_get_desc($ret);
		else $msg = "删除 " . $filename . " 成功";
	} else if ($action=="add") {
		$counter = @intval($_POST["counter"]);
		for($i = 0; $i < $counter; $i++) {
			if (!isset($_FILES['attachfile' . $i])) {
				continue;
			}
			$attpost = $_FILES['attachfile' . $i];
			@$errno = $attpost['error'];
			switch ($errno) {
			case UPLOAD_ERR_OK:
				$ofile = $attpost['tmp_name'];
				if (!file_exists($ofile)) {
					$msg .= "文件传输出错！";
					break 2;
				}
				$oname = $attpost['name'];
				$htmlname = htmlspecialchars(my_basename($oname));
				if (!is_uploaded_file($ofile)) {
					die;
				}
				if (compress_bmp($ofile, $oname)) {
					$msg .= "过大 BMP 图片 " . $htmlname . " 被自动转换成 PNG 格式。<br/>";
				}
				$ret = bbs_upload_add_file($ofile, $oname);
				if ($ret) $msg .= bbs_error_get_desc($ret);
				else {
					$msg .= $htmlname . "上传成功！<br/>";
					continue 2;
				}
				break;
			case UPLOAD_ERR_INI_SIZE:
			case UPLOAD_ERR_FORM_SIZE:
				$msg .= "文件超过上限 " . sizestring(BBS_MAXATTACHMENTSIZE) . " 字节";
				break;
			case UPLOAD_ERR_PARTIAL:
				$msg .= "文件传输出错！";
				break;
			case UPLOAD_ERR_NO_FILE:
				continue 2;
				$msg .= "没有文件上传！";
				break;
			default:
				$msg .= "未知错误";
			}
			break;
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
	page_header("上传附件", FALSE);
?>
<body>
<script type="text/javascript">
<!--
function checkAnyFiles() {
	var frm = document.forms[0];
	var i;
	for (i=0; i<fileCounter; i++) {
		var o = frm.elements["attachfile"+i];
		if (o.value) return true;
	}
	return false;
}
function addsubmit() {
	if (!checkAnyFiles()){
		alert('您还没选择上传的附件');
		return false;
	} else {
		var frm = document.forms[0];
		var e2="bbsupload.php?act=add";
		getObj("winclose").style.display = "none";
		frm.counter.value = fileCounter;
		frm.action=e2;
		frm.paste.value='附件上传中，请稍候...';
		frm.paste.disabled=true;
		frm.submit();
		return true;
	}
}

function deletesubmit(f) {
	var e2="bbsupload.php?act=delete&attachname="+f;
	document.forms[1].action=e2;
	document.forms[1].submit();
}

function clickclose() {
	if (!checkAnyFiles()) return window.close();
	else if (confirm("您填写了文件名，但没有上传。是否确认关闭？")==true) return window.close();
	return false;
}

var fileCounter = 0, fileRemains = <?php echo (BBS_MAXATTACHMENTCOUNT - $filecount); ?>;
function moreAttach() {
	var ll = getObj("idAddAtt");
	var n = document.createElement("br");
	getObj("uploads").insertBefore(n, ll);
	if (gIE) {
		n = document.createElement("<input name='attachfile" + fileCounter + "'/>");
	} else {
		n = document.createElement("input");
		n.setAttribute('name', 'attachfile' + fileCounter);
	}
	n.setAttribute('type', 'file');
	n.setAttribute('size', 30);
	getObj("uploads").insertBefore(n, ll);
	fileCounter++;
	if (fileCounter >= fileRemains) ll.style.display = "none";
}
function allAttach() {
	while(fileCounter < fileRemains) moreAttach();
}

addBootFn(function() {
	if (fileRemains > 0) {
		getObj("idAddAtt").style.display = "inline";
		getObj("idAllAtt").style.display = "inline";
		moreAttach();
	}
	if (opener) {
		try {
			opener.document.forms["postform"].elements["attachname"].value = "<?php echo $allnames; ?>";
		} catch(e) {}
	} else {
		getObj("winclose").style.display = "none";
	}
});

//-->
</script>
<div style="width: 550px; margin: 1em auto;">
<?php if ($msg) echo "<font color='red'> 提示：".$msg."</font>"; ?>
<form name="addattach" method="post" ENCTYPE="multipart/form-data" class="left" action="">
<input type="hidden" name="counter" vaue="0" />
<?php if ($sessionid) echo "<input type='hidden' name='sid' value='$sessionid' />"; ?>
选择需要上传的文件后点上传：(<a id="idAllAtt" style="display:none;" href="javascript:void(0);" onclick="allAttach();">我要传好多附件</a>)
<div id="uploads">
<?php
	if ($filecount<BBS_MAXATTACHMENTCOUNT) {
?>
		<input type="hidden" name="MAX_FILE_SIZE" value="<?php echo(BBS_MAXATTACHMENTSIZE);?>" />
		<a id="idAddAtt" style="margin-left: 1em; display:none;" href="javascript:void(0);" onclick="moreAttach();">增加一个附件</a>
<?php
	} else {
?>
		附件个数已满！
		<input type="hidden" name="attachfile" />
<?php
	}
?>
</div>
<input type="button" value="上传" name="paste" onclick="addsubmit();" />
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
		echo "<li>".$f["name"]." (".sizestring($f["size"])."字节) <a href=\"javascript:deletesubmit('".base64_encode($f["name"])."');\">删除</a></li>";
	}
?>
</ol>
</form>
</div>
</body>
</html>
