<?php

	require_once("www2-funcs.php");
	login_init();
	bbs_session_modify_user_mode(BBS_MODE_CSIE_ANNOUNCE);
		
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
?>
<script src="www2-addons.js" type="text/javascript"></script>
<script type="text/javascript">
var p = new ipathWriter('<?php echo $annpath; ?>');
<?php
	$count = count($ipaths);
	for($i=0; $i<$count; $i++)
	{
		$title = htmlspecialchars($ipaths[$i]["TITLE"], ENT_QUOTES);
		$path = htmlspecialchars($ipaths[$i]["PATH"], ENT_QUOTES);
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