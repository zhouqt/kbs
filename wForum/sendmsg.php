<?php


require("inc/funcs.php");

require("inc/usermanage.inc.php");

require("inc/user.inc.php");

html_init();

if ($loginok==1) {
	main();
}else {
	foundErr("本页需要您以正式用户身份登陆之后才能访问！");
}

if (isErrFounded()) {
	echo "<body>";
	html_error_quit();
}

show_footer();

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
<body  topmargin=0 leftmargin=0" onkeydown="if(event.keyCode==13 && event.ctrlKey)messager.submit()">

<form action="dosendmsg.php" method=post name=messager>
<input type="hidden" name="destutmp" value="<?php echo $destutmp; ?>"/>
<table cellpadding=3 cellspacing=1 align=center class=tableborder1>
          <tr> 
            <th colspan=3>发送短消息（请输入完整信息）</td>
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
            </td>
          </tr>
          <tr> 
            <td  class=tablebody2 valign=middle colspan=2 align=center> 
              <input type=Submit value="发送" name=Submit>
              &nbsp; 
              <input type=button value="查看聊天记录" name="chatlog" onclick="location.href='showmsgs.php'">
              &nbsp; 
              <input type="button" name="close" value="关闭" onclick="window.close()">
            </td>
          </tr>

        </table>
</form>
<?php

}
?>