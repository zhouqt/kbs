<?php
	require("www2-funcs.php");
	login_init();
	assert_login();

$filename_trans = array(" " => "_", 
	";" => "_", 
	"|" => "_",
	"&" => "_",
	">" => "_",
	"<" => "_",
	"*" => "_",
	"\"" => "_",
	"'" => "_"
	);

	global $errno;
	@$action=$_GET["act"];

		$attachdir=bbs_getattachtmppath($currentuser["userid"] ,$utmpnum);
		@mkdir($attachdir);
		if ($action=="add") {
			@$errno=$_FILES['attachfile']['error'];
			if ($errno==UPLOAD_ERR_OK) {
				$buf=$_FILES['attachfile']['name'];
				
				$tok = strtok($buf,"/\\");
				$act_attachname="";
				while ($tok) {
					$act_attachname=$tok;
						$tok = strtok("/\\");
				}
				$act_attachname=strtr($act_attachname,$filename_trans);
				$act_attachname=substr($act_attachname,-60);
				if ($act_attachname!="") {
						if ($_FILES['attachfile']['size']>BBS_MAXATTACHMENTSIZE) 
							$errno=UPLOAD_ERR_FORM_SIZE;
				} else
					$errno=100;
			}
		}
	
	page_header("csv导入", FALSE);
?>
<style type="text/css">
body { padding: 1em 0 0 1em; }
.txt01 {  font-family: "宋体"; font-size: 12px}
.form01 {  font-family: "宋体"; font-size: 12px; height: 20px; letter-spacing: 3px}
.form02 {  font-size: 12px}
</style>
<body bgcolor="#FFFFFF"  background="images/rback.gif">
<?php
				if ($action=="add") {
					switch ($errno) {
					case UPLOAD_ERR_OK:
						$tmpfilename=tempnam($attachdir,"att");
						if (is_uploaded_file($_FILES['attachfile']['tmp_name'])) {
							if( move_uploaded_file($_FILES['attachfile']['tmp_name'], 
								$tmpfilename) == FALSE ){
								echo "保存失败";
							}else{
								echo "文件导入成功！";
								$ret = bbs_csv_to_al($tmpfilename);
								echo "共导入".$ret."项";
								@unlink($tmpfilename);
								break;
							}
						}
						echo "保存文件失败！";
						break;
					case UPLOAD_ERR_INI_SIZE:
					case UPLOAD_ERR_FORM_SIZE:
						echo "文件超过预定的大小" . sizestring(BBS_MAXATTACHMENTSIZE) . "字节";
						break;
					case UPLOAD_ERR_PARTIAL:
						echo "文件传输出错！";
						break;
					case UPLOAD_ERR_NO_FILE:
						echo "没有文件上传！";
						break;
					case 100:
						echo "无效的文件名！";
					default:
						echo "未知错误";
					}
					echo "<br />";
					page_footer(FALSE);
				}
?>
<script language=javascript>
function addsubmit() {
  var e1,e3;
  var e2;
  var pos=0;
  var obj=document.forms[0].elements["attachfile"];
  if (!obj) return true;
  if (obj.value == ""){
	alert('您还没选择上传的文件');
	return false;
  } else {
		e2="bbsloadcsv.php?act=add";
		document.forms[0].action=e2;  
		document.forms[0].submit();
  }
}

function clickclose() {
	return window.close();
}
</script>
请选择要导入的csv文件:<br>
<form name="addattach" method="post" ENCTYPE="multipart/form-data" align="left" action="">
			  <input type="hidden" name="MAX_FILE_SIZE" value=<?php echo(BBS_MAXATTACHMENTSIZE);?>>
			  <input type="file" name="attachfile" size="20" value class="form02">
			  <input type="button" width="61"
		  height="21" value="导入" border="0" onclick="addsubmit()" class="form01" >
</form>
</body>
</html>
