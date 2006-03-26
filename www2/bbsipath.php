<?php

	require_once("www2-funcs.php");
	login_init();
	bbs_session_modify_user_mode(BBS_MODE_CSIE_ANNOUNCE);

	if(isset($_POST["ipathAction"]))
	{
		$action = $_POST["ipathAction"];
		if($action == "modify")
		{
			$num = $_POST["num"];
			$title = $_POST["ipathTitle"];
			bbs_ipath_modify($num, $title);
		}
		else if($action == "set")
		{
			$num = $_POST["num"];
			$title = $_POST["ipathTitle"];
			$path = $_POST["ipathPath"];
			$path = "0Announce" . $path;
			bbs_ipath_modify($num, $title, $path);
		}
	}
	
	$ipaths = array();
	$ipaths = bbs_ipath_list();
	if($ipaths === FALSE)
	{
		html_error_quit("无法读取丝路。");
		exit;
	}
	
	page_header("丝路");
	if(isset($_GET["annpath"]))
		$annpath = $_GET["annpath"];
	else
		$annpath = "";
	if(isset($_GET["title"]))
		$anntitle = $_GET["title"];
	else
		$anntitle = $annpath;
	if(isset($_GET["inann"]))
		$inann = $_GET["inann"];
	else
		$inann = 0;
	$count = count($ipaths);
?>
<script src="static/www2-addons.js" type="text/javascript"></script>
<script type="text/javascript">
var ipathEditing = 0;
var ititle = new Array(<?php echo $count; ?>);
var ipath = new Array(<?php echo $count; ?>);
var currAnnPath = '<?php echo $annpath; ?>';
var currAnnTitle = '<?php echo $anntitle; ?>';
var p = new ipathWriter(<?php echo $inann; ?>);
<?php
	for($i=0; $i<$count; $i++)
	{
		$title = htmlspecialchars($ipaths[$i]["TITLE"], ENT_QUOTES);
		$path = htmlspecialchars($ipaths[$i]["PATH"], ENT_QUOTES);
		if($title == "")
			$path = "";
?>
p.i('<?php echo $title; ?>','<?php echo $path; ?>');
<?php
	}
?>
p.f();
</script>
<?php
	page_footer();
	
?>
