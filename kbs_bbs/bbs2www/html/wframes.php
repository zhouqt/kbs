<?php
	require("funcs.php");
	login_init();
	if ($loginok != 1) {
		html_nologin();
		return;
	}
	$isguest = ($currentuser["userid"] == "guest"); //可以永远设成 TRUE
?>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=gb2312">
<title>欢迎莅临<?php echo BBS_FULL_NAME; ?></title>
</head>
<frameset name=mainframe frameborder=0 border=0 cols="167,11,*">
	<frame name=menu noresize="true" marginwidth=0 marginheight=0 src="/bbsleft.php">
	<frame scrolling=no noresize="true" name=toogle marginwidth=0 marginheight=0 src="/wtoogle.html">
	<frameset name="viewfrm" rows="<?php if (!$isguest) echo "18,"; ?>*,20">
<?php
	if (!$isguest) {
?>
		<frame scrolling=no noresize="true" marginwidth=4 marginheight="0" name="fmsg" src="bbsgetmsg.php">
<?php
	}
?>
		<frame marginwidth=0 marginheight=0 name="f3" src="<?php
			echo (isset($_GET["mainurl"])) ? $_GET["mainurl"] : MAINPAGE_FILE; ?>">
		<frame scrolling=no noresize="true" marginwidth=4 marginheight="1" name="f4" src="bbsfoot.php">
	</frameset>
</frameset>
</html>
