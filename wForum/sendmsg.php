<?php


require("inc/funcs.php");

require("inc/usermanage.inc.php");

require("inc/user.inc.php");

html_init();

if ($loginok==1) {
	main();
}

function main() {
	global $currentuser;
	global $_GET;
	if (isset($_GET["destid"]))
		$destid = $_GET["destid"];
	else
		$destid = "";
	if (isset($_GET["destutmp"]))
		$destutmp = $_GET["destutmp"];
	else
		$destutmp = 0;
	settype($destutmp, "integer");
?>
<body>
<script language="javascript" type="text/javascript" src="inc/browser.js"></script>
<script language="javascript">
parent.pauseMsg();
</script>
<div id="msgcontent" >
<div onkeydown="if(event.keyCode==13 && event.ctrlKey) { obj=getRawObject('oSend');obj.focus();obj.click();} ">
<form action="dosendmsg.php" method=post name=messager id=messager >
<input type="hidden" name="destutmp" value="<?php echo $destutmp; ?>"/>
<table cellpadding=3 cellspacing=1 align=center class=TableBorder1>
          <tr> 
            <th colspan=3>发送短消息（请输入完整信息）</th>
          </tr>
          <tr> 
            <td class=TableBody1 valign=middle><b>送讯息给:</b></td>
            <td class=TableBody1 valign=middle>
              <input id="odestid" name="destid" maxlength="12" value="<?php echo $destid; ?>" size="12" onchange="msg_idchange(this.value);"/>
<?php
		if (!isset($_GET["destid"])) {
?>
              <SELECT name=font onchange=DoTitle(this.options[this.selectedIndex].value)>
              <OPTION selected value="">选择</OPTION>

			  </SELECT>
<?php
}
?>
            </td>
          </tr>
           <tr> 
            <td class=TableBody1 valign=top width=15%><b>内容：</b></td>
            <td  class=TableBody1 valign=middle>
              <input id="oMsgText" name="msg" maxlength="50" size="50" onchange="msg_textchange(this.value);" />
            </td>
          </tr>
		<tr> 
            <td  class=TableBody1 colspan=2> <input type="checkbox" name="isSMS" id="isSMS" <?php echo (isset($_GET['type'])&& ($_GET['type']=='sms'))?'checked':''; ?> onchange="msg_typechanged(this.checked);">&nbsp; <b>发送手机短信</b>
			</td>
		</tr>
          <tr> 
            <td  class=TableBody1 colspan=2>
<b>说明</b>：<br>
① 您可以使用<b>Ctrl+Enter</b>键快捷发送短信<br>
<!--
② 可以用英文状态下的逗号将用户名隔开实现群发，最多<b>5</b>个用户<br>
③ 标题最多<b>50</b>个字符，内容最多<b>300</b>个字符<br>
-->
</form>
            </td>
          </tr>
          <tr> 
            <td  class=TableBody2 valign=middle colspan=2 align=center> 
              <input type=button value="发送" name=Submit id="oSend" onclick="dosendmsg();">
             &nbsp;
<!--
			  <form action="showmsgs.php">
              <input type=button value="查看聊天记录" name="chatlog" >
			  </form>
			 -->
              &nbsp; 
              <input type="button" name="close" value="关闭" onclick="closeWindow();">
            </td>
          </tr>
        </table>
</div>
</div>
	<script language="javascript">
	oFloater=getParentRawObject("floater");
	oMsg=getRawObject("msgcontent");
	oFloater.innerHTML=oMsg.innerHTML;
	show(oFloater);
	</script>
<?php

}
?>
</body>