<?php


require("inc/funcs.php");

html_init();
?>
<body >
<?php
	$ret=bbs_getwebmsg($srcid,$msgbuf,$srcutmpnum,$sndtime);
	if ($ret!=0)  {
?> 
	<bgsound src="/sound/msg.wav">
		<div id="msgcontent">
	<table cellspacing=1 cellpadding=0 align=center width="100%" class=tableBorder1 >
	<thead>
	<TR><Th height=20 align=left id=TableTitleLink align="center"><a href="dispuser.php?name=><?php echo $srcid; ?>" target=_blank><?php echo $srcid; ?></a>于(<?php echo strftime("%b %e %H:%M", $sndtime); ?>)发送给您的短信：
	</th></tr></thead>
	<tbody>
	  <tr>
		<td height=110 align="left" valign="top" class=tablebody1><?php echo htmlspecialchars($msgbuf); ?></td>
	  </tr>
	  <tr>
		<td height=20 align="right" valign="top" nowrap="nowrap" class=tablebody2><a  href="javascript:replyMsg('<?php 
	echo $srcid; ?>')" >[回讯息]</a> <a href="#" onclick="closeWindow();">[忽略]</a></td>
	  </tr>
	 </tbody>
	</table>
	</div>
	<script>
	parent.document.all.floater.innerHTML=msgcontent.innerHTML;
	parent.document.all.floater.style.visibility='visible';
	</script>
<?php
	}else {
?>
	<script>
	window.setTimeout("parent.document.frames('webmsg').document.location.reload();", 30000);
	</script>
<?php
	}
?>
</body>
</htmL>