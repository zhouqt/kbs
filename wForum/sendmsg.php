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
<div id="msgcontent" onkeydown="if(event.keyCode==13 && event.ctrlKey)messager.submit()">
<script>
function dosendmsg(){
}
</script>
<form action="dosendmsg.php" method=post name=messager id=messager>
<input type="hidden" name="destutmp" value="<?php echo $destutmp; ?>"/>
<table cellpadding=3 cellspacing=1 align=center class=tableborder1>
          <tr> 
            <th colspan=3>发送短消息（请输入完整信息）</th>
          </tr>
          <tr> 
            <td class=tablebody1 valign=middle><b>送讯息给:</b></td>
            <td class=tablebody1 valign=middle>
              <input name="destid" maxlength="12" value="<?php echo $destid; ?>" size="12"/>
              <SELECT name=font onchange=DoTitle(this.options[this.selectedIndex].value)>
              <OPTION selected value="">选择</OPTION>

			  </SELECT>
            </td>
          </tr>
           <tr> 
            <td class=tablebody1 valign=top width=15%><b>内容：</b></td>
            <td  class=tablebody1 valign=middle>
              <input name="msg" maxlength="50" size="50" />
            </td>
          </tr>
          <tr> 
            <td  class=tablebody1 colspan=2>
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
            <td  class=tablebody2 valign=middle colspan=2 align=center> 
              <input type=button value="发送" name=Submit onclick="dosendmsg();">
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
	<script>
	parent.document.all.floater.innerHTML=msgcontent.innerHTML;
	</script>
<?php

}
?>