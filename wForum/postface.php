<?php
$needlogin=1;
require("inc/funcs.php");
html_init();
?>
<body topmargin=0 leftmargin=0>
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
<tr><td class=TableBody2 valign=top height=30>
<input type="file" name="upfile">
<input type="submit" name="Submit" value="上传">
&nbsp;&nbsp;大小限制 <?php   echo intval((MYFACEMAXSIZE)/1024) ;?>K</font>
</td></tr>
</table>
</form>
</body>
</html>
