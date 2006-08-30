<?php
	require("www2-funcs.php");
	require("www2-board.php");
	require("www2-bmp.php");
	$sessionid = login_init(TRUE);
	bbs_session_modify_user_mode(BBS_MODE_EDIT);
	assert_login();

	$brdnum = @$_GET["bid"] ;
	settype($brdnum,"integer");
	$brdarr = array();
	$isnormalboard = bbs_safe_getboard($brdnum, "", $brdarr);
	if (is_null($isnormalboard)) {
		html_error_quit("错误的讨论区");
	}
	$board = $brdarr["NAME"];

	$usernum = $currentuser["index"];

	$brd_encode = urlencode($board);
	if(bbs_checkpostperm($usernum, $brdnum) == 0) {
		html_error_quit("错误的讨论区或者您无权在此讨论区发表文章");
	}
	if (bbs_is_readonly_board($brdarr))
		html_error_quit("不能在只读讨论区发表文章");
	$ftype = $dir_modes["NORMAL"];

	bbs_board_nav_header($brdarr, "修改附件");
	
	if (isset($_GET['id']))
		$id = intval($_GET['id']);
	else
		html_error_quit("错误的文编号");
	$articles = array();
	$num = bbs_get_records_from_id($brdarr["NAME"], $id, $ftype, $articles);
	if ($num == 0)
		html_error_quit("错误的文编号");
	
	@$action=$_GET["act"];
	$msg = "";
	$ret = false;
	if ($action=="delete") {
		@$act_attachnum=$_GET["attachnum"];
		settype($act_attachnum, "integer");
		$ret = bbs_attachment_del($board, $id, $act_attachnum);
		if (!is_array($ret)) $msg = "错误: " . bbs_error_get_desc($ret);
		else $msg = "删除附件成功";
	} else if ($action=="add") {
		if (!bbs_is_attach_board($brdarr) && !($currentuser["userlevel"]&BBS_PERM_SYSOP))
			html_error_quit("不能添加附件");
		if (isset($_FILES['attachfile'])) {
			@$errno=$_FILES['attachfile']['error'];
		} else {
			$errno = UPLOAD_ERR_PARTIAL;
		}
		switch ($errno) {
		case UPLOAD_ERR_OK:
			$ofile = $_FILES['attachfile']['tmp_name'];
			if (!file_exists($ofile)) {
				$msg = "文件传输出错！";
				break;
			}
			$oname = $_FILES['attachfile']['name'];
			if (!is_uploaded_file($ofile)) {
				die;
			}
			if (compress_bmp($ofile, $oname)) {
				$msg = "过大 BMP 图片被自动转换成 PNG 格式。";
			}
			$ret = bbs_attachment_add($board, $id, $ofile, $oname);
			if (!is_array($ret)) $msg = "错误:" . bbs_error_get_desc($ret);
			else $msg .= "添加附件成功";
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
	if (!is_array($ret)) {
		$attachments = bbs_attachment_list($board, $id);
		if (!is_array($attachments)) {
			html_error_quit(bbs_error_get_desc($attachments));
		}
	} else {
		$attachments = $ret;
	}
	$filecount = count($attachments);
	$totalsize=0;
	$allnames = array();$allnames_p = array();$allpos = array();$alllen=array();
	for($i=0;$i<$filecount;$i++) {
		$allnames[] = $attachments[$i]["name"];
		$allnames_p[] = "'" . addslashes($attachments[$i]["name"]) . "'";
		$allpos[] = $attachments[$i]["pos"];
		$alllen[] = $attachments[$i]["size"];
		$totalsize += $attachments[$i]["size"];
	}
	page_header("上传附件", FALSE);
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
		var e2="bbseditatt.php?bid=<?php echo $brdnum; ?>&id=<?php echo $id; ?>&act=add";
		getObj("winclose").style.display = "none";
		document.forms[0].action=e2;
		document.forms[0].paste.value='附件上载中，请稍候...';
		document.forms[0].paste.disabled=true;
		document.forms[0].submit();
		return true;
	}
}

function deletesubmit(f) {
	var e2="bbseditatt.php?bid=<?php echo $brdnum; ?>&id=<?php echo $id; ?>&act=delete&attachnum="+f;
	if(confirm("确定删除这个附件？")) {
		document.forms[1].action=e2;
		document.forms[1].submit();
	}
}

function clickclose() {
	if (document.forms[0].elements["attachfile"].value == "") return window.close();
	else if (confirm("您填写了文件名，但没有上载。是否确认关闭？")==true) return window.close();
	return false;
}
att = new attWriter(<?php echo $brdnum; ?>,<?php echo $id; ?>,0,0,<?php echo ($isnormalboard?"1":"0"); ?>);
addBootFn(function() {
	var name = [<?php echo implode(",",$allnames_p); ?>];
	var len = [<?php echo implode(",",$alllen); ?>];
	var pos = [<?php echo implode(",",$allpos); ?>];
	var i;
	for(i=0; i<pos.length; i++) {
		var o = getObj("att" + i);
		if (o) o.href = attachURL(name[i], len[i], pos[i]);
	}
});
if (!opener) {
	addBootFn(function() { getObj("winclose").style.display = "none"; });
}
//-->
</script>
<div style="width: 550px; margin: 1em auto;">
<?php if ($msg) echo "<font color='red'> 提示：".$msg."</font>"; ?>
<form name="addattach" method="post" ENCTYPE="multipart/form-data" class="left" action="">
发信人: <?php echo $articles[1]['OWNER']; ?>, 信区: <?php echo $brd_encode; ?> [<a href="bbsdoc.php?board=<?php echo $brd_encode; ?>">本讨论区</a>]<br/>
标&nbsp;&nbsp;题: <a href="bbscon.php?bid=<?php echo $brdnum; ?>&id=<?php echo $id; ?>"><?php echo $articles[1]['TITLE']; ?> </a><br/><br/>
<?php if ($sessionid) echo "<input type='hidden' name='sid' value='$sessionid' />"; ?>
选择需要添加为附件的文件后点上传：<br/>
<?php
	if (!bbs_is_attach_board($brdarr) && !($currentuser["userlevel"]&BBS_PERM_SYSOP)) {
?>
		本版不能上传附件！
		<input type="hidden" name="attachfile" />	
<?php
	} else if ($filecount<BBS_MAXATTACHMENTCOUNT) {
?>
		<input type="hidden" name="MAX_FILE_SIZE" value="<?php echo(BBS_MAXATTACHMENTSIZE);?>" />
		<input type="file" name="attachfile" size="20" />
		<input type="button" value="上传" name="paste" onclick="addsubmit();" />
<?php
	} else {
?>
		附件个数已满！
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
<ol style="padding-left: 2em; margin-left: 0em;">帖子内的附件列表: (最多能上传 <?php echo BBS_MAXATTACHMENTCOUNT; ?>
 个, 还能上传 <font color="#FF0000"><b><?php echo (BBS_MAXATTACHMENTCOUNT-$filecount); ?></b></font> 个)
<?php
	for($i=0;$i<$filecount;$i++) {
		$f = $attachments[$i];
		echo "<li><a target='_blank' href='#' id='att".$i."'>".$f["name"]."</a> (".sizestring($f["size"])."字节) <a href=\"javascript:deletesubmit('".($i+1)."');\">删除</a></li>";
	}
?>
</ol>
</form>
</div>
</body>
</html>
