<?php
	/**
	 * This file lists articles to user.
	 * $Id$
	 */
    $setboard=0;
    require("funcs.php");
login_init();
	if ($loginok != 1)
		html_nologin();
	else
	{
		html_init("gb2312");
		if ($currentuser["userid"]=="guest")
			$ret=0;
		else
			$ret=bbs_getwebmsg($srcid,$msgbuf,$srcutmpnum,$sndtime);
?>
<meta http-equiv="pragma" content="no-cache"><style type="text/css">
A {color: #0000FF}
</style>
<?php
		if ($ret)
		{
?>
<bgsound src="/sound/msg.wav">
<body style="BACKGROUND-COLOR: #f0ffd0">
<table width="100%">
<form action=/bbsgetmsg.php name=form0>
  <tr>
    <td valign="top" nowrap="nowrap"><font color="green"><?php echo $srcid; ?></font> (<?php echo strftime("%b %e %H:%M", $sndtime); ?>): </td>
    <td align="left" valign="top"><?php echo htmlspecialchars($msgbuf); ?></td>
    <td align="right" valign="top" nowrap="nowrap"><a target="f3" href="/bbssendmsg.php?destid=<?php 
echo $srcid; ?>&destutmp=<?php 
echo $srcutmpnum; ?>">[»ØÑ¶Ï¢]</a> <a href="bbsgetmsg.php?refresh">[ºöÂÔ]</a></td>
  </tr>
</form>
</table>
<?php
			$frameheight = 54;
	    } else {
			//no msg
			$frameheight = 0;
			if (isset($_GET["refresh"])) {
?>
<meta http-equiv="Refresh" content="60; url=/bbsgetmsg.php">
<?php
			} else {
?>
<meta http-equiv="Refresh" content="600; url=/bbsgetmsg.php">
<?php	    
			}
		}
?>
<script language="javascript">
<!--
if (ff = top.document.getElementById("viewfrm"))
	ff.rows = "<?php echo $frameheight; ?>,*,20";
//-->
</script>
<?php	
		html_normal_quit();
	}
?>
