<?php
	// $Id$
	if (isset($_GET["s"]))
	{
		$style = $_GET["s"];
		settype($style, "integer");
		switch ($style)
		{
		case 0:
			setcookie("STYLE",0);
			$colwidth="170,*";
			break;
		case 1:
		default:
			setcookie("STYLE",1);
			$colwidth="144,*";
		}
		$charset = "gb2312";
?>
<?xml version="1.0" encoding="<?php echo $charset; ?>"?>
<!DOCTYPE html
     PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=<?php echo $charset; ?>"/>
<meta http-equiv="Refresh" content="0; url=/cgi-bin/bbs/bbsleft" />
</head>
<body>
<script language="javascript">
top.mainframe.cols = "<?php echo $colwidth; ?>";
top.f3.navigate(top.f3.location);
top.f4.navigate(top.f4.location);
</script>
</body>
</html>
<?php
	}
?>
