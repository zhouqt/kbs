<?php
require("inc/funcs.php");
if (!USER_FACE) exit;
html_init();
?>
<body style="margin:0px;">
<script src="inc/browser.js"  language="javascript"></script>
<script language="javascript">
function disableEdit(){
	oSubmit=getParentRawObject("oSubmit");
	oSubmit2=getParentRawObject("oSubmit2");
	oSubmit.disabled=true;
	oSubmit2.disabled=true;		
}
</script>
<form name="form" method="post" action="dopostface.php" enctype="multipart/form-data" onSubmit="disableEdit();" id="oForm">
<table width="100%" border=0 cellspacing=0 cellpadding=0>
<tr><td class=TableBody2 valign=middle height=28>
<input type="file" name="upfile">
<input type="submit" name="Submit" value="上传">
&nbsp;&nbsp;大小限制 <?php   echo intval((MYFACEMAXSIZE)/1024) ;?>K</font>
</td></tr>
</table>
</form>
</body>
</html>
